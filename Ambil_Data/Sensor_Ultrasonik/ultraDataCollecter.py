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
collectionTopic = "water/debug"

# Receive input for file title
dataTitle = input("Enter data title: ")
fileTitle = dataTitle + ".xlsx"

# Create new dataframe
df = pd.DataFrame(columns = ['Percentage', 'Distance (cm)', 'Time (microseconds)'])

# Create MQTT client
client = mqtt.Client(clientID)
client.username_pw_set(username = mqttUsername, password = mqttPassword)
client.connect(host = hostName, port = portNum)

# Initial publish
client.publish("data/collector", "Initiated")
print("Collecting data for", fileTitle, "...")
print("")

dataIndex = 1

# Define callback for subscription
def collector(client, userdata, message):

    # Grab global variables
    global dataIndex
    global df

    # Data index
    print("[{}]".format(str(dataIndex)))
    data = str(message.payload.decode("utf-8"))
    data = data.split("|")
    print("Percentage:", data[0], "%")
    print("Distance:", data[1], "cm")
    print("Time:", data[2], "us")
    print("")

    # Insert into dataframe
    df = df.append(
        {
            'Percentage': data[0],
            'Distance (cm)': data[1],
            'Time (microseconds)': data[2]
        },
        ignore_index = True
    )

    # increase index
    dataIndex = dataIndex + 1

# Subscribe collector to Topic
client.subscribe(collectionTopic)
client.message_callback_add(collectionTopic, collector)

# Start loop
client.loop_start()
while True:

    if dataIndex > 100:
        client.loop_stop()
        print(df.head(20))
        print("Generating", fileTitle, "...")
        df.to_excel(fileTitle)
        exit()