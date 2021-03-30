// Store the time of last zero-crossing
unsigned long previousMicros = micros();

//Store the time of last time sending back settings
unsigned long sendBack = micros();

// Channel number to pin number
int ch1 = 8;
int ch2 = 9;
int ch3 = 10;
int ch4 = 11;

// Zero-crossing point detection
int zeroCrossing = 2;

// Power output initialization
int power[4] = {1, 1, 1, 1};
int powerMax = 100;

// Firing "delay"
int timings[4] = {8100, 8100, 8100, 8100};

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

  digitalWrite(ch1, LOW);
  digitalWrite(ch2, LOW);
  digitalWrite(ch3, LOW);
  digitalWrite(ch4, LOW);

  // Initialize hardware interrupt, start sequence
  attachInterrupt(digitalPinToInterrupt(zeroCrossing), timerReset, FALLING);

}

//Call functions in order
void loop() {

  // ch1
  if (micros() - previousMicros > timings[0] && micros() - previousMicros < timings[0] + 500)
  {
    digitalWrite(ch1, HIGH);
  } else {
    digitalWrite(ch1, LOW);
  }
  
  // ch2
  if (micros() - previousMicros > timings[1] && micros() - previousMicros < timings[1] + 500)
  {
    digitalWrite(ch2, HIGH);
  } else {
    digitalWrite(ch2, LOW);
  }

  // ch3
  if (micros() - previousMicros > timings[2] && micros() - previousMicros < timings[2] + 500)
  {
    digitalWrite(ch3, HIGH);
  } else {
    digitalWrite(ch3, LOW);
  }

  // ch4
  if (micros() - previousMicros > timings[3] && micros() - previousMicros < timings[3] + 500)
  {
    digitalWrite(ch4, HIGH);
  } else {
    digitalWrite(ch4, LOW);
  }

  //Receive settings
  if (Serial.available() > 0) {
    int temp;
    for (int i = 0; i < 4; i++) {
      temp = Serial.parseInt();
      if (temp > 0 && temp < 101) {
        power[i] = temp;
        timings[i] = map(power[i], 0, powerMax, 8100, 800);
      } else {
        break;
      }
    }
    bufferFlush();
  }

}

void timerReset() {
  previousMicros = micros();
}

//Flush the remaining garbage information in serial buffer
void bufferFlush() {
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}
