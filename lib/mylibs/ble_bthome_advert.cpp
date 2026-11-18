#include <ble_bthome_advert.h>




NimBLEAdvertising* bleadv_initdev(const char* advname){
    if(NimBLEDevice::isInitialized()==true){
        return NimBLEDevice::getAdvertising();
    }
  
    if(NimBLEDevice::init(advname==nullptr ? "" : advname)==false) return nullptr;        
    mylog("ble_advinit: NimBLEDevice name=%s\n",NimBLEDevice::toString().c_str());
    return NimBLEDevice::getAdvertising();
}

void bleadv_resetdata(NimBLEAdvertisementData* padvData){
    if(padvData==NULL) return;
    padvData->clearData();
    padvData->setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
}



size_t bleadv_setdata(NimBLEAdvertising* padv,NimBLEAdvertisementData* padvData,advCompleteCB_t pfunc,uint16_t MinInterval,uint16_t MaxInterval){
    
    if(!padvData || !padv ) return 0;

    if(padv->isAdvertising()) {
        padv->stop();
    }

    padv->setMinInterval(MinInterval);
    padv->setMaxInterval(MaxInterval);

    if(padv->setAdvertisementData(*padvData)==false) {
        mylog("ble_adving_conf:  setAdvertisementData() failed");
        return 0;
    }
    padv->setAdvertisingCompleteCallback(pfunc);
    
    return bleadv_datalen(padvData);
}

size_t bleadv_datalen(NimBLEAdvertisementData* padvData){
    if(padvData==NULL) return 0;
    return padvData->getPayload().size();
}

size_t bleadv_remainlen(NimBLEAdvertisementData* padvData){
    return LEGACY_BLEADV_MAXLEN - bleadv_datalen(padvData);
}


void print_advdata(const NimBLEAdvertisementData* padvData){
    auto raw=padvData->getPayload();
    myloghex(raw.data(),raw.size());
}

bool bleadv_bthome2_setdata(NimBLEAdvertisementData* padvData,uint8_t devinfo,uint8_t* payload,uint16_t payloadlen){

    if(padvData==NULL || payload==NULL) return false;
    if(LEGACY_BLEADV_MAXLEN <  bleadv_datalen(padvData)) return false;

    size_t remainsize=bleadv_remainlen(padvData);

    if( remainsize < payloadlen + 5 ) return false; // 5字节bthome hdr 


    // bthome payload
    uint8_t bthome_payload[BTHOME_PAYLOADMAXLEN+1];
    bthome_payload[0]=devinfo;

    /*
    //未加密的
    if((devinfo & 0x1) == 0){
        bthome_payload[1]=counter & 0xFFU;
        plugs+=1;
    }*/

    memcpy(bthome_payload+1,payload,payloadlen);
    bool ret=padvData->setServiceData(NimBLEUUID(BTHOMEUUID),bthome_payload,payloadlen+1);    

   
    #ifdef MYPRINT
    print_advdata(padvData);
    mylog("ble_bthome_build_advdata: setServiceData ret=%d\n",ret );
    #endif 

    return ret;
}


bool bleadv_start(NimBLEAdvertising *padving,uint32_t duration){
    if(padving){
        padving->start(duration);
    }
    return false;
}
