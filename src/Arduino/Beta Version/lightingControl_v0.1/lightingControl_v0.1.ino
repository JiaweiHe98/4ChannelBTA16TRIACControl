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

  // Initialize input pin
  pinMode(zeroCrossing, INPUT);

  // Initialize output pin
  pinMode(ch1, OUTPUT);
  pinMode(ch2, OUTPUT);
  pinMode(ch3, OUTPUT);
  pinMode(ch4, OUTPUT);

  // Initialize hardware interrupt, start sequence
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

  // for debugging
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

  //Push settings into temporary arrays. If power level is 0, output -1 for skipping the pulse
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

      // for debugging
//      Serial.print("channelNum:");
//      Serial.print(channelNum[i]);
//      Serial.print("states:");
//      Serial.print(states[i]);
//      Serial.print("time:");
//      Serial.println(micros() - startTime);
    }
  }
}
