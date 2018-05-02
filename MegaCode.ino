#include <Wire.h>

int measurementV[256];
int measurementC[256];
int temp;
int pressure;
boolean hasMeasurement = false;
int radCount = 0;
boolean radFlag = false;

byte measurement_number = 0;
int endDelay = 0; //delay in ms
int clockDelay = 0; //delay in ms

// Define ground commands
const int GND_CMD_GEIGER_OFF = 0
const int GND_CMD_GEIGER_ON = 1

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



void get_radiation_data() {
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
}

/*
   read from nano i
*/
void populate_data_arrays(int i) {
  Serial.printf("Reading data from nano %d", i);
  byte c = 100;
  Wire.requestFrom(i, 1);
  while(Wire.available()){    // slave may send less than requeste   
    c=Wire.read();
  }
  byte value = 1; //if we read a 1 the series is ready for transmission
  while (c!=value){
    Serial.println(c);
    delay(2000);
    Wire.requestFrom(i, 1);
    while(Wire.available()){    // slave may send less than requeste   
      c=Wire.read(); 
    }
  }
  Serial.println("cont");

  // Read data when nano is ready
  for (int j = 0; j < 256; j++) { //Recieves a measurmentV and a measurmentC array
    int position = 0;
    byte readV[2];
    byte readC[2];

    Wire.requestFrom(i, 4); // recieves one V and one C measurement 4 is for number of bytes
    while (Wire.available()) { //sends an int as two bytes
      if (position < 2) {
        readV[position] = Wire.read(); //recieves voltage value at j
        position++;
      } else {
        readC[position - 2] = Wire.read(); //recieves current value at j
        position++;
      }
      if (position == 4) {
        measurementV[j] = getInt(readV); //assembles the measurments
        measurementC[j] = getInt(readC);
        Serial.printf("V=%d,\tC=%d", measurementV[j], measurementC[j]);
        break;
      }
    }
    if (j == 255) {
      hasMeasurement = true;
    }
  }
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

void get_temp_data() {
  temp += 1;
}

void get_pressure_data() {
  pressure += 1;
}

int handle_ground_command(char command) 
{
  switch (command) {
    case GND_CMD_GEIGER_OFF:
      // turn off geiger tube
      break;
    case GND_CMD_GEIGER_ON:
      // Turn on geiger tube
      break;
  }
}

void loop() {
  /*
     Handle ground commands
  */

  /*
  char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // Read content of UDP package into packetBuffer
    Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // Define commands
  
    // Handle ground command
    char command = packetBuffer[0];
    handle_ground_command(command);
  }
  */

  /*
     Read measurement data
  */
  populate_data_arrays(measurement_nr % 2 + 1);
  // get radiation data
  get_radiation_data();
  // get temp data
  get_temp_data();
  // get pressure data
  get_pressure_data();


  /*
     Send data to ground
  */
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
  measurement_nr++;
  delay(endDelay);


  /*
     Write data to SD-card
  */


  //send radcount
  //radFlag=false

  //temp/preassure measurment?
}

