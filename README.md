# Smart Grid Communication with Light PoW

This project implements a secure and efficient communication system for smart grids using ESP32 microcontrollers, employing Light Proof-of-Work (PoW) for data verification and security. The aim is to develop a scalable and collision-free communication protocol using ESP-NOW, along with MQTT integration for central data handling.

## Overview

Our smart grid communication system addresses the challenge of secure and efficient power consumption monitoring. The system utilizes ESP32 microcontrollers to continuously gather and transmit power readings from smart meters to a central PoW-processing ESP32. The processed data is then securely published to a central MQTT broker.

### Main Files

- **ESP_NOW_Sender.ino**: This file should be run on the ESP32 connected to the smart meter. It handles the collection of power readings and sending the data via ESP-NOW.
- **espnowrelay.ino**: This file should be run on the ESP32 that collects the data from multiple meters, performs the PoW computation, and sends the verified data to the MQTT broker.
- **server.py**: Should be run on a PC or server with internet access. This acts as the grid server which receives the data from PoW ESP32 and verifies the PoW.
- **Note**: The files related to BLE were not used because BLE was too heavy for the ESP32. The remaining files are just test files for development and debugging purposes.

## Features

- **ESP-NOW Communication**: Utilizes ESP-NOW for low-latency, collision-free, and encrypted data transfer between ESP32 devices.
- **Light Proof-of-Work (PoW)**: Implements a lightweight PoW mechanism on the ESP32 to ensure data integrity and prevent tampering.
- **Multiprocessing**: The PoW ESP32 uses multiprocessing to efficiently handle incoming data from smart meters while simultaneously performing PoW calculations and sending verified data to the MQTT broker. This ensures minimal data loss and optimizes resource utilization.
- **MQTT Integration**: Data is published to an MQTT broker using secure SSL/TLS connections.
- **Security Measures**:
  - **AES Encryption**: Encrypted data transmission via ESP-NOW for secure communication.
  - **MAC Address Whitelisting**: Ensures only authorized devices can communicate with the server.
  - **SHA-256 Hashing**: Used in the PoW mechanism to provide data verification.
  - **SSL/TLS for MQTT**: Secure communication with the MQTT broker using a self-signed certificate.
- **ACK-Based Collision Handling:**: Implements an acknowledgment mechanism to handle data collisions effectively. The PoW ESP32 sends an ACK to confirm successful data reception from the meters. If an ACK is not received, the sender retransmits the data, ensuring reliable and collision-free communication. This approach minimizes data loss and maintains the integrity of the data transfer.

## Project Structure

- **ESP32 Meters**: Collect and send power readings to the central ESP32 using ESP-NOW.
- **PoW ESP32**: Receives data, performs PoW, and sends the verified data to the MQTT broker.
- **HiveMQ Cloud**: Acts as the MQTT Broker
- **Grid Server**: Collects the verified data from the PoW ESP32 and serves as the central data handler.

## Setup Instructions

### Hardware Requirements

- 4 ESP32 microcontrollers: 3 for smart meters and 1 for PoW processing.
- Sensors: ACS712 (current sensor) and ZMPT101B (voltage sensor) for power measurement.
- Wi-Fi-enabled PC or server for hosting the Grid Server.

### Software Requirements

- **Arduino IDE** with ESP32 board support.
- **Paho MQTT Client** (for testing the MQTT setup).
- **Wireshark** or similar tool for packet capture and verification.

## Installation Guide

1. **Configure ESP32 Meters**:
   - Install libraries: `esp_now.h`, `WiFi.h`, and `SHA256.h`.
   - Connect the sensors to the ESP32 and calibrate as necessary.
   - Flash the ESP32 with the `ESP_NOW_Sender.ino` file provided in the repository.

2. **Setup PoW ESP32**:
   - Install required libraries: `esp_now.h`, `WiFiClientSecure.h`, `PubSubClient.h`.
   - Configure the PoW mechanism and MQTT publishing.
   - Flash the ESP32 with the `espnowrelay.ino` file.

3. **Configure the Grid Server**:
   - Set up an MQTT broker using HiveMQ or a similar service.
   - Ensure SSL/TLS is configured for secure data transfer.

## How It Works

1. **Data Collection**: Smart meters continuously read power values and send them to the PoW ESP32 using ESP-NOW.
2. **Proof-of-Work**: The PoW ESP32 aggregates the readings, performs PoW to ensure data integrity, and sends the data to the MQTT broker.
3. **Multiprocessing**: The PoW ESP32 uses separate tasks to handle incoming data and PoW computation, allowing simultaneous data reception and processing.
4. **MQTT Transmission**: The processed data is securely transmitted to the MQTT broker and stored for further analysis.

## Security Details

- **ESP-NOW Encryption**: Uses built-in AES encryption for secure peer-to-peer communication.This is done to ensure that the smart meters are run in an isolated enviromnent with no internet access to reduce external threats.
- **MAC Whitelisting**: Only pre-approved MAC addresses can connect and send data.
- **SHA-256 Hashing**: Protects against data manipulation during PoW.
- **SSL/TLS**: Encrypts the connection between the PoW ESP32 and the MQTT broker.

## Future Improvements

- **Scalability**: Explore adding more smart meters and optimizing the PoW mechanism.
- **Blockchain Integration**: Implement blockchain near the cluster head for a decentralized, tamper-proof ledger to further secure and verify data.
- **Improving meter accuracy**: Enhance real-time accuracy of smart meters by incorporating advanced sensors for more precise power usage tracking.

## Contributors

- [Pankam Sasank]
- [Chigurubattula Jatin]
- [Kunta Sai Siddartha]
  

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

Special thanks to our mentors and the open-source community for their support and contributions.
