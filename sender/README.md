## 硬件
- ESP32C3

## 功能
- 使用 minble 实现 低功耗蓝牙广播发送 bthomev2消息 用于测试发送

## 技术栈
- PlatformIO
- framework = arduino（espressif32 Arduino Core）

## mylibs 模块功能
模块	功能
- ble_bthome_advert	BLE 广播初始化、启停、广播包加载
- mybthome	BTHome 数据对象组装、编码、AES 加密
- mybase	通用辅助函数
- mysysinfo	获取芯片 ID、可用堆、版本信息
- mytimer	用于loop函数的定时器


