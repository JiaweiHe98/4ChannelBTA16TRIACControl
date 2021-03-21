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

  // send initial power level
  sendPower();
}

//Call functions in order
void loop() {
  unsigned long timePassed = micros() - previousMicros;

  // ch1
  if (timePassed > timings[0] && timePassed < timings[0] + 500)
  {
    digitalWrite(ch1, HIGH);
  } else if (timePassed < timings[0] || timePassed > timings[0] + 500)
  {
    digitalWrite(ch1, LOW);
  }
  
  // ch2
  if (timePassed > timings[1] && timePassed < timings[1] + 500)
  {
    digitalWrite(ch2, HIGH);
  } else if (timePassed < timings[1] || timePassed > timings[1] + 500)
  {
    digitalWrite(ch2, LOW);
  }

  // ch3
  if (timePassed > timings[2] && timePassed < timings[2] + 500)
  {
    digitalWrite(ch3, HIGH);
  } else if (timePassed < timings[2] || timePassed > timings[2] + 500)
  {
    digitalWrite(ch3, LOW);
  }

  // ch4
  if (timePassed > timings[3] && timePassed < timings[3] + 500)
  {
    digitalWrite(ch4, HIGH);
  } else if (timePassed < timings[3] || timePassed > timings[3] + 500)
  {
    digitalWrite(ch4, LOW);
  }

  //Receive settings
  if (Serial.available() > 0) {
    int raw = Serial.parseInt();

    // Convet powerlevel to "delay"
    if (raw >= 100 && raw < 500) {
      int i = raw / 100 % 10 - 1;
      power[i] = raw - (i + 1) * 100 + 1;
      timings[i] = map(power[i], 1, powerMax, 8100, 600);
      sendPower();
    }
  }

  //Send back power levels
  if ((micros() - sendBack) > 10000000) {
    sendPower();
    sendBack = micros();
  }
}

void timerReset() {
  previousMicros = micros();
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

}
