'''
This module is used to handle the communicaiton between the Raspberry Pi and arduino
Communication relies on serial connection.
A USB cable can be used for physically connecting the Arduino to Raspberry Pi
'''

# If not found, you need to install pyserial
import serial
import time

# The first argument specify the serial port of the device
# Usually 'COMx' for windows, 'dev/ttyUSBx' for Raspberry Pi
arduinoData = serial.Serial('COM6', 115200)

# Power level initialization, power primarily goes from 0 to 1000
power_set = [0, 0, 0, 0]

# Flag
isChanged = False

# Reading settings from a file
while True:

    # Read the light intensity from file
    with open("lightIntensity.txt", "r") as light_intensity:

        # Read the light intensity from lines
        light_levels = light_intensity.readlines()
        
        # Update light intensity for each channel in order
        for i in range(4):

            # Parse int from string
            try:
                light_level = int(light_levels[i])

                if power_set[i] != light_level:
                    power_set[i] = light_level
                    isChanged = True

            # Print out error signal
            except:
                print('input file error!')

    time.sleep(0.5)
            
    # Update setting
    if isChanged:
        arduinoData.write(str(power_set).encode())
        