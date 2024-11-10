#include <esp_now.h>
#include <WiFi.h>
#include <SHA256.h>

#define DIFFUCULTY 20

SHA256 sha256;

// Structure to send data
typedef struct struct_message {
  int clientId;
  float powerReading; // Example power reading
  char hash[32]; // Hash value for data verification
} struct_message;

struct_message myData;

// MAC address of the receiver (PoW ESP32)
uint8_t receiverAddress[] = {0xC0, 0x49, 0xEF, 0x69, 0x9B, 0x78}; // Update with the actual MAC address

// AES encryption key (must match the receiver's key)
uint8_t encryptionKey[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22,
                             0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00};

// Flag to track acknowledgment
bool ackReceived = false;

// Callback when data is sent
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Data send status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Success");
  } else {
    Serial.println("Failure");
  }
}

// Corrected Callback for acknowledgment
void onAckReceived(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  Serial.println("onAckReceived() called");
  Serial.printf("Received data length: %d bytes\n", len);

  if (strncmp((char *)incomingData, "ACK", len) == 0) {
    ackReceived = true;
    Serial.println("Acknowledgment received");
  } else {
    Serial.println("Received data is not an acknowledgment");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing ESP-NOW Sender...");

  WiFi.mode(WIFI_STA);
  Serial.println("WiFi mode set to STA");

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } else {
    Serial.println("ESP-NOW initialized successfully");
  }

  // Register the send and acknowledgment callbacks
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onAckReceived);

  // Register peer (receiver)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = true; // Enable encryption
  memcpy(peerInfo.lmk, encryptionKey, 16);

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  } else {
    Serial.println("Peer added successfully");
  }

  // Initialize data to send
  myData.clientId = 1; // Example client ID
  myData.powerReading = random(0, 100) / 10.0; // Simulated power reading
  Serial.printf("Initialized data: clientId=%d, powerReading=%.2f\n", myData.clientId, myData.powerReading);

  // Generate hash for data verification
  String dataToHash = String(myData.clientId) + String(myData.powerReading);
  uint8_t hashResult[32];
  sha256.reset();
  sha256.update((const uint8_t*)dataToHash.c_str(), dataToHash.length());
  sha256.finalize(hashResult, sizeof(hashResult));
  memcpy(myData.hash, hashResult, sizeof(hashResult));

  Serial.print("Generated hash: ");
  for (int i = 0; i < 32; i++) {
    Serial.printf("%02x", myData.hash[i]);
  }
  Serial.println();
}

void loop() {
  ackReceived = false; // Reset the acknowledgment flag
  Serial.println("Sending data to receiver...");

  // Send data to receiver
  esp_now_send(receiverAddress, (uint8_t *)&myData, sizeof(myData));

  // Wait for acknowledgment with a timeout
  unsigned long startTime = millis();
  while (!ackReceived && millis() - startTime < 5000) { // 5-second timeout
    // Waiting for acknowledgment
  }

  if (ackReceived) {
    Serial.println("Acknowledgment received, stopping data transmission");
  } else {
    Serial.println("No acknowledgment received, resending data");
    // Retry logic: you can resend the data if needed
  }

  delay(2000); // Delay before checking/sending data again
}
