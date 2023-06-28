# bicycle-radar
Arduino, XIAO ESP32-C3, LD2410 24GHz presence sensor, Buzzer, TM1637

Unexpeced approaching behind when riding frightened me sometimes, although ready made products are in the market, I made bicycle radar demo.  

Limitations 

- LD2410 covers 4~5m, so it can miss fast followers.
- It's noisy if running next to an alley or a bridge railing because it will detect that object as moving object(actually sensor is moving).  

To prepare an app on Android phone to monitor the change in distance value when riding bike.

1) ESP32
- start with LD2410 Arduino example code 
- needs BLE server example code from https://wiki.seeedstudio.com/XIAO_ESP32C3_Bluetooth_Usage/ and notifying code from somewhere. 
- ESP-IDF BLE gatt_server is excellent but too complex and no need to use that. 

2) Android BluetoothLeGatt app
- https://github.com/android/connectivity-samples/tree/main/BluetoothLeGatt (Android SDK 28) 
- Above needs some modification for recent SDK(https://stackoverflow.com/questions/37423199/bluetooth-le-gatt-not-finding-any-devices)

