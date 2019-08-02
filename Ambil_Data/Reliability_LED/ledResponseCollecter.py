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
collectionTopic1 = "led/color"
collectionTopic2 = "led/colorstatus"

# Receive input for file title
dataTitle = input("Enter data title: ")
fileTitle = dataTitle + ".xlsx"

# Create new dataframe
df = pd.DataFrame(columns = ['Red (User)', 'Green (User)', 'Blue (User)', 'Red (Device)', 'Green (Device)', 'Blue (Device)'])

# Create MQTT client
client = mqtt.Client(clientID)
client.username_pw_set(username = mqttUsername, password = mqttPassword)
client.connect(host = hostName, port = portNum)

# Initial publish
client.publish("data/collector", "Initiated")
print("Collecting data for", fileTitle, "...")
print("")

dataIndex = 1

dataLED = {}

# Define callback for subscription
def collectorUser(client, userdata, message):

    # Grab global variables
    global dataIndex
    global dataLED

    # Data index
    print("[{}]".format(str(dataIndex)))
    data = str(message.payload.decode("utf-8"))
    data = data.split("|")
    #print(str(message.size))
    print("User")
    print("Red:", data[0])
    print("Green:", data[1])
    print("Blue:", data[2])
    print("---------------------")

    dataLED = {
        'Red': data[0],
        'Green': data[1],
        'Blue': data[2]
    }
    

# Define callback for inserting
def collectorDevice(client, userdata, message):

    # Grab global variables
    global dataIndex
    global df
    global dataLED

    # Insert into dataframe
    data = str(message.payload.decode("utf-8"))
    data = data.split("|")
    print("Device")
    print("Red:", data[0])
    print("Green:", data[1])
    print("Blue:", data[2])
    print("")
    df = df.append(
        {
            'Red (User)': dataLED['Red'], 
            'Green (User)': dataLED['Green'], 
            'Blue (User)': dataLED['Blue'],
            'Red (Device)': data[0],
            'Green (Device)': data[1],
            'Blue (Device)': data[2]
        },
        ignore_index = True
    )

    dataLED = {}

    # increase index
    dataIndex = dataIndex + 1

# Subscribe collector to Topic
client.subscribe(collectionTopic1)
client.subscribe(collectionTopic2)
client.message_callback_add(collectionTopic1, collectorUser)
client.message_callback_add(collectionTopic2, collectorDevice)

# Start loop
client.loop_start()
while True:

    if dataIndex > 5:
        client.loop_stop()
        print(df.head(20))
        print("Generating", fileTitle, "...")
        df.to_excel(fileTitle)
        exit()