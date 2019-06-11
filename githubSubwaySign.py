from google.transit import gtfs_realtime_pb2
from protobuf_to_dict import protobuf_to_dict
import numpy as np
import requests
import time
import serial
import struct           #first we import all the necessary libraries

ser = serial.Serial('COM3', 250000)     #we connect to an arduino uno which will serve as antenna

time.sleep(1)

api_key = 'Your API Key'            #get your key at https://datamine.mta.info/
station145thBD = ['D13', '21']      #these are the reference given by the MTA to get data from specific station and line
station145thAC = ['A12', '26']
station125thBD = ['A15', '21']
station125thAC = ['A15', '26']

TrainL = ['A', 'B', 'C', 'D']
direction = ['N', 'S']
Mode = 1                            #the variable is used to keep track of the stat of the sign 


def trainTimeLookUp(station):           #This function fetches the data from the MTA and only keeps the data for the stations of interest
    feed = gtfs_realtime_pb2.FeedMessage()
    response = requests.get('http://datamine.mta.info/mta_esi.php?key={}&feed_id={}'.format(api_key, station[1]))
    feed.ParseFromString(response.content)
    subway_feed = protobuf_to_dict(feed)

    realtime_data = subway_feed['entity']

    collected_times = []

    for trains in realtime_data:
        if trains.get('trip_update', False) != False:
            unique_train_schedule = trains['trip_update']
            if unique_train_schedule.get('stop_time_update', False) != False:
                ExpTime = unique_train_schedule.get('stop_time_update')
                trainInfo = unique_train_schedule['trip']
                trainLine = trainInfo['route_id']
                for stop in ExpTime:
                    if stop.get('stop_id') == station[0] + 'N':
                        output = stop.get('departure')
                        collected_times.append([trainLine, 'N', output.get('time')])
                    elif stop.get('stop_id') == station[0] + 'S':
                        output = stop.get('departure')
                        collected_times.append([trainLine, 'S', output.get('time')])
    return np.asarray(collected_times)

def MTAschedule(MTAData):           #once we have the information from the MTA for the station of interest we organise the time chronological (we have time stamps not time to arrival of train)
    L = len(TrainL)
    D = len(direction)

    schedule = np.zeros([L,D,40])

    for line in MTAData:
        for data in line:
            for t in range(L):
                for d in range(D):
                    if data[0] == TrainL[t] and data[1] == direction[d]:
                        schedule[t,d,1:] = schedule[t,d,:39]
                        schedule[t,d, 0] = data[2]

    return schedule

def timeToTrain(timeData):          #Read time stamp and converts it in minutes to arrival of trains the return the 2 closest train for each train and direction
    nextTrains = np.zeros([4,2,2])
    dataCollected = timeData

    for t in range(len(TrainL)):
        for d in range(len(direction)):
            dataCollected[t,d] -= int(time.time())
            dataCollected[t,d] /= 60

            for x in range(len(dataCollected[t,d])):
                if dataCollected[t,d,x] > 2 and dataCollected[t,d,x] < 99:
                    dataCollected[t,d,x] = int(dataCollected[t,d,x])
                else:
                    dataCollected[t,d,x] = 99
            dataCollected[t,d].sort()
            nextTrains[t,d] = dataCollected[t,d,0:2]
    
    return nextTrains

def sendData(toSend):               #Sends data to arduino uno to wirelessly communicate it to the sign 
    ser.write(struct.pack('>B', 100))   #dummy variable used to insure integrity of data transmitted

    for d in range(4):
        for w in range(2):
            ser.write(struct.pack('>B', int(toSend[d,w,0])))
            ser.write(struct.pack('>B', int(toSend[d,w,1])))

def signMode():                         #fetches information on the state of the sign
    onOff = 2                           # if the sign isn't plugged in this function will return a value of 2
    while ser.in_waiting > 0:
        onOff = ser.read()
    return onOff
    ser.flushInput()


while True:
    try:
        TData = [trainTimeLookUp(station145thBD), trainTimeLookUp(station145thAC)]
        print("Data successfully loaded")
        
    except:
        print('a problem occured when loading data from MTA server')

    for refresh in range(3):
        RawTimes = MTAschedule(TData)
        timesToDisplay = timeToTrain(RawTimes)
        sendData(timesToDisplay)
        time.sleep(1)
        Mode = signMode()
        for frame in range(14):        
            if Mode == struct.pack('>B', 1):
                time.sleep(1)
                Mode = signMode()
                

    
    if Mode == struct.pack('>B', 0):
        print("sign off")
        while Mode == struct.pack('>B', 0):
            time.sleep(1)
            Mode = signMode()

    if Mode == 2:
        print("sign not powered or out of range")
        while Mode == 2:
            time.sleep(1)
            Mode = signMode()

