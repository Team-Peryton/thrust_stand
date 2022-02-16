import time

import serial


record_time = time.strftime('%Y%m%d-%H%M%S')
f = open(f"logs/log_{record_time}.log", "w+")

s = serial.Serial(input("COM PORT:"), 9600)

store = []

try:
    while True:
        newline = s.readline().decode("utf-8")
        print(newline)
        if newline == "":
            raise
        store.append(newline)
except:
    print("Connection closed")
    s.close()
    f.writelines(store)
    f.close()


log = store[3:]
newstore = []
for line in log:
    try:
        filtered = line
        newstore.append(filtered)
    except:
        print("failed to parse a line")

import pandas as pd
pd.DataFrame(newstore).to_csv(f"logs/{record_time}_processed.csv")