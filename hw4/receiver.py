from sklearn.naive_bayes import BernoulliNB
from sklearn.externals import joblib
import numpy
import paho.mqtt.client as mqtt
from datetime import datetime
import json
import atexit

# Global states
floor_change = False
indoor = True
idle = True

# Received data
acc_data = []
baro_data = []
light_data = []
temp_data = []
humid_data = []

# Sliding window size
ACC_WINDOW = 100
BARO_WINDOW = 100
LIGHT_WINDOW = 100

# Last state change time
start_time = datetime.now()
floor_utime = None
indoor_utime = None
light_utime = None

# initilise
fout = open("output.csv", "w")
# load light sensor data model
bnb = joblib.load('bnb_model.pkl') 

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    json_obj = json.loads(msg.payload) 
    raw = json_obj["value"].split(",")
    # print(raw)
    if raw[2] == "a":
        acc_data.append([int(raw[1]),float(raw[3]),float(raw[4]),float(raw[5])])
        process_acc_data()

    elif raw[2] == "b":
        baro_data.append([int(raw[1]),float(raw[3])])
        process_baro_data()

    elif raw[2] == "l":
        light_data.append([int(raw[1]),float(raw[3])])
        process_light_data()
    
    elif raw[2] == "t":
        temp_data.append([int(raw[1]),float(raw[3])])
    
    elif raw[2] == "h":
        humid_data.append([int(raw[1]),float(raw[3])])
        
    
    sec_diff = int((datetime.now()- start_time).total_seconds())
    if sec_diff != 0 and sec_diff % 5 == 0:
        print("====================")
        print("Time: " + str(sec_diff)+"s")
        print("acc_data: " + str(len(acc_data)) + "samples receied at " + str(len(acc_data)/sec_diff)+"Hz")
        print("baro_data: " + str(len(baro_data)) + "samples receied at " + str(len(baro_data)/sec_diff)+"Hz")
        print("light_data: " + str(len(light_data)) + "samples receied at " + str(len(light_data)/sec_diff)+"Hz")
    
def process_acc_data():
    pass

def process_baro_data():
    pass

def process_light_data():
    now_time = datetime.now()
    if(light_utime is None or int((now_time - light_utime).total_seconds()) >= 10):
        readings = numpy.array([baro_data[-1][1], temp_data[-1][1], light_data[-1][1], humid_data[-1][1]])
        # using Bernoulli Naive Bayes model to predict indoor/outdoor status
        result = bool(bnb.predict(readings)[0])
        if result != indoor:
            # update state change
            indoor = result
            indoor_utime = now_time

            if(indoor == True):
                print(str(light_data[-1][0]) + ', INDOOR\n')
            else:
                print(str(light_data[-1][0]) + ', OUTDOOR\n')


def exit_handler():
    fout.close()

atexit.register(exit_handler)
# Connect to mqtt server
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set("cs4222g17test@gmail.com", password="jnuwYcGglObRCNs6")
client.connect("ocean.comp.nus.edu.sg", 1883, 1000)
client.loop_forever()