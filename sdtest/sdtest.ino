#include <Wire.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <SD.h>

// Define general constants
const char MEASUREMENT_ARRAY_LEN = 10;

short measurementV[MEASUREMENT_ARRAY_LEN];
short measurementC[MEASUREMENT_ARRAY_LEN];
int temp = 0;
int pressure = 0;
int radCount = 0;

byte measurement_count = 0;
int endDelay = 0; //delay in ms
int clockDelay = 0; //delay in ms

// SD-card file


void setup() {
  Serial.begin(9600); // only for printing

  // Setup SD-card
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin()) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

  
}


void get_temp_data() {
  temp += 1;
}

void get_pressure_data() {
  pressure += 1;
}


String format_data_string() {
  String s = "";
  for (int i=0; i<MEASUREMENT_ARRAY_LEN; i++) {
    s += (String(measurementC[i]) + "," + String(measurementV[i]) + "\n");
  }
  return s;
}

void emmulate_data_arrays() {
  for (int i=0; i<MEASUREMENT_ARRAY_LEN; i++) {
    measurementV[i] = i;
    measurementC[i] = i;
  }
}

void emmulate_rad_data() {
  radCount = 12;
}

void loop() {
  Serial.println("loopin");
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
  
    // Handle command
    char command = packetBuffer[0];
    handle_ground_command(command);
  }
  */


  /*
     Read measurement data
  */
  //populate_data_arrays(measurement_count % 2 + 1);
  emmulate_data_arrays();
  // get radiation data
  //get_radiation_data();
  emmulate_rad_data();
  // get temp data
  get_temp_data();
  // get pressure data
  get_pressure_data();

  
  /*
     Format data to string
  */
  String dataString = format_data_string();
  Serial.println(dataString);


  /*
     Write data to SD-card
  */
  File dataFile = SD.open("data.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println("Writing data to file");
  }
  else {// if the file isn't open, pop up an error:
    Serial.println("error opening datalog.txt");
  }


  /*
     Send data to ground
  */


  measurement_count++;
  delay(endDelay);
}

