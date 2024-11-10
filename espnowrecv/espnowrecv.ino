#include <esp_now.h>
#include <WiFi.h>
#include <AESLib.h>

// Structure to hold incoming data
typedef struct struct_message {
  int clientId;
  float reading;
  char encryptedData[32]; // Encrypted payload
} struct_message;

struct_message incomingData;

// Whitelist of authorized MAC addresses
uint8_t whitelist[3][6] = {
  {0xC0, 0x49, 0xEF, 0x69, 0x9B, 0x7A}, // MAC address of Meter 1
  {0xC0, 0x49, 0xEF, 0x69, 0x9B, 0x7B}, // MAC address of Meter 2
  {0xC0, 0x49, 0xEF, 0x69, 0x9B, 0x7C}  // MAC address of Meter 3
};
// AES encryption key and IV
uint8_t aesKey[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
byte aesIv[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Example IV

AESLib aesLib;

// Check if the device is in the whitelist
bool isAuthorizedDevice(const uint8_t *mac) {
  for (int i = 0; i < 3; i++) {
    if (memcmp(mac, whitelist[i], 6) == 0) {
      return true;
    }
  }
  return false;
}

// Decrypt the received data
void decryptData(const char* encryptedData, char* decryptedData) {
  uint16_t encryptedLength = strlen(encryptedData);
  aesLib.decrypt(
    (byte*)encryptedData, encryptedLength, (byte*)decryptedData, aesKey, 128, aesIv
  );
}

// Callback function for receiving data
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (!isAuthorizedDevice(info->src_addr)) {
    Serial.println("Unauthorized device attempted to send data!");
    return;
  }

  memcpy(&incomingData, data, sizeof(incomingData));
  Serial.printf("Data received from client %d\n", incomingData.clientId);

  // Decrypt the data
  char decryptedData[32];
  decryptData(incomingData.encryptedData, decryptedData);
  Serial.printf("Decrypted Data: %s\n", decryptedData);

  // Send acknowledgment
  const char* ackMessage = "ACK";
  esp_now_send(info->src_addr, (uint8_t *)ackMessage, strlen(ackMessage));
  Serial.println("Acknowledgment sent");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Set device in STA mode

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the receive callback
  esp_now_register_recv_cb(onDataRecv);
  Serial.println("ESP-NOW receiver initialized");
}

void loop() {
  // The receiver loop can remain empty
}