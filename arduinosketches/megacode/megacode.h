#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Lodestar-constants.h>

Adafruit_BME280 Thermometer;
EthernetUDP Udp;
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress remoteIP(192, 168, 0, 2);
IPAddress localIP(192, 168, 0, 200);

// -------- Data stuff -------- //
File dataFile;
uint8_t timeStamp[2];
uint8_t frameNumber[2];
uint8_t radiation[2];
uint8_t temperature[2];
uint8_t cigsData1[CIGS_DATA_LEN];
uint8_t cigsData2[CIGS_DATA_LEN];
uint8_t formatedData[CIGS_DATA_LEN*2+7];

bool recievedFromNano1 = false;
bool recievedFromNano2 = false;

// -------- Functions -------- //

// Initiates the ethernet interface and the SD-card interface.
void initEthernet();
void initSDCard();

// Increments the frame number and stores it as high byte and low byte in
// frameNumber.
void setFrameNumber();

// Stores the number of seconds past since the Mega was turned on as high byte
// and low byte in timeStamp.
void setTimeStamp();

// Takes the Nano number, 1 or 2, and loades the data into
// cigsData 1 or cigsData2. The data will be
// stored with the format:
// [voltage high byte, voltage low byte,
//  current high byte, current low byte,...., cigsNumber]. Will return True if
//  succsessful and False if not.
bool getCigsData(int slave);

// Reads the temperature in celius and stores it as the number of temperature
// units above -40. Each unit is (84+125) / 2^16 ≈ 0.001 C. The data is stored
// as high byte and low byte in temperatureData.
void getTemperatureData();

// Reads the radiation level from the Ardino Uno and stores it as high byte
// and low byte in radiationData.
void getRadiationData();

// Takes all the data arrays and puts them into formatedData. The order is:
// 1. Time stamp      
// 2. Frame number    
// 3. Radiation level 
// 4. Temperature     
// 5. Cigs data from Nano 1          
// 6. Cigs data from Nano 2          
void formatData();

// Writes formatedData to the SD-card.
void writeToSD();

// Prints the data to Serial. Used for debugging.
void printData();
