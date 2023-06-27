#include <ld2410.h>
#include <TM1637Display.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "05194ca5-b9b0-44fe-9c3c-2565a2e93d24"
#define CHARACTERISTIC_UUID "0a83277d-3d43-4bbe-b75e-70dbd3927819"

#if defined(ESP32)
  #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
    #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
      #define MONITOR_SERIAL Serial
      #define RADAR_SERIAL Serial1
      #define RADAR_RX_PIN 32
      #define RADAR_TX_PIN 33
    #elif CONFIG_IDF_TARGET_ESP32S2
      #define MONITOR_SERIAL Serial
      #define RADAR_SERIAL Serial1
      #define RADAR_RX_PIN 9
      #define RADAR_TX_PIN 8
    #elif CONFIG_IDF_TARGET_ESP32C3
      #define MONITOR_SERIAL Serial
      #define RADAR_SERIAL Serial1
      #define RADAR_RX_PIN 5   // D3
      #define RADAR_TX_PIN 4   // D2 
    #else 
      #error Target CONFIG_IDF_TARGET is not supported
    #endif
  #else // ESP32 Before IDF 4.0
    #define MONITOR_SERIAL Serial
    #define RADAR_SERIAL Serial1
    #define RADAR_RX_PIN 32
    #define RADAR_TX_PIN 33
  #endif
#elif defined(__AVR_ATmega32U4__)
  #define MONITOR_SERIAL Serial
  #define RADAR_SERIAL Serial1
  #define RADAR_RX_PIN 0
  #define RADAR_TX_PIN 1
#endif

ld2410 radar;

uint32_t lastReading = 0;
bool radarConnected = false;

int buzzer_pin = D1; // GPIO3 

// Module connection pins (Digital Pins)
#define CLK 8  // D8 GPIO8 
#define DIO 9  // D9 GPIO9

TM1637Display display(CLK, DIO);

BLECharacteristic* pCharacteristic = NULL; 
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};


void setup(void)
{
  MONITOR_SERIAL.begin(115200); //Feedback over Serial Monitor
  //radar.debug(MONITOR_SERIAL); //Uncomment to show debug information from the library on the Serial Monitor. By default this does not show sensor reads as they are very frequent.
  #if defined(ESP32)
    RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN); //UART for monitoring the radar
  #elif defined(__AVR_ATmega32U4__)
    RADAR_SERIAL.begin(256000); //UART for monitoring the radar
  #endif
  delay(500);
  MONITOR_SERIAL.print(F("\nConnect LD2410 radar TX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_RX_PIN);
  MONITOR_SERIAL.print(F("Connect LD2410 radar RX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_TX_PIN);
  MONITOR_SERIAL.print(F("LD2410 radar sensor initialising: "));
  if(radar.begin(RADAR_SERIAL))
  {
    MONITOR_SERIAL.println(F("OK"));
    MONITOR_SERIAL.print(F("LD2410 firmware version: "));
    MONITOR_SERIAL.print(radar.firmware_major_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.print(radar.firmware_minor_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.println(radar.firmware_bugfix_version, HEX);
  }
  else
  {
    MONITOR_SERIAL.println(F("not connected"));
  }

    uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };  
  display.setBrightness(0x07); 
  // All segments on
  display.setSegments(data);
  delay(1000); 
  display.clear(); 

  pinMode(buzzer_pin, OUTPUT); 
  noTone(buzzer_pin); 

  // BLE 
  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop()
{
  radar.read();
  if(radar.isConnected() && millis() - lastReading > 500)  //Report every 500ms
  {
    lastReading = millis();
    if(radar.presenceDetected())
    {
      if(radar.stationaryTargetDetected())
      {
        Serial.print(F("Stationary target: "));
        Serial.print(radar.stationaryTargetDistance());
        Serial.print(F("cm energy:"));
        Serial.print(radar.stationaryTargetEnergy());
        Serial.print(' ');
      }
      if(radar.movingTargetDetected())
      {
        int distance = radar.movingTargetDistance(); 
        int energy = radar.movingTargetEnergy(); 

        Serial.print(F("Moving target: "));
        Serial.print(distance);
        Serial.print(F("cm energy:"));
        Serial.print(energy);

        display.showNumberDec(distance, true);  
        if (deviceConnected) {
          String s = String(distance); 
          pCharacteristic->setValue(s.c_str()); 
          pCharacteristic->notify(); 
        }

        if (distance > 400) {
            tone(buzzer_pin, 500, 100); 
        } else if (distance > 300) { 
            tone(buzzer_pin, 800, 100); 
        } else if (distance > 200) {
            tone(buzzer_pin, 1100, 100); 
        } else if (distance > 100) {
            tone(buzzer_pin, 1400, 100); 
        } else {
            tone(buzzer_pin, 1700, 100); 
        }
      }

      Serial.println();
    }
    else
    {
      Serial.println(F("No target"));
    }
  }
}
