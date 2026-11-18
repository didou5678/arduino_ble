#ifndef MYSYSTEMINFO
#define MYSYSTEMINFO
#include <mybase.h>

#ifdef ESP32
#include "esp_mac.h"
#endif 


//打印系统信息 
void print_sysinfo();




//获取设备型号 如esp32c3
String getChipModel();


//my_printf("self def id: %s\n",get_selfdef_espID(WiFi.getHostname(),WiFi.macAddress().c_str()).c_str());

//获取 esp32c3_xxxxxx mac地址
String get_selfdef_espID(const char* hostname,const char* macaddr);


//获取cpu温度 特定esp32 S2 / S3 / C3 / C6 才能读取,其他不在定义内的返回 0
#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
float get_chiptemperature();
#endif




union mymills {
    uint64_t u64;
    struct{
        //小端 low,high , 大端 high,low
       #ifdef IS_LITTLE_ENDIAN 
         uint32_t low;
         uint32_t high;
       #elif defined(IS_BIG_ENDIAN)
         uint32_t high;
         uint32_t low;
       #endif 
    }part;
};

mymills myuptime1();

//将uptime转换可读的 xx天xx时xx分的形式 
void uptime2time(OUT char* buff,size_t bufflen);



#ifdef ENABLE_CPU_STATS  /*定义开关 一般不使用*/
//打印cpu负载 
/*
ini文件配置 
build_flags -DconfigGENERATE_RUN_TIME_STATS=1
    -DconfigUSE_STATS_FORMATTING_FUNCTIONS=1
    -DconfigUSE_TRACE_FACILITY=1
*/
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) \
|| defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
#include "freertos/FreeRTOS.h"
#include "esp_cpu.h"
// IDF >=5.x 统一API
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE() esp_cpu_get_cycle_count()

void printTaskStats();

#elif defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2)
// IDF v4.x Xtensa 旧版本，保留xthal
#include "xtensa/core-macros.h"
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE() xthal_get_ccount()
#else
// 其他芯片不支持，空定义，编译不报错，但runtime stats无效
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE() 0
#endif

#endif //ENABLE_CPU_STATS 


/*
设置 mac地址类型
typedef enum {
    ESP_MAC_WIFI_STA,    0  // WiFi Station 接口MAC，6字节
    ESP_MAC_WIFI_SOFTAP, 1  // WiFi SoftAP 接口MAC，6字节
    ESP_MAC_BT,          2  // 蓝牙（BLE/BR）控制器MAC，6字节 
    ESP_MAC_ETH,         3  // 以太网MAC，6字节
    ESP_MAC_IEEE802154,  4 // 802.15.4（Zigbee/Thread，C6/H2支持），8字节
    ESP_MAC_BASE,          // Base基准MAC（用于派生其他接口MAC）
    ESP_MAC_EFUSE_FACTORY, // eFuse出厂原始MAC（只读，不能用在set）
    ESP_MAC_EFUSE_CUSTOM,  // eFuse客户自定义烧录MAC-----只读
    ESP_MAC_EFUSE_EXT      // 802.15.4扩展MAC------2字节，只读  
} esp_mac_type_t;
*/


#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
#include "esp_mac.h"
//设置mac地址  必须在ble或wifi 堆栈初始化之前调用才会生效
bool set_esp32_macaddr(uint32_t type,const uint8_t* macaddr=NULL);

#endif









#endif
