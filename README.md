# bicycle-radar
Arduino, XIAO ESP32-C3, LD2410 24GHz presence sensor, Buzzer, TM1637

To prepare an app on your Android phone to monitor the change in distance value as you ride your bike.

1) ESP32
- start with LD2410 Arduino example code 
- needs BLE server example code from https://wiki.seeedstudio.com/XIAO_ESP32C3_Bluetooth_Usage/ and notifying code from somewhere. 
- ESP-IDF BLE gatt_server is excellent but too complex and no need to use that. 

2) Android BluetoothLeGatt app
- https://github.com/android/connectivity-samples/tree/main/BluetoothLeGatt (Android SDK 28) 
- Above needs some modification for recent SDK(https://stackoverflow.com/questions/37423199/bluetooth-le-gatt-not-finding-any-devices)

