#include <mybthome.h>
#include <string.h>
#include <mbedtls/ccm.h>


bool bleadv_bthome_adddata_next(uint8_t* buff,uint8_t bufflen,uint8_t* offset,uint8_t id,const uint8_t* pdata,uint16_t datalen){

    if( buff==NULL || pdata==NULL) return false;

    if(*offset+datalen+1 > bufflen)
        return false; //+1字节id 空间不足，失败

    if(*offset>BTHOMEMAXPAYLOAD) return false;  //超过bthome最大载荷长度 

    uint8_t pos=*offset;
    buff[pos++]=id;
    
    //text和raw在id后面附加字节长度
    if(id == 0x53 || id ==0x54 )  
    {
        //正序拷贝 对于text和raw 
        if((datalen+2) > bufflen) return false; //空间不足  +2bytes type len
        buff[pos++]=datalen;  
    }
    
    for(uint16_t i=0; i<datalen; i++)
    {
        buff[pos++] = pdata[i];
    }

    *offset=pos;
    return true;
}

//////////////////////////////////////////////////////////////////////


int bleadv_bthome2_payload_aesccm128_enc(const uint8_t* key,uint8_t* pbuff,uint16_t pk_remainlen,const uint8_t* payload,uint16_t payloadlen,const uint8_t* mac,uint16_t uuid,uint8_t devinfo,uint32_t count){

    if(pbuff==NULL || key==NULL || payload==NULL || payloadlen<1 || mac==NULL  || payloadlen+8+5 > pk_remainlen || payloadlen > BTHOMEMAXPAYLOAD_AESCCM)
        return 0;

    uint8_t nonce[13];
    uint8_t tag[4];

    //mac
    memcpy(nonce,mac,6);
    // uuid
    memcpy(nonce+6,&uuid,2);
    // device data
    memcpy(nonce+8,&devinfo,1);
    //count
    memcpy(nonce+9,&count,4);    

/*
//counter 网络字节序
    uint8_t* pc=(uint8_t*)&count;
    for(int i=0;i<4;i++)
        nonce[6+i]=pc[3-i];
*/
    
    mbedtls_ccm_context ctx;
    mbedtls_ccm_init(&ctx);
    int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, 128);
    if(ret != 0 )
    {
        ret = 0;
        goto _func_exit;
    }
    ret = mbedtls_ccm_encrypt_and_tag(&ctx,payloadlen,nonce,sizeof(nonce),NULL,0,payload,pbuff,tag,sizeof(tag));
    if(ret!=0){
        ret=0;
        goto _func_exit;
    }
    ret=payloadlen;

    memcpy(pbuff+payloadlen,&count,4);
    ret+=4;
    memcpy(pbuff+payloadlen+4,tag,4);
    ret+=4;

_func_exit:
    mbedtls_ccm_free(&ctx);
    return ret;
}


int bleadv_bthome2_payload_aesccm128_dec(const uint8_t* key,uint8_t* pbuff,uint32_t* out_count,const uint8_t* mac,const uint8_t* payload,uint16_t payloadlen,uint16_t uuid,uint8_t devinfo){

    if(pbuff==NULL ||payload==NULL||mac==NULL)
      return 0;

    uint16_t cipherlen=payloadlen-8;  //密文长度

    if(cipherlen > BTHOMEMAXPAYLOAD_AESCCM || cipherlen < 2 ) {
        mylog("cipherlen=%u,payloadlen=%u\n",cipherlen,payloadlen);
        return 0; //最小2字节传感器数据
    }

    uint8_t* cipher=(uint8_t*)payload;

    uint8_t* tag=(uint8_t*)(payload+cipherlen+4);  

    uint32_t _count;
    memcpy(&_count,cipher+cipherlen,4);
/*
nonce = mac 6 + uuid 2 + devdata 1 + count 4
*/
    uint8_t nonce[13];
    memcpy(nonce,mac,6);
    memcpy(nonce+6,&uuid,2);
    memcpy(nonce+6+2,&devinfo,1);
    memcpy(nonce+9,&_count,4);

    mbedtls_ccm_context ctx;
    mbedtls_ccm_init(&ctx);
    int ret = mbedtls_ccm_setkey(&ctx,MBEDTLS_CIPHER_ID_AES,key,128);
    if(ret !=0) { mylog("bleadv_bthome2_payload_aesccm128_dec,mbedtls_ccm_setkey, ret=%d\n",ret);  
        ret=0; goto _func_exit;
    }

    ret=mbedtls_ccm_auth_decrypt(&ctx,cipherlen,nonce,sizeof(nonce),NULL,0,cipher,pbuff,tag,4);
    if(ret !=0) { 
        mylog("bleadv_bthome2_payload_aesccm128_dec,mbedtls_ccm_auth_decrypt,failed ret=%d\n",ret); 
        mylog("cipherlen=%u  count=0x%08x tag=%x%x%x%x\n",cipherlen,_count,tag[0],tag[1],tag[2],tag[3]);
        ret=0; 
        goto _func_exit;
    }
    if(out_count) *out_count=_count;
    ret=cipherlen;
    _func_exit:
    mbedtls_ccm_free(&ctx);
    return ret;
}


////////////////////////////////////////////////////

BleAdvHdr* bleadv_segment_next(const uint8_t** pdata,uint8_t totallen,uint8_t* offset){
    
    if(pdata ==NULL || totallen==0 ||totallen>BLEADVMAXLEN+1 || offset==NULL ) return NULL;
   
    uint8_t remain=totallen - (*offset);
    if(remain < sizeof(BleAdvHdr)+1) return NULL; //adhdr minlen 3 

    BleAdvHdr* hdr=(BleAdvHdr*)*pdata;
    if(hdr->adlen==0 || hdr->adlen +1 > remain) return NULL;

    uint16_t seglen=hdr->adlen +1;
  
    *offset+=seglen;
    *pdata=*pdata+seglen;
    return hdr;
}


bool bleadv_is_bthome(const BleAdvHdr_Bthome* pseg){
   /* if(pseg->base.adlen < 7) return false; // bthome长度5字节头+最小2字节载荷 
   // if(pseg->base.adtype == 0x16 && pseg->uuid_part.uuid_low== 0xd2 && pseg->uuid_part.uuid_high==0xfc)
    if(pseg->uuid==BTHOMEUUID)
        return true;*/

    if(pseg->adlen > 6 && pseg->adtype == 0x16 && pseg->uuid == BTHOMEUUID)  // 4字节头(adlen不含自己长度)+最小2字节载荷 这个判断不允许空载荷
        return true;
    return false;    
}

bool bleadv_is_mibeacon(const BleAdvHdr_UUID* pseg){
    //if(pseg->base.adtype == 0x16 && pseg->uuid == 0xfe95 && pseg->uuid_part.uuid_high == 0xfe)
    if(pseg->base.adtype == 0x16 && pseg->uuid == 0xfe95)
        return true;
    return false;
}

bool bleadv_is_eddystone(const BleAdvHdr_UUID* pseg){
    //if(pseg->base.adtype == 0x16 && pseg->uuid_part.uuid_low == 0xaa && pseg->uuid_part.uuid_high == 0xfe)
    if(pseg->base.adtype == 0x16 && pseg->uuid ==0xfeaa)
        return true;
    return false;
}

bool bleadv_is_atc_mithermometer(const BleAdvHdr_UUID* pseg){
    //if(pseg->base.adtype == 0x16 && pseg->uuid_part.uuid_low == 0x1a && pseg->uuid_part.uuid_high == 0x18)
    if(pseg->base.adtype == 0x16 && pseg->uuid == 0x181a)
        return true;
    return false;
}

bool bleadv_is_sonoff(const BleAdvHdr *pseg){
    if(pseg->adlen>4){
        if(pseg->adtype==0xFF){
            uint16_t id = *(const uint16_t *)((const uint8_t*)pseg + sizeof(BleAdvHdr));
            if(id== 0x07D0 ) //0xD0,0x07
            return true;
        }
    }
    return false;
}

bool bleadv_is_ibeacon(const BleAdvHdr *pseg){
    if(pseg->adlen>6){
        if(pseg->adtype==0xFF){
            const uint16_t *id = (const uint16_t *)((const uint8_t*)pseg + sizeof(BleAdvHdr));
            if(*id== 0x004C && *(uint16_t*)(id+2) == 0x1502) 
            return true;
        }
    }
    return false;    
}




//如果是BTHOME 返回bthome的载荷 否则返回NULL,参数len返回载荷长度  
uint8_t* bleadv_bthome_header(const BleAdvHdr_Bthome* pseg,uint16_t* payloadlen,uint8_t* version,uint8_t* isencrypt,uint8_t* istrigger,uint32_t* count,uint32_t* tag){
    //存在缺陷 adlen 小于 实际数据 依然能返回  但是数据不全  在接下来的数据区获取是灾难,大于可以被条件判断到,小于无法因为如果bthome数据段处于ad区域的中间:ad1 bthome ad2 ad3 
    //uint16_t _len=pseg->base.adlen - sizeof(BleAdvHdr_Bthome)+1; //adlen不包含自己 

    uint16_t _len=pseg->adlen - sizeof(BleAdvHdr_Bthome)+1; //adlen不包含自己 
    uint8_t _isencrypt=pseg->devinfo & 0x1;
        
    //小端 
    if(isencrypt) *isencrypt= _isencrypt;
    if(istrigger) *istrigger= (pseg->devinfo >> 2) & 0x1;
    if(version) *version=(pseg->devinfo >> 5) & 0x07;
    
    
    uint8_t* payload=(uint8_t*)pseg+sizeof(BleAdvHdr_Bthome); //bthome头部固定开销 5字节
    if(_isencrypt==0){
        if(_len<2) return NULL; // 这里假定 至少 2bytes的传感器数据 没使用count
        if(payloadlen) *payloadlen=_len;
        if(count) *count=0; //不加密不使用 输出参数被赋值为0
        if(tag) *tag=0; //不使用
    }else{
        //加密
        if(_len < 10) return NULL;  //加密的 假定 至少2byte传感器数据 + 8bytes nonce+tag 
        if(payloadlen) *payloadlen=_len; //输出的payloadlen包含 data+nonce+tag 不仅是data
        if(count) memcpy(count,(uint8_t*)(payload + (_len - 8)),4);
        if(tag) memcpy(tag,(uint8_t*)(payload + (_len - 4)),4);
    }
    return payload;
}


template<typename T>
inline void setprtval(T* p, T v){
if(p!=NULL)
    *p=v;
}


/*
部分解析bthome payload 不认识的停止解析 ,offset初次调用设置为0 
返回值 如果为false 在循环中本次循环调用 输出参数值是无效的 因为溢出total了 
!!! 这个半成品函数  没有实现定义的type 这个函数返回false 不再执行,因此在当前type后面附带识别的type也得不到执行
*/
bool bleadv_bthome_payload_next(const uint8_t* payload,uint16_t totallen,uint16_t* offset,uint8_t* type,uint8_t** value,uint8_t* valuelen,uint32_t* factor){
    if(payload==NULL || offset==NULL ) return false;

    //mylog("----------offset=%u,total=%u\n",*offset,totallen);
    //if(*offset>totallen) return false;
    if(*offset + 2 > totallen ) return false; // type+value 最小2字节

    payload+= *offset;

    uint8_t _type= payload[0];

    *value= (uint8_t*)(payload+1); //直接指向地址 
    setprtval<uint8_t>(type,_type);
    //*type=_type;

//type严格从小到大 为了新旧设备的兼容性     
    switch(_type){
        case BTHOME_TYPE_PACKETID:
            setprtval<uint8_t>(valuelen,1);
            setprtval<uint32_t>(factor,1);
            *offset+=2;
        break;

        case BTHOME_TYPE_TEMPE02: // temperature
        case BTHOME_TYPE_HUMI03: //humidity
            setprtval<uint8_t>(valuelen,2); //2 bytes
            setprtval<uint32_t>(factor,100);
            *offset+=3;
        break;

        case BTHOME_TYPE_ILLUM:  // illuminance
            setprtval<uint8_t>(valuelen,3); //3 bytes
            setprtval<uint32_t>(factor,100);
            *offset+=4;
        break;

        case BTHOME_TYPE_TIMESTAMP: //timestamp
            setprtval<uint8_t>(valuelen,4); //4 bytes
            setprtval<uint32_t>(factor,1); //不需要倍数
            *offset+=5;
        break;

        case BTHOME_TYPE_TEXT: //text
        case BTHOME_TYPE_RAW: //raw
            setprtval<uint8_t>(valuelen,payload[1]);
            setprtval<uint32_t>(factor,1);
            *offset+=payload[1]+1;
            *value= (uint8_t*)(payload+2); //偏移+1byte
        break;   

        case BTHOME_TYPE_VOL0C: //voltage
            setprtval<uint8_t>(valuelen,2);
            setprtval<uint32_t>(factor,1000);            
            *offset+=3;
        break;

        default:
        //大部分没有使用 没有实现 ,直接退出函数 
        return false; 
        break;
    };
    
    //偏移量超过总长
    if(*offset > totallen) return false;
    return true;
}

//将uint8* 指针的值 转到16位 uint16_t
uint16_t bleadv_bthome_payload_getvalue16(const uint8_t* ptr){
    //小端
    uint16_t ret=ptr[1]; 
    ret = ret <<8 | ptr[0];
    /*
    float f= (float)ret /factor;
    printf("value=%.2f\n",f);
    */
    return ret;
}

//将uint8* 指针的值 转到24位 保存为uint32_t,最高两字节为0x00
uint32_t bleadv_bthome_payload_getvalue24(const uint8_t* ptr){
    //小端
    uint32_t ret = ptr[2];
    ret <<= 8;
    ret |= ptr[1]; 
    ret <<=8;
    ret|=ptr[0];
    return ret ;
}


uint32_t bleadv_bthome_payload_getvalue32(const uint8_t* ptr){
    //小端
    uint32_t ret = ptr[3];
    ret <<= 8;
    ret |= ptr[2]; 
    ret <<=8;
    ret|=ptr[1];
    ret <<=8;
    ret |=ptr[0];
    return ret ;
}



const char* bleadv_bthome_payload_getraw(const uint8_t* str){
    /*
    //0x53 text 530C48656C6C6F 20576F726C6421
    const char* str="Hello World!";
    uint8_t raw5[]={0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64,0x21};
    const char* s=bleadv_bthome_payload_getraw(raw5);
    printf("%s\n",s);
    */
    return (const char*)(str);
}

