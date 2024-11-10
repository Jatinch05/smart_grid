#include <Crypto.h>
#include <SHA256.h>

// The data to be hashed
const char* data = "example dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample dataexample data";
unsigned long extra_mask = 0;

// Difficulty in bits (number of leading zeros in the hash)
int difficulty_bits = 40; // Adjust this as needed for higher or lower difficulty
int target_zeros = difficulty_bits / 4; // Number of full zero hex digits
int extra_bits = difficulty_bits % 4;   // Extra bits for partial hex digit

// SHA256 object from Crypto library
SHA256 hash;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  delay(1000);  // Wait for serial monitor to open

  // Print the starting message
  Serial.println("Starting Proof of Work...");

  // Track the start time
  unsigned long start_time = millis();

  // Initialize nonce
  unsigned long nonce = 0;
  
  // Mask for checking extra bits (for partial hex digits)
  if (extra_bits > 0) {
    extra_mask = (1 << (4 - extra_bits)) - 1;  // Create the mask
  }

  // Perform Proof of Work by incrementing nonce
  while (true) {
    // Create the string with data + nonce
    String dataWithNonce = String(data) + String(nonce);
    
    // Compute the SHA256 hash
    hash.reset();
    hash.update((const byte*)dataWithNonce.c_str(), dataWithNonce.length());
    byte hashResult[32];
    hash.finalize(hashResult, sizeof(hashResult));
    
    // Convert the hash to hex
    String hashHex = bytesToHex(hashResult, 32);
    
    // Check if the hash has the required number of leading zeros
    if (checkLeadingZeros(hashHex)) {
      // Calculate the time taken
      unsigned long end_time = millis();
      unsigned long duration = end_time - start_time;

      // Print the valid nonce, hash, and time taken
      Serial.print("Nonce: ");
      Serial.println(nonce);
      Serial.print("Hash: ");
      Serial.println(hashHex);
      Serial.print("Time taken (ms): ");
      Serial.println(duration);
      break;
    }

    // Increment the nonce for the next attempt
    nonce++;
  }
}

void loop() {
  // Nothing to do here
}

// Function to convert a byte array to a hex string
String bytesToHex(byte* byteArray, size_t len) {
  String hexString = "";
  for (size_t i = 0; i < len; i++) {
    char buf[3];
    sprintf(buf, "%02x", byteArray[i]);
    hexString += buf;
  }
  return hexString;
}

// Function to check if the hash has the required number of leading zeros
bool checkLeadingZeros(String hash) {
  // Check for full zero hex digits
  for (int i = 0; i < target_zeros; i++) {
    if (hash[i] != '0') {
      return false;
    }
  }

  // Check if extra bits are satisfied (if difficulty is not a multiple of 4)
  if (extra_bits > 0) {
    int nextHexDigit = hexCharToInt(hash[target_zeros]);
    if ((nextHexDigit & extra_mask) != 0) {
      return false;
    }
  }

  return true;
}

// Function to convert a hex char to an integer
int hexCharToInt(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;  // Invalid character
}