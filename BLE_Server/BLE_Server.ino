#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
// #include <BLE2902.h>
#include <BLESecurity.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// BLE Variables
BLECharacteristic *pCharacteristic;
String currentClientMAC;
int dataReceivedCount = 0;
int meterCount = 10;  // Number of data pieces expected from each client

// Array to store the MAC addresses of clients waiting for data
String clientsWaitingForData[10];  // Max 10 clients (adjust size as needed)
int clientsWaitingCount = 0;  // Counter for the number of clients in the array

// Trusted clients' MAC addresses (example)
const char* trustedClients[] = {
  "C0:49:EF:69:D8:42",  // Client 1 MAC Address
  "C0:49:EF:69:C8:FE"   // Client 2 MAC Address
};

// WiFi and MQTT credentials
const char* ssid = "iQOO 9 SE";
const char* password = "12345678";
const char* mqtt_server = "e75df26b78d24d67aa8fcc75770584f1.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;  // SSL port
const char* mqtt_topic = "esp32/test";
const char* mqtt_user = "Siddu";
const char* mqtt_password = "Siddu@123";

// Root CA certificate for SSL
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
"-----END CERTIFICATE-----\n";

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// Function to connect to WiFi
void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// Function to connect to MQTT broker
void setup_mqtt() {
  espClient.setCACert(root_ca);
  mqttClient.setServer(mqtt_server, mqtt_port);
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32SecureClient", mqtt_user, mqtt_password)) {
      mqttClient.publish(mqtt_topic, "Hello from ESP32 with SSL and credentials!");
    } else {
      delay(5000); // Retry connection
    }
  }
}

// Check if the client is trusted
bool isClientTrusted(const String& clientMAC) {
  for (int i = 0; i < sizeof(trustedClients) / sizeof(trustedClients[0]); i++) {
    if (clientMAC == String(trustedClients[i])) {
      return true;
    }
  }
  return false;
}

// Function to forward data to MQTT once all clients have sent their data
void forwardAllClientsData() {
  if (clientsWaitingCount == 0) {
    String dataToSend = "All clients' data received.";  // You can customize this message
    mqttClient.publish(mqtt_topic, dataToSend.c_str());
  }
}

// BLE Server Callbacks
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    BLEDevice::getAddress().toString().c_str();
    currentClientMAC = BLEDevice::getAddress().toString().c_str();
    if (isClientTrusted(currentClientMAC)) {
      if (clientsWaitingCount < meterCount) {
        clientsWaitingForData[clientsWaitingCount++] = currentClientMAC;
      }
    }
  }

  void onDisconnect(BLEServer* pServer) {
    if (isClientTrusted(currentClientMAC)) {
      for (int i = 0; i < clientsWaitingCount; i++) {
        if (clientsWaitingForData[i] == currentClientMAC) {
          clientsWaitingForData[i] = "";  // Clear the MAC address
          clientsWaitingCount--;
          break;
        }
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  setup_wifi();
  setup_mqtt();
  
  BLEDevice::init("ESP32 BLE Server");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pService->start();
  
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  mqttClient.loop();
}
