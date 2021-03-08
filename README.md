# 4ChannelBAT16TRIACControl
A program that is used to control four dimmer channels simultaneously

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

// Power output initilization
int power[4] = {0, 0, 0, 0};
int powerMax = 1000;

//Flag
bool isChanged = false;

// Firing queue initilization. 8333 to 0
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
  
  if (Serial.available() > 0) {
    receivePower();
  }

  if (isChanged) {
    makeQueue();
    isChanged = false;
  }

  if ((millis() - sendBack) > 10000) {
    sendPower();
    sendBack = millis();
  }

  // for debuging
//  delay(1000);
//  sendPower();
//  sequenceStart();
}

void receivePower() {
  for (int i = 0; i < 4; i++) {
    power[i] = Serial.parseInt();
  }
  bufferFlush();
  isChanged = true;
  sendPower();
}

void sendPower() {
  Serial.print("[");
  for (int i = 0; i < 4; i++) {
    Serial.print(power[i]);
    if (i < 3) {
      Serial.print(",");
    }
  }
  Serial.println("]");

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

void bufferFlush() {
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}

void firing(int chNum, bool state) {
  if (state) {
    digitalWrite(chNum, HIGH);
  } else {
    digitalWrite(chNum, LOW);
  }
}

void makeQueue() {
  int timeOn[4];
  int timeOnPin[4] = {ch1, ch2, ch3, ch4};
  int timeOff[4];
  int timeOffPin[4] = {ch1, ch2, ch3, ch4};
  for (int i = 0; i < 4; i++) {
    if (power[i] == 0) {
      timeOn[i] = -1;
      timeOff[i] = -1;
    } else {
      timeOn[i] = map(power[i], 0, powerMax, 8333, 0);
      timeOff[i] = timeOn[i] + 500;
    }
  }

  int index;
  int minNum;
  int temp;

  //sort time on
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

  //sort time off
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

//Set zrossing time
void sequenceStart() {

  //get start time
  startTime = micros();

  //initilize current time
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
    dataFormArduino = arduinoData.readline().decode('ASCII')[:-2]
    end = time.perf_counter()

    #Convert string to list
    power_read = eval(dataFormArduino)

    #Check the power level of the Arduino
    if power_read != power_set:
        arduinoData.write(str(power_set).encode())

    #for debug
    print(power_read, ' ', str(power_set).encode(), ' ', end - start, power_read != power_set)

```