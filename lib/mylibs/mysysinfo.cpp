#include <mysysinfo.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif


String getChipModel(){
#ifdef ARDUINO_ARCH_ESP32
    const char* raw = ESP.getChipModel();
    if(raw==nullptr){
        return String("unknown");
    }
    return String(ESP.getChipModel());
#elif ARDUINO_ARCH_ESP8266
  return String("esp8266"); // esp8266 只有一种芯片型号
#endif
}


String get_selfdef_espID(const char* hostname,const char* macaddr){
    if(! hostname || !macaddr) return String("unknown");
    String str(hostname);
    int idx=str.indexOf('-');
    if(idx !=-1){
        str=str.substring(0,idx);
    }
    String macraw(macaddr);
    String macnew;
    for(auto c: macraw)
    {
    if(c!=':')
    macnew+=c;
    }
    return str+ "_" + macnew;
}

static mymills gmills={0};

mymills myuptime1(void){
  uint32_t nowt =(uint32_t)millis();
  //溢出
  gmills.part.high += (gmills.part.low > nowt);
  gmills.part.low = nowt;
  return gmills;
}



void uptime2time(OUT char* buff,size_t bufflen){

__uint64_t high_total_sec = (__uint64_t)gmills.part.high * 0xFFFFFFFFUL /1000;

__uint32_t high_days = high_total_sec / 86400;
__uint32_t high_secs = high_total_sec % 86400;

__uint32_t low_total_sec = (gmills.part.low / 1000) + high_secs;
__uint32_t low_days = low_total_sec / 86400;

__uint32_t rem_sec = low_total_sec % 86400;
__uint32_t hours = rem_sec / 3600;
__uint32_t mins  = (rem_sec % 3600) / 60;
__uint32_t secs  = rem_sec % 60;

__uint32_t total_days = high_days + low_days;

my_printf("u64=%llu ", gmills.u64);

if(total_days>0)
  snprintf(buff, bufflen, "%d天 %d小时 %d分", total_days, hours, mins);
  else if (hours>0)
    snprintf(buff, bufflen, "%d小时 %d分", hours, mins);
    else if(mins>0)
      snprintf(buff, bufflen, "%d分 %d秒", mins,secs);
      else
        snprintf(buff,bufflen,"%d秒",secs);

}


void print_sysinfo(){
  char buf[13];
  #ifdef ESP8266
  uint32_t chip_id = ESP.getChipId();
  #elif defined(ESP32)
  uint64_t chip_id = ESP.getEfuseMac();
  #endif
  my_printf("chip_id: in dec: %" PRIu64 "\n", chip_id);
  my_printf("chip_id: in hex: %" PRIX64 "\n", chip_id);
  sprintf(buf, "%08" PRIX32, chip_id);
  my_printf("chip_id: %s\n",buf);

  my_printf("getChipModel: %s\n",getChipModel().c_str());  

 
  String hostname = WiFi.getHostname();
  my_printf("Hostname: %s\n", hostname.c_str());

  String ssid=WiFi.SSID();
  my_printf("SSID: %s\n", ssid.c_str());

  uint8_t rssi = WiFi.RSSI();
  my_printf("RSSI: %d dBm\n", rssi);

  IPAddress ip = WiFi.localIP();
  String ipStr = ip.toString();
  my_printf("IP: %s\n", ipStr.c_str());

  String macStr = WiFi.macAddress();
  my_printf("MAC: %s\n", macStr.c_str());

  uint32_t freeHeap = ESP.getFreeHeap(); 
  my_printf("Free Heap: %u bytes\n", freeHeap);
  
}

//判断：ESP32S2 || ESP32S3 || ESP32C3 || ESP32C6
#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
float get_chiptemperature(){
//mylog("Chip Temp: %.2f °C\n", t);
return temperatureRead();
}
#endif


#ifdef ENABLE_CPU_STATS
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) \
|| defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
void printTaskStats()
{
    char buf[2048];
    vTaskGetRunTimeStats(buf);
    Serial.println("\r\n==== FreeRTOS Runtime Stats ====");
    Serial.println(buf);
}
#endif 
#endif //ENABLE_CPU_STATS

#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
bool set_esp32_macaddr(uint32_t type,const uint8_t* macaddr){
  esp_err_t ret;

  //if(type > 4) return false; //类型

  if( macaddr ==NULL){
    uint8_t randmac[6];
    random_macaddr(randmac);
    ret = esp_iface_mac_addr_set(randmac,(esp_mac_type_t)type);
  }else{
    ret = esp_iface_mac_addr_set(macaddr, (esp_mac_type_t)type);
  }
  if(ret != ESP_OK){
    mylog("set bt mac fail: %d\n", ret);
    return false;
  }
  return true;
}
#endif 




