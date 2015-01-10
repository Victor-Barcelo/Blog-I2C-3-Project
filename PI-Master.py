import smbus
import time
import struct
from datetime import datetime
import thread

bus = smbus.SMBus(1)

#Arduino[0x04] & [0x05] 
def setDatetimeArduino(address):
    actualTime = int(time.time())
    bytesTuple = struct.unpack("4B", struct.pack("I", actualTime))
    bytes = []
    for item in bytesTuple:
    	bytes.append(item)
    bus.write_i2c_block_data(address, 0, bytes)

#Arduino[0x04] 
def getLightSensorData(delay):
    while True:
        time.sleep(delay)
        lightBytes = bus.read_i2c_block_data(0x04,1,2)
        light = lightBytes[0] + (lightBytes[1] << 8)
        print "Arduino[0x04] -> [Light]: ", light
        print

#Arduino[0x05] 
def getTempSensorData(delay):
    while True:
        time.sleep(delay)
        tempBytes = bus.read_byte_data(0x05,1)
        print "Arduino[0x05] -> [Temp]: ", tempBytes
        print

#Arduino[0x05] 
def getBtnData(delay):
    while True:
        time.sleep(delay)
        byteLen = bus.read_byte_data(0x05,2)
        if byteLen>0:
            buttonInfo = bus.read_i2c_block_data(0x05,3,byteLen)
            buttonPresses = byteLen/4
            data = []
            print "Number of button presses", buttonPresses
            for x in range(0,buttonPresses):
                data.append(buttonInfo[x+(3*x)] + (buttonInfo[x+1+(3*x)] << 8) + (buttonInfo[x+2+(3*x)] << 16) + (buttonInfo[x+3+(3*x)] << 24)) 
            for btnTime in data:
                print "Button pressed at: ", datetime.fromtimestamp(btnTime)
                print 

#Arduino[0x04] 
def getBtnState(delay):
    while True:
        time.sleep(delay)
        isButtonPressed = bus.read_byte_data(0x04,2)
        if isButtonPressed:
            print "Arduino[0x04] -> Button pressed"
            print

#Main
setDatetimeArduino(0x04)
setDatetimeArduino(0x05)
try:
   thread.start_new_thread(getTempSensorData, (3, ))
   thread.start_new_thread(getLightSensorData, (8, ))
   thread.start_new_thread(getBtnData, (5, ))
   thread.start_new_thread(getBtnState, (0.75, ))
except:
   print "Error: unable to start thread"
while 1:
   pass