import time

import serial
from serial.tools import list_ports

print(list(list_ports.comports()[0]))

record_time = time.strftime('%Y%m%d-%H%M%S')
f = open(f"logs/log_{record_time}.log", "w+")

s = serial.Serial(f"COM{input('COM PORT:')}", 57600)

store = []


while True:
    try:
        newline = s.readline().decode("utf-8")
        print(newline)
        store.append(newline)
    except UnicodeDecodeError:
        pass
    except KeyboardInterrupt:
        print("Connection closed")
        s.close()
        f.writelines(store)
        f.close()
        break


log = store[:]
newstore = []
for line in log:
    try:
        filtered = line.replace("\n", "").split(",")
        newstore.append(filtered)
    except:
        print("failed to parse a line")

import pandas as pd
pd.DataFrame(newstore).to_csv(f"logs/{record_time}_processed.csv")