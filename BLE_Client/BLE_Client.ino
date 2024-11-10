#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEClient* pClient;
BLERemoteCharacteristic* pRemoteCharacteristic;

String trustedServer = "c0:49:ef:69:9b:7a";
float accumulatedPower = 0.0;
unsigned long lastCalculationTime = 0;
const unsigned long calculationInterval = 1000; // 1-second interval
const unsigned long sendInterval = 5000; // 5 seconds in milliseconds

TaskHandle_t PowerCalculationTaskHandle = NULL;
TaskHandle_t DataTransmissionTaskHandle = NULL;

// Function prototypes
float readCurrent();
float readVoltage();
bool connectToServer();
void PowerCalculationTask(void *pvParameters);
void DataTransmissionTask(void *pvParameters);

// Notification callback function
void onNotificationReceived(BLERemoteCharacteristic* pCharacteristic, uint8_t* data, size_t length, bool isNotify) {
  String ack = String((char*)data);
  Serial.print("Notification received: ");
  Serial.println(ack);

  if (ack[0] == 'A' && ack[1] == 'C' && ack[2] == 'K' ) {
    Serial.println("Acknowledgment received. Disconnecting...");
    pClient->disconnect();
  }
}

bool connectToServer() {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  Serial.println("Starting BLE scan...");

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

      serverAddress.toLowerCase();
      trustedServer.toLowerCase();

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

          // Register for notifications
          pRemoteCharacteristic->registerForNotify(onNotificationReceived);

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

void PowerCalculationTask(void *pvParameters) {
  while (true) {
    // Calculate power every calculationInterval
    if (millis() - lastCalculationTime >= calculationInterval) {
      float current = readCurrent();
      float voltage = readVoltage();
      float power = current * voltage; // Instantaneous power in watts

      // Convert to watt-seconds, then accumulate in watt-hours
      accumulatedPower += power * (calculationInterval / 3600000.0);
      lastCalculationTime = millis();

      Serial.print("Accumulated Power: ");
      Serial.println(accumulatedPower);
    }
    vTaskDelay(calculationInterval / portTICK_PERIOD_MS); // Delay for calculation interval
  }
}

void DataTransmissionTask(void *pvParameters) {
  unsigned long lastSendTime = millis(); // Initialize last send time

  while (true) {
    // Check if it's time to send data
    if (millis() - lastSendTime >= sendInterval) {
      if (pClient->isConnected() && pRemoteCharacteristic != nullptr) {
        String clientMAC = BLEDevice::getAddress().toString();
        String data = String(accumulatedPower);

        Serial.print("Sending accumulated power data: ");
        Serial.println(data);

        // Send accumulated power data to the server
        pRemoteCharacteristic->writeValue(data.c_str(), data.length());

        // Reset accumulated power and update send time
        accumulatedPower = 0.0;
        lastSendTime = millis();
      } else {
        Serial.println("Attempting to reconnect...");
        if (connectToServer()) {
          Serial.println("Reconnected successfully!");
        }
      }
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Check every 5 seconds
  }
}

float readCurrent() {
  return 1.5; // Example current value in amperes
}

float readVoltage() {
  return 220; // Example voltage value in volts
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("ESP32_Client");

  pClient = BLEDevice::createClient();

  // Create FreeRTOS tasks for power calculation and data transmission
  xTaskCreatePinnedToCore(PowerCalculationTask, "Power Calculation Task", 2048, NULL, 1, &PowerCalculationTaskHandle, 1);
  xTaskCreatePinnedToCore(DataTransmissionTask, "Data Transmission Task", 4096, NULL, 1, &DataTransmissionTaskHandle, 1);
}

void loop() {
  // Not used since we're using FreeRTOS tasks
}
