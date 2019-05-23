# import library package
import paho.mqtt.client as mqtt
from time import sleep
import pandas as pd

# Creating a new client
client = mqtt.Client("Test MQTT")
client.username_pw_set(username = "xmxzgirv", password = "VIlzm7f9S8So")
client.connect(host = "m16.cloudmqtt.com", port = 14311)

# Create simple dataframe
df = pd.DataFrame(columns = ['Topic', 'Message']) 

# Test publish
client.publish("house/main-light","ON")

hoqr = 1

# Define callback for subscription
def process(client, userdata, message):
    global hoqr
    global df
    print("[{}]".format(str(hoqr)))
    df = df.append(
        {
            'Topic': message.topic, 
            'Message': str(message.payload.decode("utf-8"))
        }, 
        ignore_index=True)
    hoqr = hoqr + 1
    print("Topic:", message.topic)
    print("Message:", str(message.payload.decode("utf-8")))


# Subscribe to a topic
client.subscribe("switch")
client.message_callback_add("switch", process)

# Start loop
client.loop_start()
while True:
    sleep(1)

    if hoqr > 3:
        client.loop_stop()
        print(df.head())
        df.to_excel("output.xlsx")
        exit()