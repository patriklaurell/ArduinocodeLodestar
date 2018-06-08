/*
 UDPSendReceiveString:
 This sketch receives UDP message strings, prints them to the serial port
 and sends an "acknowledge" string back to the sender

 A Processing sketch is included at the end of file that can be used to send
 and received messages for testing with a computer.

 created 21 Aug 2010
 by Michael Margolis

 This code is in the public domain.
 */


#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <string.h>
#include <SD.h>

#define SS_SD_CARD   4
#define SS_ETHERNET  10

// Define general constants
const char MEASUREMENT_ARRAY_LEN = 10;

const char GND_CMD_GEIGER_OFF = 0;
const char GND_CMD_GEIGER_ON = 1;
const char GND_CMD_TRANS_OFF = 2;
const char GND_CMD_TRANS_ON = 3;

short measurementV[MEASUREMENT_ARRAY_LEN];
short measurementC[MEASUREMENT_ARRAY_LEN];
int temp = 0;
int pressure = 0;
int radCount = 0;

byte measurement_count = 0;
int endDelay = 0; //delay in ms
int clockDelay = 0; //delay in ms


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 200);
IPAddress remoteIp(192, 168, 0, 2);

unsigned int localPort = 8888;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void setup() {
  // Setup SD-card  
  /*
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin()) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);  
  }
  Serial.println("card initialized.");
  
  digitalWrite(SS_SD_CARD, HIGH);
  */
  
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.println("Set up UDP");

  digitalWrite(SS_ETHERNET, HIGH);
  
  
 

  Serial.begin(9600);
}



byte* formatDataArray() {
  byte data[MEASUREMENT_ARRAY_LEN*4];
  for (int i=0; i<MEASUREMENT_ARRAY_LEN; i++) {
    data[i*2] = highByte(measurementC[i]);
    data[i*2 + 1] = lowByte(measurementC[i]);
    data[i*2 + 2] = highByte(measurementV[i]);
    data[i*2 + 3] = lowByte(measurementV[i]);
  }
  return data;
}

void emmulateDataArrays() {
  for (int i=0; i<MEASUREMENT_ARRAY_LEN; i++) {
    measurementV[i] = i;
    measurementC[i] = i;
  }
}

int recievePacket(char* packetBuffer, int bufferSize) {
  /*
  Serial.print("Received packet of size: ");
  Serial.println(packetSize);
  Serial.print("From: ");
  IPAddress remote = Udp.remoteIP();
  for (int i = 0; i < 4; i++) {
    Serial.print(remote[i], DEC);
    if (i < 3) {
      Serial.print(".");
    }
  }
  Serial.print(", port: ");
  Serial.println(Udp.remotePort());
  */
  // read the packet into packetBufffer
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    memset(packetBuffer, 0, bufferSize);
    Udp.read(packetBuffer, bufferSize);

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();

    return 1;
  }
  return 0;
}

void handleGroundCommands(char* packet) {
  int command = atoi(packet);
  Serial.print("Recieved command: ");

  switch (command) {
    case GND_CMD_GEIGER_OFF:
      Serial.println("GEIGER_OFF");
      break;
    case GND_CMD_GEIGER_ON:
      Serial.println("GEIGER_ON");
      break;
  }
}

void sendData(byte* data, int dataSize, IPAddress remoteIp, int port) {
  digitalWrite(SS_ETHERNET, LOW);
  
  Serial.print("Sending data to ");
  Serial.println(remoteIp);
  Udp.beginPacket(remoteIp, port);
  Serial.println(1);
  byte sent = Udp.write(data, dataSize);
  Serial.println(2);
  int success = Udp.endPacket();
  Serial.print("success = ");
  Serial.println(success);
  Serial.print(sent);
  Serial.println(" bytes sent");

  digitalWrite(SS_ETHERNET, HIGH);
}


void loop() {
  /*
   *  Recieve and handle ground commands
   
  // if there's data available, read a packet
  int newPacket = recievePacket(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
  // if new packet handle commands
  if (newPacket) {
    handleGroundCommands(packetBuffer);  
  }


  /*
   *  Fill up data arrays with measurements
   */
  //populate_data_arrays(measurement_count % 2 + 1);
  emmulateDataArrays();
 

  /* 
   *  Format data
   */
  byte* data = formatDataArray();

  /*
   *  Send data to ground
   */
  sendData(data, 4*MEASUREMENT_ARRAY_LEN, remoteIp, 8888);

  /*
   *  Save data to SD-card
   
  char* filename = "data2.txt";
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    Serial.println("Writing data to file...");
    int i = dataFile.write(data, 40);
    Serial.print(i);
    Serial.println(" bytes written");
    dataFile.close();
  }
  else { // if the file isn't open, pop up an error:
    Serial.print("error opening ");
    Serial.println(filename);
  }
  */
  
  
  delay(2000);
}


/*
  Processing sketch to run with this example
 =====================================================

 // Processing UDP example to send and receive string data from Arduino
 // press any key to send the "Hello Arduino" message


 import hypermedia.net.*;

 UDP udp;  // define the UDP object


 void setup() {
 udp = new UDP( this, 6000 );  // create a new datagram connection on port 6000
 //udp.log( true );         // <-- printout the connection activity
 udp.listen( true );           // and wait for incoming message
 }

 void draw()
 {
 }

 void keyPressed() {
 String ip       = "192.168.1.177"; // the remote IP address
 int port        = 8888;        // the destination port

 udp.send("Hello World", ip, port );   // the message to send

 }

 void receive( byte[] data ) {          // <-- default handler
 //void receive( byte[] data, String ip, int port ) {   // <-- extended handler

 for(int i=0; i < data.length; i++)
 print(char(data[i]));
 println();
 }
 */
