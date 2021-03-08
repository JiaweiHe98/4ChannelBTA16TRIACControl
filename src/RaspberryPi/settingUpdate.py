'''
This module is used to handle the communicaiton between the Raspberry Pi and arduino
Communication relies on serial connection.
A USB cable can be used for physically connecting the Arduino to Raspberry Pi
'''

import serial
import time

#The first argument specify the serial port of the device
#Usually 'COMx' for windows, 'dev/ttyUSBx' for Raspberry Pi
arduinoData = serial.Serial('COM6', 115200)

#Power level initialization, power primarily goes from 0 to 1000
power_set = [0, 0, 0, 0]

#Reading settings from a file
while True:

    #Flag
    isChanged = False

    with open("lightIntensity.txt", "r") as light_intensity:

        light_levels = light_intensity.readlines()
        
        for i in range(4):

            try:
                light_level = int(light_levels[i])

                if power_set[i] != light_level:
                    power_set[i] = light_level
                    isChanged = True

            except:
                print('input file error!')

            
    #Update setting
    if isChanged:
        arduinoData.write(str(power_set).encode())
        isChanged = False

    #Decode message send back from Arduino
    start = time.perf_counter()
    dataFormArduino = arduinoData.readline().decode('ASCII')[:-2]
    end = time.perf_counter()

    #Convert string to list
    power_read = eval(dataFormArduino)

    #Check the power level of the Arduino
    if power_read != power_set:
        arduinoData.write(str(power_set).encode())

    #for debug
    print(power_read, ' ', str(power_set).encode(), ' ', end - start, power_read != power_set)
