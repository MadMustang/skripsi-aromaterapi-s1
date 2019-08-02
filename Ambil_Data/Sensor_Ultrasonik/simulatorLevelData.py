# import library package
import paho.mqtt.client as mqtt
from time import sleep
import pandas as pd

# Define MQTT Information
clientID = "SensorDataCollector"
mqttUsername = "xmxzgirv"
mqttPassword = "VIlzm7f9S8So"
hostName = "m16.cloudmqtt.com"
portNum = 14311
collectionTopic = "Sensor"

# Create MQTT client
client = mqtt.Client(clientID)
client.username_pw_set(username = mqttUsername, password = mqttPassword)
client.connect(host = hostName, port = portNum)

while True:
    client.publish(collectionTopic, "20")
    print("data sent")
    print("")
    sleep(5)