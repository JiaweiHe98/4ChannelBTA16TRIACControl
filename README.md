# 4ChannelBAT16TRIACControl
A program that is used to control four dimmer channels simultaneously.

## Prepare for Developing
This section will guide you through the software and tools required for building this project.

### Visual Studio Code
Visual Studio Code, also known as VSCode, is a popular and powerful text editor for developers. A good text editor is crucial for speeding up the coding process and debugging process.

#### Install Visual Studio Code
We recommend to use the official website for downloading the packages.
Please go to [https://code.visualstudio.com/](https://code.visualstudio.com/) and follow the instruction on the web page.

#### Recommended Extensions
We recommend you to install the listed extensions for boosting your coding experience with VSCode
* Arduino
* C/C++
* Python
* vscode-icons

### Arduino IDE
Arduino IDE is a open-source integrated development environment designated for all Arduino boards. We will use this software to compile our code and burn it to our Arduino. It also has a serial monitor which allows us to interact with Arduino through serial communication.

#### Install Arduino IDE
We also recommend to use the official website for downloading the packages if you are on a Windows or MacOS machine.
Simply go to [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software) and find out your version.
If you want to use a Raspberry Pi or a Linux machine to program the Arduino, you can simply update your software list by ```sudo apt-get update``` and install Arduino IDE by ```sudo apt-get install arduino``` in your console.

### Python
Python is a widely used program language and both python2 and python3 interpreters are pre installed into Raspberry Pi OS and MacOS.

#### Install Python3 for Windows
Go to [https://www.python.org/downloads/](https://www.python.org/downloads/) and choose the version you prefer. Don't forget to add PATH for python interpreter.

### Node-Red
Node-Red is a visual programming tool based on Node.js. It allows you to edit working flows inside a web browser through a wide range of nodes and deploy your flow by simply click deploy button on the right top corner.

#### Install Node-Red
If you are using Raspberry Pi and you chose "Raspberry Pi OS with desktop and recommended software", Node-red is already installed into your Raspberry Pi. Simply click the start button on the top-left and go to programming tab. Then, you will able to find Node-Red.

If you are using a Windows machine, you first need to install Node.js and nmp. Go to [https://nodejs.org/](https://nodejs.org/) and download for Windows. Run the MSI file as administrator. Accept the default settings while installing. When finishing the installation, type ```node --version; npm --version``` in Powershell or ```node --version && npm --version``` in cmd to make sure that your installation is completed.

You should see something similar to this:
```
v14.15.3
6.14.9
```

Then, install Node-Red through ```npm install -g --unsafe-perm node-red``` and add ```node-red``` to your system path.

If you are using Linux, you can install Node-Red with npm, docker, or snap. You can follow the [Documentation](https://nodered.org/docs/getting-started/local) on Node-Red official website.

## Start Developing

## Node-Red Dashboard
<img src="./img/UI.png">
The gauges on the top show the current settings of each channel. The sliders below each gauge allow the users to assign new power level to each channel. We divided the power level into 1000 units. The line charts below the sliders help the users to monitor the history of the settings.

### Real-time interaction with Python scrip and Arduino
<img src="./img/PythonScript.png">
False at the end of each line indicates that there is no difference between the user defined settings and settings stored on Arduino

## Node-Red Flow
<img src="./img/Flow1.PNG">

## Inter-process communication
We use a file to handle inter-process communication. The light intensity settings can be easily read from a file or write to a file with plain text. Therefore, we use a .txt file to store our settings and also to be our API between the Python script and Node-Red. The .txt file only contains 4 lines of integers.

```
100
200
1000
0
```

## Arduino code

```c++
//Main Timer
unsigned long startTime = micros();

// Channel number to pin number
int ch1 = 8;
int ch2 = 9;
int ch3 = 10;
int ch4 = 11;

// Zero-crossing point detection
int zeroCrossing = 2;

// Power output initialization
int power[4] = {0, 0, 0, 0};
int powerMax = 1000;

//Flag
bool isChanged = false;

// Firing queue initialization. 8333 to 0
int timings[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
int channelNum[8] = {ch1, ch1, ch2, ch2, ch3, ch3, ch4, ch4};
bool states[8] = {true, false, true, false, true, false, true, false};

//sand back
unsigned long sendBack = millis();

// Setup serial connection and define pins
void setup() {
    // opens serial port, sets data rate to 115200 bps
    Serial.begin(115200);    

    // Initilize input pin
    pinMode(zeroCrossing, INPUT);

    // Initilize output pin
    pinMode(ch1, OUTPUT);
    pinMode(ch2, OUTPUT);
    pinMode(ch3, OUTPUT);
    pinMode(ch4, OUTPUT);

    // Initilize hardware interrupt, start sequence
    attachInterrupt(digitalPinToInterrupt(zeroCrossing), sequenceStart, FALLING);

    // send initial power level
    sendPower();
}

//Call functions in order
void loop() {

  //Save power
  if (micros() - startTime > 10^7) {
    delay(1000);
  }
  
  //Receive settings
  if (Serial.available() > 0) {
    receivePower();
  }

  //Update settings
  if (isChanged) {
    makeQueue();
    isChanged = false;
  }

  //Send back power levels
  if ((millis() - sendBack) > 10000) {
    sendPower();
    sendBack = millis();
  }

  // for debuging
//  delay(1000);
//  sendPower();
//  sequenceStart();
}


//For receiving settings
void receivePower() {
  for (int i = 0; i < 4; i++) {
    power[i] = Serial.parseInt();
  }
  bufferFlush();
  isChanged = true;
  sendPower();
}

//Send back power levels
void sendPower() {
  Serial.print("[");
  for (int i = 0; i < 4; i++) {
    Serial.print(power[i]);
    if (i < 3) {
      Serial.print(",");
    }
  }
  Serial.println("]");

// For debuging
//  Serial.print("[");
//  for (int i = 0; i < 8; i++) {
//    Serial.print(timings[i]);
//    if (i < 7) {
//      Serial.print(",");
//    }
//  }
//  Serial.println("]");
//  
//  Serial.print("[");
//  for (int i = 0; i < 8; i++) {
//    Serial.print(channelNum[i]);
//    if (i < 7) {
//      Serial.print(",");
//    }
//  }
//  Serial.println("]");
//
//  Serial.print("[");
//  for (int i = 0; i < 8; i++) {
//    Serial.print(states[i]);
//    if (i < 7) {
//      Serial.print(",");
//    }
//  }
//  Serial.println("]");
}

//Flush the remaining garbage information in serial buffer
void bufferFlush() {
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}

//Sending pulse to the dimmer
void firing(int chNum, bool state) {
  if (state) {
    digitalWrite(chNum, HIGH);
  } else {
    digitalWrite(chNum, LOW);
  }
}

//Make a queue for pulling up and pulling down the voltage of control pins
void makeQueue() {

  //Temporary arrays 
  int timeOn[4];
  int timeOnPin[4] = {ch1, ch2, ch3, ch4};
  int timeOff[4];
  int timeOffPin[4] = {ch1, ch2, ch3, ch4};

  //Push settings into temporary arrays. If power level is 0, output -1 for skiping the pulse
  for (int i = 0; i < 4; i++) {
    if (power[i] == 0) {
      timeOn[i] = -1;
      timeOff[i] = -1;
    } else {
      timeOn[i] = map(power[i], 0, powerMax, 8333, 0);
      timeOff[i] = timeOn[i] + 500;
    }
  }

  //Temporary variables for sorting
  int index;
  int minNum;
  int temp;

  //Sort timings of pulling up
  for (int i = 0; i < 3; i++) {
    minNum = timeOn[i];
    index = i;
    for (int j = i + 1; j < 4; j++) {
      if (timeOn[j] < minNum) {
        minNum = timeOn[j];
        index = j;
      }
    }
    if (index != i) {
      temp = timeOn[i];
      timeOn[i] = timeOn[index];
      timeOn[index] = temp;

      temp = timeOnPin[i];
      timeOnPin[i] = timeOnPin[index];
      timeOnPin[index] = temp;
    }
  }

  //sort timings of pulling down
  for (int i = 0; i < 3; i++) {
    minNum = timeOff[i];
    index = i;
    for (int j = i + 1; j < 4; j++) {
      if (timeOff[j] < minNum) {
        minNum = timeOff[j];
        index = j;
      }
    }
    if (index != i) {
      temp = timeOff[i];
      timeOff[i] = timeOff[index];
      timeOff[index] = temp;

      temp = timeOffPin[i];
      timeOffPin[i] = timeOffPin[index];
      timeOffPin[index] = temp;
    }
  }

  //merge 4 array
  int onIndex = 0;
  int offIndex = 0;
  int queueIndex = 0;

  while (onIndex < 4 && offIndex < 4) {
    if (timeOn[onIndex] < timeOff[offIndex]) {
      timings[queueIndex] = timeOn[onIndex];
      channelNum[queueIndex] = timeOnPin[onIndex];
      states[queueIndex] = true;
      onIndex ++;
    } else {
      timings[queueIndex] = timeOff[offIndex];
      channelNum[queueIndex] = timeOffPin[offIndex];
      states[queueIndex] = false;
      offIndex ++;
    }
    queueIndex ++;
  }

  if (onIndex >= 4) {
    while (queueIndex < 8) {
      timings[queueIndex] = timeOff[offIndex];
      channelNum[queueIndex] = timeOffPin[offIndex];
      states[queueIndex] = false;
      offIndex ++;
      queueIndex ++;
    }
  } else {
    while (queueIndex < 8) {
      timings[queueIndex] = timeOn[onIndex];
      channelNum[queueIndex] = timeOnPin[onIndex];
      states[queueIndex] = true;
      onIndex ++;
      queueIndex ++;
    }
  }
  
}

//Set zero crossing time
void sequenceStart() {

  //get start time
  startTime = micros();

  //initialize current time
  unsigned long timeNow;

  //start sequence
  for (int i = 0; i < 8; i ++) {
    timeNow = micros();
    if ((timeNow - startTime) < timings[i]) {
      delayMicroseconds(timings[i] - (timeNow - startTime));
    }

    //skip 0 power level
    if (timings[i] != -1) {
      firing(channelNum[i], states[i]);

      // for debuging
//      Serial.print("channelNum:");
//      Serial.print(channelNum[i]);
//      Serial.print("states:");
//      Serial.print(states[i]);
//      Serial.print("time:");
//      Serial.println(micros() - startTime);
    }
  }
}
```

## Python script on windows or Raspberry Pi

```python
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
    dataFromArduino = arduinoData.readline().decode('ASCII')[:-2]
    end = time.perf_counter()

    #Convert string to list
    power_read = eval(dataFromArduino)

    #Check the power level of the Arduino
    if power_read != power_set:
        arduinoData.write(str(power_set).encode())

    #for debug
    print(power_read, ' ', str(power_set).encode(), ' ', end - start, power_read != power_set)

```