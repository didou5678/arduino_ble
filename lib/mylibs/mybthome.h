#ifndef _MYBTHOME_H
#define _MYBTHOME_H
#include <mybase.h>

// https://bthome.io/format/  用户传感器数据格式 

/*
(ad=3bytes)+ [name=2+namelen] + (adhdr 2bytes + bthome uuid 0xd2fc + devinfo 1byte 0x40 + counter 1byte )=6 + bthome data max= 22 -[namelen+2]

bthome hdr 未加密的
bthome uuid 0xd2fc + devinfo 1byte 0x40 + counter 1byte = 4 bytes

加密的 
UUID(2) | devinfo(1) | ciphertext | counter(4,小端) | MIC(4) = 3bytes + 后面的8bytes =11 bytes

*/

#define BLEADVMAXLEN 31 
#define BTHOMEMAXPAYLOAD 23  // 明文最大载荷长度 31-3-5 
#define BTHOMEMAXPAYLOAD_AESCCM 15  // 不考虑其他ad段落 加密包允许的最大载荷  31 - adhdr 3 +  5 bthome hdr + 4 count +4 tag    



#ifdef IS_LITTLE_ENDIAN
#define BTHOMEUUID ((uint16_t)0xFCD2U)  //在空中的包 值为 0xD2FC,在发送和接收端变量读取则是 0xFCD2 
#else 
#define BTHOMEUUID ((uint16_t)0xD2FCU)
#endif //IS_LITTLE_ENDIAN

//bit5-7 = 010（版本 2），bit2=0（周期上报），bit0=0（未加密），其余保留位全部 0
#define BTHOMEV2_PLAIN_PERIOD 0x40 //明文+周期上报 
#define BTHOMEV2_ENCRYPT_PERIOD 0x41 //加密+周期上报
#define BTHOMEV2_PLAIN_EVENT_NONPERIOD  0x44 //明文+事件随机上报
#define BTHOMEV2_ENCRYPT_EVENT_NONPERIOD 0x45 //加密+事件随机上报


#define BTHOMEV2_40 BTHOMEV2_PLAIN_PERIOD
#define BTHOMEV2_41 BTHOMEV2_ENCRYPT_PERIOD
#define BTHOMEV2_44 BTHOMEV2_PLAIN_EVENT_NONPERIOD
#define BTHOMEV2_45 BTHOMEV2_ENCRYPT_EVENT_NONPERIOD



// complete local name , short name 
#define BLE_TYPENAMELONG 0x09 
#define BLE_TYPENAMESHORT 0x08


//bthome 发布顺序严格按照TYPE 从小到大
#define BTHOME_TYPE_TEXT 0x53  //文本  不需要字节序反转
#define BTHOME_TYPE_RAW 0x54 //raw数据  不需要字节序反转
#define BTHOME_TYPE_TEMPE02 0x02  //温度 uint16 采样值放大倍数 100  需要字节序反转
#define BTHOME_TYPE_HUMI03 0x03 //湿度 uint16 采样值放大倍数 100  需要字节序反转
#define BTHOME_TYPE_ILLUM 0x05 //光照 uint24 采样值放大倍数 100  需要字节序反转
#define BTHOME_TYPE_TIMESTAMP 0X50   //unix时间戳 uint32 不需要字节序反转
#define BTHOME_TYPE_VOL0C 0x0C  //电压 uint16  1000倍 反转
#define BTHOME_TYPE_PACKETID 0x00 //可选的 包id 用于去重
#define BTHOME_TYPE_DOOR 0x1A // 门二元开关 uint8 

#define BTHOME_TYPE_BUTTON 0x3A //按钮事件
#define BTHOME_TYPE_BUTTON_NONE 0x00   //按钮事件子类 悬空
#define BTHOME_TYPE_BUTTON_PRESS 0x01 //按钮事件 按一下
#define BTHOME_TYPE_BUTTON_DBPRESS 0x02 // 按双击
#define BTHOME_TYPE_BUTTON_TRLPRESS 0x03 //三击
#define BTHOME_TYPE_BUTTON_LPRESS 0x04  // 长按
#define BTHOME_TYPE_BUTTON_LDBPRESS 0x05 // 不懂 
#define BTHOME_TYPE_BUTTON_LTRLPRESS 0x06 // 理解不了
#define BTHOME_TYPE_BUTTON_HOLDPRESS 0x80 //按住 



bool bleadv_bthome_adddata_next(uint8_t* buff,uint8_t bufflen,uint8_t* offset,uint8_t id,const uint8_t* pdata,uint16_t datalen);


/*
bthome AES-128-CCM 加密报文 布局  

[2B UUID(LE)] [DevInfo(1B)] [密文ciphertext(N字节)] [counter(4B little-endian)] [MIC(4B)]

    MAC address 5448E68F80A5
    UUID D2FC
    BTHome device data byte 41
    Counter as bytes in little endian 33221100

    nonce = 5448e68f80a5d2fc4133221100

加密后的布局 Service Data = d2fc标识 , 41 info , e445f3c9962b 密文, 33221100 count, 6c7c4519 mic

*/


/*失败返回0, 成功返回pubff的长度
pk_remainlen ble包剩余长度 包括自己的要计算在内的ad头 2 +5 
pbuff 返回 ciphertext+count+mic 拼接  不包含 ad头 2 + 5 
*/
int bleadv_bthome2_payload_aesccm128_enc(const uint8_t* key,uint8_t* pbuff,uint16_t pk_remainlen,const uint8_t* payload,uint16_t payloadlen,const uint8_t* mac,uint16_t uuid,uint8_t devinfo,uint32_t count);

/*
payload 包含 ciphertext+count+mic 的部分 
*/
int bleadv_bthome2_payload_aesccm128_dec(const uint8_t* key,uint8_t* pbuff,uint32_t* out_count,const uint8_t* mac,const uint8_t* payload,uint16_t payloadlen,uint16_t uuid,uint8_t devinfo);


//void test_aesccm();

//////////////////////////////////////////////////////////////////////////

#define GAP_ADTYPE_FLAGS                        0x01  // Flags 
#define GAP_ADTYPE_16BIT_UUID_MORE              0x02  // 部分16bit服务UUID列表
#define GAP_ADTYPE_16BIT_UUID_COMPLETE          0x03  // 完整16bit服务UUID列表
#define GAP_ADTYPE_32BIT_UUID_MORE              0x04  // 部分32bit UUID
#define GAP_ADTYPE_32BIT_UUID_COMPLETE          0x05  // 完整32bit UUID
#define GAP_ADTYPE_128BIT_UUID_MORE             0x06  // 部分128bit UUID
#define GAP_ADTYPE_128BIT_UUID_COMPLETE         0x07  // 完整128bit UUID
#define GAP_ADTYPE_SHORT_NAME                   0x08  // 短设备名
#define GAP_ADTYPE_FULL_NAME                    0x09  // 完整设备名
#define GAP_ADTYPE_TX_POWER                     0x0A  // Tx功率，int8 dBm
#define GAP_ADTYPE_SERVICE_DATA_16              0x16  // 16bit UUID Service Data  bthome用这个
#define GAP_ADTYPE_APPEARANCE                   0x19  // Appearance外观图标
#define GAP_ADTYPE_ADV_INTERVAL                 0x1A  // 广播间隔
#define GAP_ADTYPE_SERVICE_DATA_32              0x20  // 32bit Service Data
#define GAP_ADTYPE_SERVICE_DATA_128             0x21  // 128bit Service Data
#define GAP_ADTYPE_MANUF_SPECIFIC_DATA          0xFF  // 厂商自定义数据  iBeacon用这个


typedef struct __attribute__((packed)){
    uint8_t adlen;  //接下来的ad段落长度,不含当前结构域,排除自己 
    uint8_t adtype;  
}BleAdvHdr;


#define BLE_GAP_FLAG_LIMITED_DISCOVERABLE       (1 << 0) // bit0 0x01 Limited
#define BLE_GAP_FLAG_GENERAL_DISCOVERABLE       (1 << 1) // bit1 0x02 General
#define BLE_GAP_FLAG_BREDR_NOT_SUPPORTED        (1 << 2) // bit2 0x04 BR/EDR不支持

// 组合
#define BLE_GAP_FLAGS_LIMITED              (BLE_GAP_FLAG_LIMITED_DISCOVERABLE | BLE_GAP_FLAG_BREDR_NOT_SUPPORTED)  //0x05
#define BLE_GAP_FLAGS_GENERAL              (BLE_GAP_FLAG_GENERAL_DISCOVERABLE | BLE_GAP_FLAG_BREDR_NOT_SUPPORTED)  //0x06


typedef struct __attribute__((packed)){
    BleAdvHdr base;
    uint8_t flag;
}BleAdvHdr_Flag;


typedef struct __attribute__((packed)){
    BleAdvHdr base;
    uint8_t name[];  //柔性数组
}BleAdvHdr_Name;



typedef struct __attribute__((packed)) {
    uint8_t adlen; //数据总长 不含自己
    uint8_t adtype; //**0x16 = Service Data - 16-bit UUID**
    uint16_t uuid; 
    uint8_t devinfo; 
}BleAdvHdr_Bthome;

/*
typedef struct __attribute__((packed)){
    BleAdvHdr base;
    union{
        uint16_t uuid; // 0xfcd2 (小端)
        struct{
            uint8_t uuid_low;  
            uint8_t uuid_high;
        }uuid_part;
    };

    union
    {
       uint8_t devinfo;
       struct{
#if !defined(IS_LITTLE_ENDIAN)
        uint8_t encrypt :1; //加密为1
        uint8_t reserve_1 :1;
        uint8_t trigger :1; //周期上报0 事件/随机上报1
        uint8_t reserve_2 :2;
        uint8_t version :3; //bthome版本2 
#else
        uint8_t version :3;
        uint8_t reserve_2 :2;
        uint8_t trigger :1;
        uint8_t reserve_1 :1;
        uint8_t encrypt :1;
#endif
       }devinfo_part;
    };
}BleAdvHdr_Bthome;
*/


typedef struct __attribute__((packed)){
    BleAdvHdr base;
    union{
        uint16_t uuid; // 0xfcd2 (小端)
        struct{
            uint8_t uuid_low;  
            uint8_t uuid_high;
        }uuid_part;
    };
}BleAdvHdr_UUID;   


/*
//调用示例 


uint8_t* pdata=(uint8_t*)test_packets[i];
uint8_t totallen=test_packet_len[i];

uint8_t offset=0;
BleAdvHdr* hdr=NULL;

uint16_t len;
int isenc,istrigger;
uint8_t* bthomepayload;

int count=0;

do{

    hdr=bleadv_segment_next(&pdata,totallen,&offset);
    if(hdr){
        if(bleadv_is_bthome((const BleAdvHdr_Bthome*)hdr)==false) continue;
        uint32_t count=0,tag=0;
        bthomepayload=bleadv_decode_bthome((const BleAdvHdr_Bthome*)hdr,&len,&isenc,&istrigger,&count,&tag);
        if(bthomepayload!=NULL){
           printf("bthome segment: len=%d isenc=%d istrigger=%d,count=%u,tag=%u\n",len,isenc,istrigger,count,tag);
            uint16_t pl_offset=0;

            uint8_t vtype,*value,valuelen;
            uint32_t factor;
            while( bleadv_bthome_payload_next(&bthomepayload,len,&pl_offset,&vtype,&value,&valuelen,&factor) ==true){
            if(vtype == 0x02 || vtype ==0x03)    
            {
           // uint16_t v = ( (uint16_t)value[0] ) | ( (uint16_t)value[1] << 8 ); //小端
           uint16_t v = bleadv_bthome_payload_getvalue16(value);

             float f= (float)v /factor;
             printf("value=%.2f\n",f);
            }
            
            if(vtype==0x53 ){
                char* str=(char*)value;
                printf("str=%s\n",str);
            }


            }

        }
    }
  ++count;
}while (hdr!=NULL);

printf("while count %d\n",count);

*/






//提取广播的segment,这个函数在循环中反复调用,内部自己计算偏移
BleAdvHdr* bleadv_segment_next(const uint8_t** pdata,uint8_t totallen,uint8_t* offset);

//判断当前ad是否为bthome的标识
bool bleadv_is_bthome(const BleAdvHdr_Bthome* pseg);

//判断ad为 小米的 MiBeacon
bool bleadv_is_mibeacon(const BleAdvHdr_UUID* pseg);

//google Eddystone
bool bleadv_is_eddystone(const BleAdvHdr_UUID* pseg);

//ATC_MiThermometer
bool bleadv_is_atc_mithermometer(const BleAdvHdr_UUID* pseg);

//sonoff 
bool bleadv_is_sonoff(const BleAdvHdr *pseg);

//iBeacon
bool bleadv_is_ibeacon(const BleAdvHdr *pseg);



/*
解析bthome的数据结构
pesg bthome结构的地址 通过 bleadv_segment_next获取 
payloadlen 输出参数 载荷长度,
isencrypt输出参数 为1加密为0明文,
istrigger 输出参数为0规律上报,为1不规律或事件驱动
count 输出参数 bthome报文计数 不加密情况 该值不使用  加密情况为uint32_t大小 ,
tag 输出参数 aes128ccm加密的tag值 
返回值 返回bthome载荷部分的地址,返回NULL失败
*/
uint8_t* bleadv_bthome_header(const BleAdvHdr_Bthome* pseg,uint16_t* payloadlen,uint8_t* version,uint8_t* isencrypt,uint8_t* istrigger,uint32_t* count,uint32_t* tag);


/*
解析bthome payload 
参考手册  https://bthome.io/format
这个半成品函数  没有实现定义的type 这个函数返回false 不再执行,因此在当前type后面附带识别的type也得不到执行
payload  载荷部分的起始地址,这个函数内部自己计算偏移
totallen payload的总长 用于检查内存越界
offset 输入输出参数 在循环中调用设置偏移量 首次调用设置为0为即可 内部自己计算
value 输出参数 指向payload的首地址 uint8* 
valuelen 输出参数 返回 value字节长度
factor 输出参数 返回 value需要减少的倍数 
返回值 true,输出参数是有效的,并且可以继续在循环中调用 直到返回false;返回false 输出参数的值是无效的  
完整的实现 https://github.com/Bluetooth-Devices/bthome-ble/blob/V2/tests/test_parser_v2.py
*/
bool bleadv_bthome_payload_next(const uint8_t* payload,uint16_t totallen,uint16_t* offset,uint8_t* type,uint8_t** value,uint8_t* valuelen,uint32_t* factor);


//将uint8* 指针的值 转换到16位 uint16_t
uint16_t bleadv_bthome_payload_getvalue16(const uint8_t* ptr);


//将uint8* 指针的值 转换到24位 保存为uint32_t,最高两字节为0x00
uint32_t bleadv_bthome_payload_getvalue24(const uint8_t* ptr);

//将uint8* 指针的值 转换到32位  uint32_t 
uint32_t bleadv_bthome_payload_getvalue32(const uint8_t* ptr);

/*
模板T类型 目的转换类型 通常为float , U类型 有符号还是无符号的整形,这关系到T作为float时的精度  
ptr 指向bthome读取到payload的地址
bit 只能是 8 16 24 32 就是uint8_t,uint16_t uint24_t ,uint32_t  
factor 缩小倍数

 */
template<typename T,typename U>
T bleadv_bthome_payload_getvalueT(const uint8_t* ptr,uint8_t bit,uint32_t factor){
/*
//speed 0x62
//62 40 99 DF FF 0.000001	-2.123456
uint8_t raw[] = {0x40,0x99,0xdf,0xff}; 
//uint8_t raw[]={0xff,0xdf,0x99,0x40};
int r=bleadv_bthome_payload_getvalue32(raw);
float f= (float)r /1000000;
float t2=bleadv_bthome_payload_getvalueT<float,int>(raw,32,1000000);

//温度 0x02
uint8_t raw2[]={0xCA,0x09};
float t3=bleadv_bthome_payload_getvalueT<float,int>(raw2,16,100);

//duration 0x42 0.001 	424E3400 	13.390
uint8_t raw3[]={0x4E,0x34,0x00};
float t4=bleadv_bthome_payload_getvalueT<float,int>(raw3,24,1000);

//timestamp 505d396164 
uint8_t raw4[]={0x5d,0x39,0x61,0x64};
uint32_t t5= bleadv_bthome_payload_getvalueT<uint32_t,uint32_t>(raw4,32,1);
*/

    T t;
    U u;
    switch(bit){
        case 8:
            u=(U)(ptr[0] & 0xFF);
        break;
        case 16:
            u=(U)bleadv_bthome_payload_getvalue16(ptr);
        break;
        case 24:
            u=(U)bleadv_bthome_payload_getvalue24(ptr);
        break;
        case 32:
            u=(U)bleadv_bthome_payload_getvalue32(ptr);
        break;    
        };
    t=(T)u;
    t/=factor;        
    return t;
}

// 用于 text 0x53 注意这个指针直接指向原始数据的地址,不是拷贝副本
const char* bleadv_bthome_payload_getraw(const uint8_t* str);

//raw 0x54  直接取其值即可


#endif //_MYBTHOME_H
