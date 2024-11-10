#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#include <RTClib.h> // Include library for RTC (Real-Time Clock)

RTC_DS3231 rtc;

BLEClient* pClient;
BLERemoteCharacteristic* pRemoteCharacteristic;

// Server MAC address
float accumulatedPower = 0.0;
unsigned long lastCalculationTime = 0;
const unsigned long calculationInterval = 1000; // 1 second interval

// Function prototypes
float readCurrent();
float readVoltage();
bool connectToServer();


String trustedServer = "c0:49:ef:69:9b:7a";

bool connectToServer() {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  Serial.println("Starting BLE scan...");

  // Use pointer instead of an object
  BLEScanResults* scanResults = pBLEScan->start(10);
  Serial.printf("Found %d devices\n", scanResults->getCount());

  for (int i = 0; i < scanResults->getCount(); i++) {
    BLEAdvertisedDevice advertisedDevice = scanResults->getDevice(i);

    Serial.print("Checking device: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(BLEUUID(SERVICE_UUID))) {
      String serverAddress = advertisedDevice.getAddress().toString();
      Serial.print("Server MAC Address: ");
      Serial.println(serverAddress.c_str());

      // Convert both MAC addresses to lowercase for comparison
      serverAddress.toLowerCase();
      trustedServer.toLowerCase();

      // Check if the MAC address is in the trusted list
      if (serverAddress == trustedServer) {
        Serial.println("Server is trusted. Attempting to connect...");
        if (pClient->connect(&advertisedDevice)) {
          Serial.println("Successfully connected to BLE server");

          BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
          if (pRemoteService == nullptr) {
            Serial.println("Failed to find service. Disconnecting...");
            pClient->disconnect();
            return false;
          }

          pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
          if (pRemoteCharacteristic == nullptr) {
            Serial.println("Failed to find characteristic. Disconnecting...");
            pClient->disconnect();
            return false;
          }

          Serial.println("Characteristic found. Ready to send data!");
          return true;
        } else {
          Serial.println("Failed to connect to BLE server");
        }
      } else {
        Serial.println("Server is not trusted. Skipping...");
      }
    }
  }

  Serial.println("No trusted server found. Retrying...");
  return false;
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("ESP32_Client");

  pClient = BLEDevice::createClient();
  if (!connectToServer()) {
    Serial.println("Connection failed. Retrying...");
  }

   if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
}

void loop() {
  if (pClient->isConnected() && pRemoteCharacteristic != nullptr) {
    String data = "Client Data";
    Serial.print("Sending data: ");
    Serial.println(data);
    pRemoteCharacteristic->writeValue(data.c_str(), data.length());
    delay(2000);
  } else {
    Serial.println("Reconnecting...");
    if (connectToServer()) {
      Serial.println("Reconnected successfully!");
    }
    delay(5000);
  }
  DateTime now = rtc.now();

  // Check if it's 11:59 PM
  if (now.hour() == 23 && now.minute() == 59) {
    // Check if connected to BLE server and ready to send data
    if (pClient->isConnected() && pRemoteCharacteristic != nullptr) {
      // Send the accumulated power value
      String data = String(accumulatedPower);
      Serial.print("Sending power data at 11:59 PM: ");
      Serial.println(data);
      pRemoteCharacteristic->writeValue(data.c_str(), data.length());

      // Reset accumulated power for the next day
      accumulatedPower = 0.0;
    }

    // Wait until the next minute (to avoid multiple sends within the same minute)
    delay(60000); // Wait for a minute
  }
  
}

float readCurrent() {
  return 1.5; // Example current value in amperes
}

// Mock function to read voltage from sensor (replace with actual sensor code)
float readVoltage() {
  return 220; // Example voltage value in volts
}



