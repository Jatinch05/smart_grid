#include <esp_now.h>
#include <WiFi.h>

// Sender's MAC address (replace with actual sender MAC address)
uint8_t senderAddress[] = {0xC0, 0x49, 0xEF, 0x69, 0xD8, 0x40}; //C0:49:EF:69:D8:40

// Define the same 16-byte encryption key as the sender
uint8_t encryptionKey[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22,
                             0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00};   

// Callback function to handle received data
void onDataReceive(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  int receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.print("Received data: ");
  Serial.println(receivedData);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register sender as a peer with encryption enabled
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, senderAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = true; // Enable encryption

  // Set encryption key
  memcpy(peerInfo.lmk, encryptionKey, 16);

  // Add the peer with encryption settings
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Register the receive callback function
  esp_now_register_recv_cb(onDataReceive);
}

void loop() {
  esp_now_register_recv_cb(onDataReceive);
}
