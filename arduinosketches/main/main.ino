#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

unsigned int localPort = 8888;      // local port to listen on

EthernetUDP Udp;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,

void setup() {
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);

  Serial.begin(9600)
}

float** readData()
{
  // TODO: Read real data
  srand (time(NULL));

  rows = 3;
  cols = 3;
  data = float[rows][cols]
  for(int i=0; i<rows; i++) {
    for(int j=0; j<cols; j++) {
      data[i][j] = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/X));
    }
  }
  
  return data;
}

int handle_ground_commands() 
{
  char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // Read content of UDP package into packetBuffer
    Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // Define commands
    const int GEIGER_OFF = 0
    const int GEIGER_ON = 1
  
    // Handle ground command
    int command = packetBuffer[0];
    switch (command) {
      case GEIGER_OFF:
        // turn off geiger tube
        break;
      case GEIGER_ON:
        // Turn on geiger tube
        break;
    }
  }
}

void loop() {
  /*
     Read and handle commands from ground if there are any
  */

  /*
     Read data
  */
  float** data = readData();

  /*
     Write data to SD-card
  */

  /*
     Send over E-link
  */
  // TODO: Implement real e-link connection
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(ReplyBuffer);
  Udp.endPacket();


  delay(10);
}
