#ifndef _MYBLE_BTHOME_ADVERT_H
#define _MYBLE_BTHOME_ADVERT_H
#include <mybase.h>
#include <NimBLEDevice.h>
#include <mybthome.h>



//ini加载库 lib_deps = h2zero/NimBLE-Arduino

/*
用于 bthome v2 规范 的 ble 广播
*/

#define  LEGACY_BLEADV_MAXLEN 31U

/*
    BTHOME 载荷最大字节
    ble4 adv 最大31  -3 adhdr = 28
    behome hdr固定开销 5 = 28-5 =23
*/
#define BTHOME_PAYLOADMAXLEN 23U



//初始, 调用 NimBLEDevice::deinit(true) 关闭
NimBLEAdvertising* bleadv_initdev(const char* advname=nullptr);

//如果 padvData复用 每次发送不同的数据之前 需要清空原有的数据 
void bleadv_resetdata(NimBLEAdvertisementData* padvData);

/*发送通告 interval * 0.625= 实际等待间隔 
返回已经装载的数据长度 函数调用失败返回0
*/
size_t bleadv_setdata(NimBLEAdvertising* padv,NimBLEAdvertisementData* padvData,advCompleteCB_t pfunc,uint16_t MinInterval=200,uint16_t MaxInterval=400);

/*
调用这个函数会序列化NimBLEAdvertisementData(aka 组成一组uint数组,未调用之前保存在类的成员),如果添加新的data需要再次调用此函数重新计算长度
返回0失败,成功返回字节数 
*/
size_t bleadv_datalen(NimBLEAdvertisementData* padvData);

//获取剩余长度
size_t bleadv_remainlen(NimBLEAdvertisementData* padvData);


void print_advdata(const NimBLEAdvertisementData* padvData);


//payloadlen <=22 
//counter arduino使用 RTC_DATA_ATTR uint8_t bth_counter = 0;初始为0 在断电前 包括休眠期间变量的值会被保存 因此在每个新的传感器数值更新 可以累计+1 
//加密情况下忽略counter
bool bleadv_bthome2_setdata(NimBLEAdvertisementData* padvData,uint8_t devinfo,uint8_t* payload,uint16_t payloadlen);

//发送广播
bool bleadv_start(NimBLEAdvertising *padving,uint32_t duration=3000U);

#endif //_MYBLE_BTHOME_ADVERT_H
