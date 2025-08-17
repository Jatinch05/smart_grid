#include <esp_now.h>
#include <WiFi.h>
#include <SHA256.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// WiFi and MQTT credentials
const char* ssid = "enter the ssid of your wifi";
const char* password = "enter the password of your wifi";
const char* mqtt_server = "the cloud broker url";
const int mqtt_port = 8883;  // SSL port
const char* mqtt_topic = "topic name";

// MQTT broker credentials
const char* mqtt_user = "enter your username";
const char* mqtt_password = "enter your password";

// Root CA certificate for verification
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFYDCCBEigAwIBAgIQQAF3ITfU6UK47naqPGQKtzANBgkqhkiG9w0BAQsFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTIxMDEyMDE5MTQwM1oXDTI0MDkzMDE4MTQwM1ow\n" \
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwggIiMA0GCSqGSIb3DQEB\n" \
"AQUAA4ICDwAwggIKAoICAQCt6CRz9BQ385ueK1coHIe+3LffOJCMbjzmV6B493XC\n" \
"ov71am72AE8o295ohmxEk7axY/0UEmu/H9LqMZshftEzPLpI9d1537O4/xLxIZpL\n" \
"wYqGcWlKZmZsj348cL+tKSIG8+TA5oCu4kuPt5l+lAOf00eXfJlII1PoOK5PCm+D\n" \
"LtFJV4yAdLbaL9A4jXsDcCEbdfIwPPqPrt3aY6vrFk/CjhFLfs8L6P+1dy70sntK\n" \
"4EwSJQxwjQMpoOFTJOwT2e4ZvxCzSow/iaNhUd6shweU9GNx7C7ib1uYgeGJXDR5\n" \
"bHbvO5BieebbpJovJsXQEOEO3tkQjhb7t/eo98flAgeYjzYIlefiN5YNNnWe+w5y\n" \
"sR2bvAP5SQXYgd0FtCrWQemsAXaVCg/Y39W9Eh81LygXbNKYwagJZHduRze6zqxZ\n" \
"Xmidf3LWicUGQSk+WT7dJvUkyRGnWqNMQB9GoZm1pzpRboY7nn1ypxIFeFntPlF4\n" \
"FQsDj43QLwWyPntKHEtzBRL8xurgUBN8Q5N0s8p0544fAQjQMNRbcTa0B7rBMDBc\n" \
"SLeCO5imfWCKoqMpgsy6vYMEG6KDA0Gh1gXxG8K28Kh8hjtGqEgqiNx2mna/H2ql\n" \
"PRmP6zjzZN7IKw0KKP/32+IVQtQi0Cdd4Xn+GOdwiK1O5tmLOsbdJ1Fu/7xk9TND\n" \
"TwIDAQABo4IBRjCCAUIwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYw\n" \
"SwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5pZGVudHJ1\n" \
"c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTEp7Gkeyxx\n" \
"+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEEAYLfEwEB\n" \
"ATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2VuY3J5cHQu\n" \
"b3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0LmNvbS9E\n" \
"U1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFHm0WeZ7tuXkAXOACIjIGlj26Ztu\n" \
"MA0GCSqGSIb3DQEBCwUAA4IBAQAKcwBslm7/DlLQrt2M51oGrS+o44+/yQoDFVDC\n" \
"5WxCu2+b9LRPwkSICHXM6webFGJueN7sJ7o5XPWioW5WlHAQU7G75K/QosMrAdSW\n" \
"9MUgNTP52GE24HGNtLi1qoJFlcDyqSMo59ahy2cI2qBDLKobkx/J3vWraV0T9VuG\n" \
"WCLKTVXkcGdtwlfFRjlBz4pYg1htmf5X6DYO8A4jqv2Il9DjXA6USbW1FzXSLr9O\n" \
"he8Y4IWS6wY7bCkjCWDcRQJMEhg76fsO3txE+FiYruq9RUWhiF1myv4Q6W+CyBFC\n" \
"Dfvp7OOGAN6dEOM4+qR9sdjoSYKEBpsr6GtPAQw4dy753ec5\n" \
"-----END CERTIFICATE-----\n"; // Your certificate here

WiFiClientSecure espClient;
PubSubClient client(espClient);

SHA256 sha256;
#define THRESHOLD 3
#define DIFFUCULTY 10
uint8_t senderAddress[] = {0xC0, 0x49, 0xEF, 0x69, 0xD8, 0x40};
uint8_t encryptionKey[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00};

String* accumulatedData = new String();
size_t receivedCount = 0;

void onDataReceive(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {


  // Serial.print("Process Id of ondatareceive: ");
  // Serial.println(xPortGetCoreID());

  // Serial.println("onDataReceive() called");
  // Serial.printf("Received data length: %d bytes\n", len);

  char receivedData[len + 1];
  memcpy(&receivedData, incomingData, len);
  receivedData[len] = '\0';

  // for(int i = 0; i < len; i++) {
  //   Serial.print(incomingData[i]);
  //   Serial.print("  ");
  //   Serial.print(receivedData[i]);
  //   Serial.println();
  // }

  // Serial.print("Received data: ");
  // Serial.println(receivedData);

  *(accumulatedData) = *(accumulatedData) + String("\n") + String(receivedData);
  // Serial.print("Accumulated data: ");
  // Serial.println(*accumulatedData);

  receivedCount = (receivedCount + 1) % THRESHOLD;
  // Serial.printf("Received count: %d\n", receivedCount);

  if (receivedCount == 0) {
    // Serial.println("Threshold reached, creating task to send data to broker...");
    xTaskCreatePinnedToCore(
      sendToBroker,
      "sendToBroker",
      16384,
      accumulatedData,
      1,
      NULL,
      1
    );
    accumulatedData = new String();
  }

  char* randomData = "ACK";
  esp_now_send(info->src_addr, (uint8_t *)randomData, 3);
  // Serial.println("Acknowledgment sent to sender");
}

String hashToString(const uint8_t* hash, size_t length) {
  String hexString = "";
  for (size_t i = 0; i < length; i++) {
    hexString += String(hash[i], HEX);  // Convert each byte to hex
  }
  return hexString;
}

void sendToBroker(void* pvParameters) {
  // Serial.print("Process Id of sendtobroker: ");
  // Serial.println(xPortGetCoreID());
  // Serial.println("sendToBroker() called");

  String* recv = (String*)pvParameters;
  Serial.print("Data to send: ");
  Serial.println(*recv);

  uint8_t hashResult[32];
  long int nonce = 0;
  proofOfWork(recv, nonce, hashResult);

  WiFi.begin(ssid, password);
  // Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    // Serial.print(".");
    delay(500);
  }
  // Serial.println("\nConnected to WiFi");

  espClient.setCACert(root_ca);

  // char* charecterHash[] = (char*)hashResult;
  // char charecterHash[32];

  client.setServer(mqtt_server, mqtt_port);


  // Serial.print("Connecting to MQTT broker...");
  while (!client.connect("ESP32SecureClient", mqtt_user, mqtt_password)) {
    // Serial.print(".");
    delay(500);
  }
  // Serial.println("\nConnected to MQTT broker");

  String finalMessage = *(recv) + String("?") + String(nonce) + String("?") + hashToString(hashResult, 32);
  // Serial.print("Final message: ");
  // Serial.println(finalMessage);

  client.publish(mqtt_topic, finalMessage.c_str());
  // Serial.println("Data published to MQTT topic");
  client.disconnect();

  delete recv;
  // vTaskDelete(NULL);
  return;
}



void proofOfWork(const String* data, long int& nonce, uint8_t* hashResult) {
  // Serial.println("Starting Proof of Work...");
  int completeZeros = DIFFUCULTY / 8;
  int leadingZerosInNextByte = DIFFUCULTY % 8;
  uint8_t mask = 0xFF >> (leadingZerosInNextByte);
  Serial.println("doinng  pow to: ");
  Serial.println(*data);

  while (true) {
    String noncedData = *(data) + String(nonce);
    sha256.reset();
    sha256.update((const uint8_t*)noncedData.c_str(), noncedData.length());
    sha256.finalize(hashResult, 32);

    // Debugging hash values
    // Serial.print("Nonce: ");
    // Serial.println(nonce);

    bool meetsDifficulty = true;
    for (int i = 0; i < completeZeros; i++) {
      if (hashResult[i] != 0x00) {
        meetsDifficulty = false;
        break;
      }
    }

    if (meetsDifficulty && (leadingZerosInNextByte == 0 || hashResult[completeZeros] <= mask)) {
      Serial.print("Proof of Work found: ");
      Serial.println(nonce);
      // Serial.println(*(hashResult));

      // Serial.print("Hash: ");
      // for (int i = 0; i < 32; i++) {
      //   Serial.printf("%02x", hashResult[i]);
      // }
      // Serial.println();
      
      return;
    }

    nonce++;
  }
}


void setup() {
  Serial.begin(115200);
  delay(500);
    // Serial.println("Initiating 0");
  WiFi.mode(WIFI_STA);
  // Serial.println("Initiating 1");

  if (esp_now_init() != ESP_OK) {
    // Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Serial.println("Initiating 2");

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, senderAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = true;
  memcpy(peerInfo.lmk, encryptionKey, 16);

  // Serial.println("Initiating 3");

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Serial.println("Initiating 5");
  
  esp_now_register_recv_cb(onDataReceive);
  Serial.println("ESP-NOW initialized successfully");
}

void loop() {
  delay(1000);
}
