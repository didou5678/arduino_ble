#include <mybase.h>
#include <mytimer.h>
#include <ble_bthome_advert.h>
#include <mybthome.h>
#include <mysysinfo.h>

//#define ENABLE_RANDOM_COUNT

#define DEVICE_NAME "test"
NimBLEAdvertising *padving;
NimBLEAdvertisementData advData;
NimBLEAddress ble_mac;


/*
  BLEScanFilter_AESKeyTable::instance.add(
    {0xdd,0x21,0xd2,0x55,0x05,0x55},
    { 0x27,0x22,0x4e,0x93,0x87,0x57,0x0c,0x4b,0xc9,0xdf,0xdc,0x5f,0x81,0xcd,0x39,0xe3} );

  BLEScanFilter_AESKeyTable::instance.add(
  {0xc9,0xf5,0xdf,0xff,0x52,0x83},
  { 0xc8,0xb1,0x60,0x1c,0xd0,0xe6,0x84,0x1f,0x73,0x4c,0x7f,0x2b,0x30,0xac,0x12,0xaf} );

  BLEScanFilter_AESKeyTable::instance.add(
    {0xe0,0x3c,0x27,0xb5,0x4d,0xf6},
    { 0xff,0xa9,0x33,0x35,0x2d,0xa0,0x1b,0x44,0xae,0x31,0x7e,0x00,0xd8,0x39,0xd7,0xf1});

   BLEScanFilter_AESKeyTable::instance.add(
    {0xc0,0x0b,0x01,0x08,0xc5,0x21},
    { 0xa9,0x3e,0x39,0xb7,0x03,0x22,0xc6,0xeb,0xab,0x4c,0xb9,0x99,0x08,0x27,0x80,0x7a});
*/


typedef struct {
 uint8_t mac[6];
 uint8_t aeskey[16];
}macaeskey;



//转换为 ha 可添加的bindkey
//echo  '0x23,0x1d,0x39,0xc1,0xd7,0xcc,0x1a,0xb1,0xae,0xe2,0x24,0xcd,0x09,0x6d,0xb9,0x32' | sed -e 's/0x//g' -e 's/,//g' -e 's/ //g'


macaeskey mackey[4]= {

 {
    { 0xc7, 0x2c, 0x79, 0xa5, 0x0c, 0x95 },
    { 0xdb, 0x8c, 0x5d, 0x54, 0xe6, 0x22, 0x54, 0x80, 0x6d, 0x3a, 0x1d, 0x10, 0x2b, 0x24, 0xe2, 0xb3 } 
 },

{
{ 0xc2, 0x40, 0x02, 0xc7, 0xf7, 0x3a },
{ 0x17, 0x58, 0x6e, 0x52, 0xfc, 0x43, 0x3a, 0x1e, 0x43, 0xf0, 0xea, 0x3b, 0x8d, 0x67, 0xe2, 0x90 }
},

{
   { 0xc9, 0x90, 0xec, 0x97, 0x91, 0xf9 },
{ 0x28, 0x3f, 0x30, 0x86, 0x01, 0x67, 0x44, 0x4b, 0xb8, 0x41, 0xbf, 0xfb, 0x73, 0xc7, 0x5b, 0x68 } 
},

{
 { 0xf3, 0x73, 0x0f, 0x08, 0xf6, 0x7b },
{ 0x42, 0xc2, 0xd3, 0xb9, 0x66, 0x69, 0x5a, 0x60, 0x49, 0xce, 0x99, 0xb6, 0x11, 0x6e, 0x10, 0xe4 } 
}


};


volatile bool gosleep=false;

RTC_DATA_ATTR uint32_t bth_counter = 0;

const uint32_t sendduration=3000;


// openssl rand 16 | xxd -i 密钥产生 测试用途 


//广播完成回调
void advCompleteCallback(NimBLEAdvertising *pAdv);





 void sendbleadv(const uint8_t* mac,uint8_t* aeskey){

mylog("sendbleadv\n");

float t=temperatureRead();
mylog("Chip Temp: %.2f °C\n", t);

float h = random(40,76) + (random(0,10)/10.0f);
mylog("Humidity: %.2f %%RH\n", h);

//05 138A14 13460.67 
//illuminance (uint24, 单位 0.01 lx)**，最大值 167772.15 lx
//发送时放大100倍 小数占1字节 共3字节 接收时缩小100倍
float lx = random(0, 5000L) + random(1,100)/100.0f;
mylog("illuminance: %.2f lx\n",lx);
 
int volraw = random(200, 500);
float vol = volraw/100.0f;
mylog("voltage: %.2fv\n",vol );


uint16_t t16=(uint16_t)lroundf(t * 100.0f); 

//uint16_t t16=(uint16_t)lroundf(25.06 * 100.0f); 
uint16_t h16=(uint16_t)lrintf(h*100.0f);

//发送时放大100倍 小数占1字节 共3字节 接收时缩小100倍
//uint32_t lx24=(uint32_t)lrintf(lx*100.00f);
uint32_t lx24=0x148a13; //13460.67
lx24 &= 0x00FFFFFF;

uint16_t vol16=(uint16_t) lrintf(vol*1000.0f);

mylog("%x ,%x\n",t16,h16);

uint8_t buff[BTHOME_PAYLOADMAXLEN];
uint16_t bufflen=BTHOME_PAYLOADMAXLEN;
uint8_t offset=0;

bleadv_resetdata(&advData);

if(bleadv_bthome_adddata_next(buff,BTHOME_PAYLOADMAXLEN,&offset,BTHOME_TYPE_TEMPE02,(const uint8_t*)&t16,sizeof(uint16_t)))
    if(bleadv_bthome_adddata_next(buff,BTHOME_PAYLOADMAXLEN,&offset,BTHOME_TYPE_HUMI03,(const uint8_t*)&h16,sizeof(uint16_t)))
        if(bleadv_bthome_adddata_next(buff,BTHOME_PAYLOADMAXLEN,&offset,BTHOME_TYPE_ILLUM,(const uint8_t*)&lx24,3))
            bleadv_bthome_adddata_next(buff,BTHOME_PAYLOADMAXLEN,&offset,BTHOME_TYPE_VOL0C,(const uint8_t*)&vol16,2);



mylog("offset=%u\n",offset);

if(bth_counter % 2 == 0 ){

//if(bth_counter % 2 <= 0xffffff){

    //不加密 
 if(bleadv_bthome2_setdata(&advData,BTHOMEV2_40,buff,offset))
     if (bleadv_setdata(padving,&advData,advCompleteCallback))
        if(bleadv_start(padving,sendduration))
            mylog("ble adv send\n");
}else{
    //加密发送 
mylog("aes enc------------\n");
//const uint8_t *mac =ble_mac.getVal();

uint8_t ciphertext[BTHOMEMAXPAYLOAD_AESCCM];

int ret = bleadv_bthome2_payload_aesccm128_enc(aeskey,ciphertext,bleadv_remainlen(&advData),buff,offset,mac,BTHOMEUUID,BTHOMEV2_41,bth_counter);

if(ret ==0){
    mylog("aes enc error==========================\n");
    return;
}

myloghex(ciphertext,ret);

 if(bleadv_bthome2_setdata(&advData,BTHOMEV2_41,ciphertext,ret))
     if (bleadv_setdata(padving,&advData,advCompleteCallback))
        if(bleadv_start(padving,sendduration))
            mylog("ble adv send\n");    
 }

}



void sendbleadv_text(const uint8_t* mac,uint8_t* aeskey){

    mylog("sendbleadv_text\n");

uint8_t buff[BTHOME_PAYLOADMAXLEN];
uint16_t bufflen=BTHOME_PAYLOADMAXLEN;
uint8_t offset=0;

char randstr[11];
random_string(randstr,11,10);

bleadv_resetdata(&advData);


if(bleadv_bthome_adddata_next(buff,BTHOME_PAYLOADMAXLEN,&offset,BTHOME_TYPE_TEXT,(const uint8_t*)randstr,10))

if(bth_counter  % 2 == 0 ){

//if(bth_counter % 2 >= 0xffffff ){

    //不加密 
 
 //添加 name
 advData.setName(DEVICE_NAME,true);
    
 if(bleadv_bthome2_setdata(&advData,BTHOMEV2_40,buff,offset))
     if (bleadv_setdata(padving,&advData,advCompleteCallback))
        if(bleadv_start(padving,sendduration))
            mylog("ble adv send\n");
}else{
    //加密发送 
mylog("aes enc---------------\n");

//如果使用随机mac 不能调用 nimble系列 获取
//const uint8_t *mac =ble_mac.getVal();


uint8_t ciphertext[BTHOMEMAXPAYLOAD_AESCCM];
int ret=
 bleadv_bthome2_payload_aesccm128_enc(aeskey,ciphertext,bleadv_remainlen(&advData),buff,offset,mac,BTHOMEUUID,BTHOMEV2_41,bth_counter);
if(ret ==0){
    mylog("aes enc error=========================\n");
    return ;
}

 if(bleadv_bthome2_setdata(&advData,BTHOMEV2_41,ciphertext,ret))
     if (bleadv_setdata(padving,&advData,advCompleteCallback))
        if(bleadv_start(padving,sendduration))
            mylog("ble adv send\n");    
}

}

void sendbleadv_timestamp(const uint8_t* mac,uint8_t* aeskey){

    mylog("sendbleadv_timestamp\n");

 uint32_t tm=1700748499;
 long step=random(0x0FFFFFFF);

 tm+= step;
 
 mylog("tm=0x%x\n",tm);

uint8_t buff[BTHOME_PAYLOADMAXLEN];
uint16_t bufflen=BTHOME_PAYLOADMAXLEN;
uint8_t offset=0;


bleadv_resetdata(&advData);



bool ok=bleadv_bthome_adddata_next(buff,BTHOME_PAYLOADMAXLEN,&offset,BTHOME_TYPE_TIMESTAMP,(const uint8_t*)&tm,sizeof(uint32_t));
if(ok==false) return;

mylog("tm offset=%u\n",offset);

if(bth_counter % 2 == 0 ) {
//if(bth_counter % 2 >= 0xffffff ){


    //不加密 
    advData.setName("timestamp",true);
    ok=bleadv_bthome2_setdata(&advData,BTHOMEV2_40,buff,offset);
}
else{
    //加密
    //const uint8_t *mac =ble_mac.getVal();

    mylog("aes enc--------------\n");

    uint8_t ciphertext[BTHOMEMAXPAYLOAD_AESCCM];
    int ret= bleadv_bthome2_payload_aesccm128_enc(aeskey,ciphertext,bleadv_remainlen(&advData),buff,offset,mac,BTHOMEUUID,BTHOMEV2_41,bth_counter);
    if(ret == 0){
        mylog("aes enc error=========================\n");
        return;  
    }

    if(bleadv_bthome2_setdata(&advData,BTHOMEV2_41,ciphertext,ret))
        if (bleadv_setdata(padving,&advData,advCompleteCallback))
            if(bleadv_start(padving,sendduration))
                mylog("ble adv send\n");    

    }

 
}  


//二元传感器上报 
void sendbleadv_binarysensor(const uint8_t* mac,uint8_t* aeskey){

    mylog("sendbleadv_binarysensor\n");

    uint8_t buff[BTHOME_PAYLOADMAXLEN];
    uint16_t bufflen=BTHOME_PAYLOADMAXLEN;
    uint8_t offset=0;


    //随机 门开关状态
    uint8_t door_state= random() % 1;
  
    //随机packetid 正常应该每次++
    uint8_t pid=(uint8_t)random(0,256);

    bleadv_resetdata(&advData);



//添加可有可无的packetid 
if(bleadv_bthome_adddata_next(buff,BTHOME_PAYLOADMAXLEN,&offset,BTHOME_TYPE_PACKETID,(const uint8_t*)&pid,sizeof(uint8_t)))    
    if(bleadv_bthome_adddata_next(buff,BTHOME_PAYLOADMAXLEN,&offset,BTHOME_TYPE_DOOR,(const uint8_t*)&door_state,sizeof(uint8_t)) ==false)    
        return;

 if(bth_counter % 2 == 0 ){

 //if(bth_counter % 2 >= 0xffffff ){

   //不加密 
advData.setName("bigdoor");

  if(bleadv_bthome2_setdata(&advData,BTHOMEV2_40,buff,offset))
     if(bleadv_setdata(padving,&advData,advCompleteCallback))
        if(bleadv_start(padving,sendduration))
            mylog("ble adv send\n");   
 }else{
    mylog("aes enc-----------\n");
        uint8_t ciphertext[BTHOMEMAXPAYLOAD_AESCCM];
        int ret= bleadv_bthome2_payload_aesccm128_enc(aeskey,ciphertext,bleadv_remainlen(&advData),buff,offset,mac,BTHOMEUUID,BTHOMEV2_41,bth_counter);
        if(ret ==0){
            mylog("aes enc error=====================\n");
            return;
        }    

      if(bleadv_bthome2_setdata(&advData,BTHOMEV2_41,ciphertext,ret))
     if (bleadv_setdata(padving,&advData,advCompleteCallback))
        if(bleadv_start(padving,sendduration))
            mylog("ble adv send\n");          
 }    
 

}

//随机事件上报 
void sendbleadv_event_button(const uint8_t* mac,uint8_t* aeskey){

 mylog("sendbleadv_event_button\n");

uint8_t buff[BTHOME_PAYLOADMAXLEN];
uint16_t bufflen=BTHOME_PAYLOADMAXLEN;
uint8_t offset=0;

bleadv_resetdata(&advData);


//随机事件
uint8_t buttonid=random()% 0x08;
if(buttonid == 0x07) buttonid=0x80;
//加密
    if(bleadv_bthome_adddata_next(buff,BTHOME_PAYLOADMAXLEN,&offset,BTHOME_TYPE_BUTTON,(const uint8_t*)&buttonid,sizeof(uint8_t)) ==false)    
    return;

uint8_t ciphertext[BTHOMEMAXPAYLOAD_AESCCM];    
int ret= bleadv_bthome2_payload_aesccm128_enc(aeskey,ciphertext,bleadv_remainlen(&advData),buff,offset,mac,BTHOMEUUID,BTHOMEV2_45,bth_counter);
if(ret ==0){
    mylog("aes enc error=====================================\n");
    return;
}

 if(bleadv_bthome2_setdata(&advData,BTHOMEV2_45,ciphertext,ret))
     if (bleadv_setdata(padving,&advData,advCompleteCallback))
        if(bleadv_start(padving,sendduration))
            mylog("ble adv send\n");    
}    

void test(int i){

    pinMode(8,OUTPUT);
    digitalWrite(8,LOW); //蓝灯常亮

    mylog("----------------------\n");


    long s = random(256)%5;

    switch(s){

    case 0:    
        sendbleadv(mackey[i].mac,mackey[i].aeskey);  
    break;
    case 1:     
        sendbleadv_text(mackey[i].mac,mackey[i].aeskey);
    break;
    case 2:
        sendbleadv_timestamp(mackey[i].mac,mackey[i].aeskey);
    break;    
    case 3:
        sendbleadv_binarysensor(mackey[i].mac,mackey[i].aeskey);
    break;

    case 4:
        sendbleadv_event_button(mackey[i].mac,mackey[i].aeskey);
    break;
    }

    #ifdef ENABLE_RANDOM_COUNT
    //启动随机计数器 用于测试 aes policy
    bth_counter=(uint32_t)random(); 
    #else
    ++bth_counter; 
    #endif    
}


void setup() {
  serial_begin();

    int i=random(0,3); 
    set_esp32_macaddr(2,mackey[i].mac);

    padving=bleadv_initdev(nullptr);
    if(padving==nullptr){
        mylog("ble_advdev_init failed\n");
        gosleep=true;
        return;
    }

    ble_mac = NimBLEDevice::getAddress();
    mylog("ble mac: %s\n",ble_mac.toString().c_str());

  test(i);

}

MyTimer timer1=15*1000;

void loop() {

 
if(timer1) {
   digitalWrite(8,HIGH); //蓝灯关闭
    NimBLEDevice::deinit(true);
    delay(5);
    esp32_deepsleep(10);

    //测试 不休眠
    //test();


    
    }
        


if(gosleep){
      //其他业务代码
/*
   digitalWrite(8,HIGH); //蓝灯关闭 
   delay(3000);

   NimBLEDevice::deinit(false);
   test();*/

}

loop_idle();
}



void advCompleteCallback(NimBLEAdvertising *pAdv){
    mylog("BLE Advertising finished\n");
    gosleep=true;
}
