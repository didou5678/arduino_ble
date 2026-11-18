#include <mybase.h>


void myloghex(const uint8_t* pdata,size_t datalen){
    for(size_t i=0;i<datalen;i++){
        mylog("0x%02x ",pdata[i]);
    }
    mylog("\n");
}


//同步时间 
bool time_init(const char* ntpserver,long tzoffset,int daylight,uint32_t sync_secs){
  my_printf("config time from: %s\n",(const char*)ntpserver);
  configTime((long)tzoffset,(int)daylight,(const char*)ntpserver);

#if defined(ESP32)
  if(sync_secs > 15){
    sntp_set_sync_interval(sync_secs*1000);
    sntp_restart();
  }
#endif

  return true;
}

void time_sntpstop(){
    #if defined(ESP32)
    if (esp_sntp_enabled()){
        esp_sntp_stop();
    }  
    #endif 
}


void random_macaddr(uint8_t* macaddr) {
#ifdef ESP32    
    if(macaddr){
        macaddr[0] = 0xC0 | (esp_random() & 0x3F); // 保证最高两bit=11
        for(int i=1;i<6;i++)
            macaddr[i] =(uint8_t)(esp_random()%256);
    }
#elif defined(ESP8266)
    macaddr[0] = 0xC0 | (ESP.random() & 0x3F);
    for(int i=1;i<6;i++){
        macaddr[i] = (uint8_t)(ESP.random() % 256);
    }

#endif 
}

bool macstr2uint8_ble(const char *str, uint8_t outmac[6]){
 if(str){
 return sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
        &outmac[5], &outmac[4], &outmac[3],
        &outmac[2], &outmac[1], &outmac[0]) ==6;}   
 return false;
 }


bool macstr2uint8(const char *str, uint8_t outmac[6]){
 if(str){
 return sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
        &outmac[0], &outmac[1], &outmac[2],
        &outmac[3], &outmac[4], &outmac[5]) ==6;}
 return false;        
}
 

#ifndef ESP8266
void esp32_deepsleep(uint32_t secs){
    uint64_t nsec=secs;
    nsec*=1000000ULL;
    esp_deep_sleep(nsec);    
}
#endif

void failed2restartESP(uint32_t waitms){
    delay(waitms);
    ESP.restart();
}


void random_string(char* buff,size_t bufflen,size_t count){
if(count> bufflen) return;
//const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const int charsetLen = sizeof(charset)-1;    
for(size_t i=0;i<count && i < bufflen-1;i++){
        buff[i] = charset[random(charsetLen)];
    }
    buff[count] = '\0';    
}

/*
volatile static uint32_t failedrestart_waitms = 0;
void failed2restartESP_loop(uint32_t waitms){
  unsigned long nowt=millis();  
  if(nowt-this->lastmills > this->interval){
        this->lastmills=nowt;
        return true;
    }    
    ESP.restart();
}*/


//让loop降低cpu 
static unsigned long loop_lasttick=0;
const unsigned char LOOP_DUMMYRUN=10; // 需要测试空转的时间

void loop_idle(){
unsigned long nowt=millis();
if(nowt-loop_lasttick <= LOOP_DUMMYRUN){ 
  delay(10); 
  } 
loop_lasttick=millis();
}
