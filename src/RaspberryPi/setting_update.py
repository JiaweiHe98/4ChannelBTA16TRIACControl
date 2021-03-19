'''
This module is used to handle the communicaiton between the Raspberry Pi and arduino
Communication relies on serial connection.
A USB cable can be used for physically connecting the Arduino to Raspberry Pi
'''

# If not found, you need to install pyserial
import serial
import concurrent.futures
import time

# The first argument specify the serial port of the device
# Usually 'COMx' for windows, 'dev/ttyUSBx' for Raspberry Pi
arduinoData = serial.Serial('COM6', 115200)

# Power level initialization, power primarily goes from 0 to 1000
power_set = [0, 0, 0, 0]

# Flag
isChanged = False
lock = False

# Update seetings function
def update_settings():
    global lock

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

                
        # Update setting
        if isChanged:

            while lock:
                pass

            lock = True
            arduinoData.write(str(power_set).encode())
            lock = False

            isChanged = False
        
            
# Function for checking the settings and resend the settings
def check_settings():

    global lock

    while True:
        # Decode message send back from Arduino
        dataFromArduino = arduinoData.readline().decode('ASCII')[:-2]

        # Convert string to list
        power_read = eval(dataFromArduino)

        # Check the power level of the Arduino and update the settings if the settings are different
        if power_read != power_set:

            while lock:
                pass

            lock = True
            arduinoData.write(str(power_set).encode())
            lock = False

        # For debug
        print(power_read, ' ', power_set, ' ', power_read != power_set)


if __name__ == "__main__":

    # Running settings update and checking concurrently
    with concurrent.futures.ThreadPoolExecutor() as executor:
        executor.submit(update_settings)
        executor.submit(check_settings)
