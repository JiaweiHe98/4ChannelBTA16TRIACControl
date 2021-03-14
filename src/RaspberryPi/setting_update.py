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

# Reading settings from a file
while True:

    # Flag
    isChanged = False

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

            
    # Update setting
    if isChanged:
        arduinoData.write(str(power_set).encode())
        isChanged = False

    # Decode message send back from Arduino
    start = time.perf_counter()
    dataFromArduino = arduinoData.readline().decode('ASCII')[:-2]
    end = time.perf_counter()

    # Convert string to list
    power_read = eval(dataFromArduino)

    # Check the power level of the Arduino and update the settings if the settings are different
    if power_read != power_set:
        arduinoData.write(str(power_set).encode())

    # For debug
    print(power_read, ' ', str(power_set).encode(), ' ', end - start, power_read != power_set)
