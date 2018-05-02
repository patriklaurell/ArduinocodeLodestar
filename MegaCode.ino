#include <Wire.h>

int measurementV[256];
int measurementC[256];
boolean hasMeasurement = false;
int radCount = 0;
boolean radFlag = false;

byte x = 0;
int endDelay = 0; //delay in ms
int clockDelay = 0; //delay in ms


void setup() {
  Serial.begin(9600); // only for printing
  Wire.begin();

  //for (int i = 1; i <= 1; i++) {
  //delay(clockDelay);
  //Wire.beginTransmission(i); // transmit to device #1,2,3
  //Wire.write(1);              // sends one bit, 1 means initialize
  //Wire.endTransmission();    // stop transmitting
  //delay(0); //ms to prevent flooding the first nanos with recieve requests
  // }
}


void loop() {
  //change to 2 in second argument later
  for (int i = 1; i <= 1; i++) { //Reading from diffrent nanos
    Serial.println(i);
    byte c = 100;
    Wire.requestFrom(1, 1);
    while(Wire.available()){    // slave may send less than requeste   
      c=Wire.read();
      Serial.println("hej");
    }
    byte value = 1; //if we read a 1 the series is ready for transmission
    while (c!=value){
      Serial.println(c);
      delay(2000);
      Wire.requestFrom(1, 1);
      while(Wire.available()){    // slave may send less than requeste   
        c=Wire.read(); 
      }
    }
    Serial.println("cont");


    for (int j = 0; j < 256; j++) { //Recieves a measurmentV and a measurmentC array
      Serial.println("loop");
      Serial.println(j);

      delay(10);
      int position = 0;
      byte readV[2];
      byte readC[2];

      Wire.requestFrom(i, 4); // recieves one V and one C measurement 4 is for number of bytes
      int f=0;

      while (Wire.available()) { //sends an int as two bytes
        f++;
        if (position < 2) {
          readV[position] = Wire.read(); //recieves voltage value at j


          position++;
        } else {
          readC[position - 2] = Wire.read(); //recieves current value at j
          position++;
        }
        if (position == 4)
        {
          measurementV[j] = getInt(readV); //assembles the measurments
          measurementC[j] = getInt(readC);
          Serial.println("Measurment values");
          Serial.println(measurementV[j]);
          Serial.println(measurementC[j]);
          break;
        }
      }
      Serial.println("number of loops");
      Serial.println(f);
      if (j == 255) {
        hasMeasurement = true;
      }
    }
    delay(1000);
    if (hasMeasurement == true) {
      Serial.println("Z");

      Serial.println(measurementV[0]);
      Serial.println(measurementC[0]);
      //Sends the measurment away

      //Wire.beginTransmission(i); // transmit to device #1,2,3
      //Wire.write(1);              // sends one bit, 1 means reinitialize
      //Wire.endTransmission();    // stop transmitting
      hasMeasurement = false;
    }
    x++;
    delay(endDelay);
  }

  Wire.requestFrom(3, 2); // recieves one int from UNO
  byte readR[2];
  int position = 0;

  while (Wire.available()) { //sends an int as two bytes
    readR[position]=Wire.read();
    position++;
    if (position==2){
      radCount=getInt(readR);
      radFlag=true;
    }
  }

  //send radcount
  //radFlag=false

  //temp/preassure measurment?
}

// function converts an array of two bytes into an int
int getInt (byte B[]) {
  int first = (2 ^ 8) * (int)(B[0]);
  int second = (int)(B[1]);
  //Serial.println("first");
  //Serial.println(first);
  //Serial.println(second);
  return first + second;
}
