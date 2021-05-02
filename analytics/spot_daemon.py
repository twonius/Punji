from pymongo import MongoClient
# pprint library is used to make the output look more pretty
from prettyprinter import pprint
from bson.objectid import ObjectId
import datetime
import pandas
import matplotlib.pyplot as plt
from filterpy.kalman import KalmanFilter
from filterpy.common import Q_discrete_white_noise

from scipy.signal import butter,filtfilt,find_peaks
import numpy as np 


# Filter requirements.
T = 5.0         # Sample Period
fs = 30.0       # sample rate, Hz
cutoff = 2      # desired cutoff frequency of the filter, Hz ,      slightly higher than actual 1.2 Hz
nyq = 0.5 * fs  # Nyquist Frequency
order = 1       # sin wave can be approx represented as quadratic
n = int(T * fs) # total number of samples

def butter_lowpass_filter(data, cutoff, fs, order):
    normal_cutoff = cutoff / nyq
    # Get the filter coefficients 
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    y = filtfilt(b, a, data)
    return y
avg_window = 50
days = 7 # window for data processing
lag = int(20) # SAMPLES

readingWeight = []
readingTime= []
y_lag=[]

# connect to MongoDB, change the << MONGODB URL >> to reflect your own connection string
client = MongoClient('mongodb+srv://user:yqpAl6Q4oHLliI00@beta.d501j.mongodb.net/spot?retryWrites=true&w=majority')
database = client.spot
sensordata = database.sensordatas



gen_time = datetime.datetime.utcnow() - datetime.timedelta(days=days) 
dummy_id = ObjectId.from_datetime(gen_time)

query = {"userID" : 1, "_id" :{"$gte": dummy_id}}
fltr = {"_id" : 0, "userID" : 0, "weight" : 0, "battery" : 0, "timestamp": 1}

for data in sensordata.find(query): 
    readingWeight.append(data["weight"])
    #readingTime.append(data["_id"].generation_time)
    readingTime.append(pandas.to_datetime(data["timestamp"], format = '%b %d %y , %H:%M:%S', infer_datetime_format=True)) #because minute is not zero padded this code is needed
   

plt.plot(readingTime,readingWeight)


# Filter the data, and plot both the original and filtered signals.
y = butter_lowpass_filter(readingWeight, cutoff, fs, order)


plt.plot(readingTime,y,'r-')
plt.show()

# list is sorted so we can take the middle of the window range as the median 
# sits = y.sort()[-round(avg_window/2)]
# stands = y.sort()[round(avg_window/2)]
# 
# print(stands-sits)

i=0

while i < len(y)-lag: 
    y_lag.append(y[i+lag]-y[i])
    i+=1


plt.figure()
plt.plot(y_lag)
plt.show()