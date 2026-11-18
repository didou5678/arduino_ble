#ifndef MYBASEHDR_H
#define MYBASEHDR_H
#include <Arduino.h>  



#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif

//字节序 大小端判断
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define IS_LITTLE_ENDIAN  
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define IS_BIG_ENDIAN    
#endif

/*
用法 
#if IS_LITTLE_ENDIAN
  //小端平台代码
#else
  //大端平台代码
#endif
*/



//定义 MYDEBUG 宏（必须放在自定义宏之前），开启调试模式
//build_flags = -DMYPRINT


#ifdef MYPRINT
  // 开启调试：执行Serial.println
  #define my_println(msg) Serial.println(msg)
  #define my_printf(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)

#ifndef mylog
#define mylog(fmt, ...)  Serial.printf(fmt, ##__VA_ARGS__)
void myloghex(const uint8_t* pdata,size_t datalen);
#endif 

#else
  // 关闭调试：空语句（什么都不执行）
  #define my_println(msg)
  #define my_printf(fmt, ...)
  #define mylog(fmt, ...)

#endif //MYPRINT




#ifndef serial_begin
#define serial_begin() do { Serial.begin(115200); delay(1000); } while(0);
#endif 

#ifndef serial_end
#define serial_end Serial.end()
#endif

#if defined(ESP32)
#include <esp_sntp.h>
#endif

//参数 sync_secs同步sntp间隔 最小15s 默认1小时,如果设置为0 ==使用默认 
bool time_init(const char* ntpserver,long tzoffset=8*3600,int daylight=0,uint32_t sync_secs=900);

//停止sntp
void time_sntpstop();

//这个函数必须放在loop()最后一行 否则行为不可预测
void loop_idle();


//产生随机mac地址  
void random_macaddr(uint8_t* macaddr);

/*
BLE‑AdvA wire 空中原始字节序
将mac地址 11:22:33:44:55:66 转uint8t样式 
0x66 0x55 0x44 0x33 0x22 0x11  
参数str需要严格的mac格式 否则crash 
*/
bool macstr2uint8_ble(const char *str, uint8_t outmac[6]);

/*
将mac地址 11:22:33:44:55:66 转uint8t样式 
0x11 0x22 0x33 0x44 0x55 0x66 
*/
bool macstr2uint8(const char *str, uint8_t outmac[6]);


//产生随机字符串 
void random_string(char* buff,size_t bufflen,size_t count);


//在进入 deep sleep 前，**必须停掉 WiFi、NimBLE BLE，不然可能会异常、唤醒后崩溃**
#ifndef ESP8266
#include <esp_sleep.h>
void esp32_deepsleep(uint32_t secs);
#endif 

void failed2restartESP(uint32_t waitms=15000);

#endif
