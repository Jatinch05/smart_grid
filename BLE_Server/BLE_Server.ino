#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLESecurity.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
// std::map<std::string, int> connectedClients;

short int difficulty = 20;

class Storage {
  std::vector<String> data;
  uint32_t nonce = 0;
  uint8_t* hashResult;
  

// Proof of Work function
  void proofOfWork() {
    // uint32_t nonce = 0;
    String concatenatedData = "";

    // Concatenate all strings in the vector
    for (const auto& transaction : data) {
      concatenatedData += transaction;
    }

    while (true) {
      noncedData = concatenatedData + String(nonce);
      sha256.reset();
      sha256.update((const uint8_t*)noncedData.c_str(), noncedData.length());

      // Compute the hash
      hashResult = sha256.result();

      // Check if the hash meets the difficulty level (leading zero bits)
      if (meetsDifficulty(hashResult, difficulty)) {
        return;
      }

      // Increment the nonce and update the data
      nonce++;
    }
  }

  // Helper function to check if the hash meets the difficulty requirement
  bool meetsDifficulty(uint8_t* hash, int difficulty) {
    // Calculate how many complete bytes should be zero
    int completeZeros = difficulty / 8;
    // Calculate how many leading zero bits are needed in the next byte
    int leadingZerosInNextByte = difficulty % 8;

    // Check complete zero bytes
    for (int i = 0; i < completeZeros; i++) {
      if (hash[i] != 0x00) {
        return false; // If any complete zero byte is not zero, return false
      }
    }

    // Check the partial zero byte if necessary
    if (leadingZerosInNextByte > 0) {
      // Create a mask for the required number of leading zeros
      uint8_t mask = 0xFF >> (leadingZerosInNextByte);

      if(hash[completeZeros + 1] > mask) return false;
    }

    return true; // All checks passed
  }
}

Storage currentBlock;

const char* trustedClients[] = {
  "C0:49:EF:69:D8:42",
  "C0:49:EF:69:C8:FE"
};

// Variable to store the client's MAC address
String currentClientMAC;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param) {
    char clientAddress[18];
    sprintf(clientAddress, "%02X:%02X:%02X:%02X:%02X:%02X",
            param->connect.remote_bda[0], param->connect.remote_bda[1],
            param->connect.remote_bda[2], param->connect.remote_bda[3],
            param->connect.remote_bda[4], param->connect.remote_bda[5]);

    // Store the MAC address of the connected client
    currentClientMAC = String(clientAddress);

    Serial.printf("Client connected: %s\n", clientAddress);
    // connectedClients[std::string(clientAddress)]++;
    BLEDevice::startAdvertising();
  }

  void onDisconnect(BLEServer* pServer) {
    Serial.println("Client disconnected. Restarting advertising...");
    delay(100);
    BLEDevice::startAdvertising();
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = String(pCharacteristic->getValue());
      
    // Print the stored MAC address and received data
    Serial.print("Received data from ");
    Serial.print(currentClientMAC.c_str());
    Serial.print(": ");
    Serial.println(value.c_str());
  }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("ESP32_Server");

  // Initialize BLE Security
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
  pSecurity->setCapability(ESP_IO_CAP_IO);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  Serial.println("BLE Server with security is now advertising...");
}

void loop() {
  delay(1000);
}
