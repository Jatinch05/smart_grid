#include <esp_now.h>
#include <WiFi.h>

// Receiver's MAC address (replace with the actual MAC address of the receiver ESP32)
uint8_t receiverAddress[] = {0xC0, 0x49, 0xEF, 0x69, 0xC8, 0xFC};

// Define a 16-byte encryption key (same key must be used on the receiver)
uint8_t encryptionKey[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22,
                             0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00};

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer (receiver's MAC address)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = true; // Enable encryption

  // Set encryption key
  memcpy(peerInfo.lmk, encryptionKey, 16);

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // Generate random data and send it to the receiver
  int randomData = random(0, 100);  // Random data between 0 and 99
  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *) &randomData, sizeof(randomData));

  if (result == ESP_OK) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Error sending data");
  }

  delay(1000); // Send data every second
}