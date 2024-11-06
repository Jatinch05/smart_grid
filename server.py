import paho.mqtt.client as mqtt

# MQTT Broker settings
broker = "e75df26b78d24d67aa8fcc75770584f1.s1.eu.hivemq.cloud"
port = 8883  # SSL port
topic = "esp32/test"
username = "Jatin"
password = "Jatinch@2005"

# Path to the CA certificate file
ca_cert_path = "broker_cert.pem"
# Define what happens on connection
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(topic)

# Define what happens when a message is received
def on_message(client, userdata, msg):
    print("Message received from topic: " + msg.topic)
    print("Message content: " + str(msg.payload.decode()))

# Set up client and callbacks
client = mqtt.Client()
client.username_pw_set(username, password)
client.on_connect = on_connect
client.on_message = on_message

# Enable TLS and set the CA certificate for broker verification
client.tls_set(ca_certs=ca_cert_path)  # Ensure the path to your root CA certificate file is correct

# Connect and run the client loop
client.connect(broker, port)
client.loop_forever()
