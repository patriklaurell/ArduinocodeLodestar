#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>

#define REMOTE_PORT    8888
#define LOCAL_PORT     8888

// These depends on which arduino in in use.
#define SD_SS_PIN         4
#define ETHERNET_SS_PIN  10
#define NANO_1_ADDRESS 0x01 
#define NANO_2_ADDRESS 0x02 
#define CIGS_DATA_LEN    20

// ---- Things for ethernet ---- //
EthernetUDP Udp;
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress remoteIP(192, 168, 0, 2);
IPAddress localIP(192, 168, 0, 200);

// -------- Data stuff -------- //
File dataFile;
uint8_t timeStamp[2];
uint8_t frameNumber[2];
uint8_t voltageBuffer1[CIGS_DATA_LEN];
uint8_t currentBuffer1[CIGS_DATA_LEN];
uint8_t voltageBuffer2[CIGS_DATA_LEN];
uint8_t currentBuffer2[CIGS_DATA_LEN];
uint8_t radiation[2];
uint8_t temperature[2];
uint8_t formatedData[CIGS_DATA_LEN*4+7];

// Initializes the ethernet interface.
void initEthernet()
{
    Ethernet.begin(mac, localIP);
    Udp.begin(LOCAL_PORT);
}

// Initializes the SD-card interface.
void initSDCard()
{
    if(!SD.begin(SD_SS_PIN))
    {
        Serial.println("Failed initializing SD-card.");
        while(1){}
    }
    else
    {
        Serial.println("Successfully initialized SD-card.");
    }
}

void setFrameNumber()
{
    uint16_t frame = word(frameNumber[0], frameNumber[1]);
    frame++;
    frameNumber[0] = highByte(frame);
    frameNumber[1] = lowByte(frame);
}

void setTimeStamp()
{
    uint16_t  seconds = (uint16_t) (millis()/1000);
    timeStamp[0] = highByte(seconds);
    timeStamp[1] = lowByte(seconds); 
}

// Request CIGS data from a slace device. Returns false is slave is busy.
bool getCigsData(int device)
{
    uint8_t index = 0;
    Serial.print("Requesting data from device ");
    Serial.println(device); 
    Wire.requestFrom(1, CIGS_DATA_LEN*1);
    Serial.println("Cigsdata");

    // Return false if slace is busy.
    if(!Wire.available())
    {
       Serial.println("Slave, busy");
       return false;
    }

    // Read data on wire and store in correct data buffer.
    while(Wire.available())
    {
        if(index < CIGS_DATA_LEN)
        {
            voltageBuffer1[index] = Wire.read();
        }
        else
        {
            currentBuffer2[index % CIGS_DATA_LEN] = Wire.read();
        }
        index++;
    }
    Serial.println("Data recieved from slave.");
    return true;
}

void writeToSD()
{
    dataFile = SD.open("data", FILE_WRITE);
    Serial.print("Number of bytes written: ");
    Serial.println(dataFile.write(formatedData, CIGS_DATA_LEN*4 + 8));
    dataFile.close();
}

// Puts all data into formatedData.
void formatData()
{
    // Time stamp
    formatedData[0] = timeStamp[0];
    formatedData[1] = timeStamp[1];

    // Frame number
    formatedData[2] = frameNumber[0];
    formatedData[3] = frameNumber[1];

    // Voltage and current
    for(int i = 4; i < CIGS_DATA_LEN + 4; i++)
    {
        formatedData[i] = voltageBuffer1[i];
        formatedData[CIGS_DATA_LEN+i] = currentBuffer1[i];
        formatedData[2*CIGS_DATA_LEN+i] = voltageBuffer2[i];
        formatedData[3*CIGS_DATA_LEN+i] = currentBuffer2[i];
    }

    // Radaition and temperature
    formatedData[CIGS_DATA_LEN*4 + 4] = radiation[0];
    formatedData[CIGS_DATA_LEN*4 + 5] = radiation[1];
    formatedData[CIGS_DATA_LEN*4 + 6] = temperature[0];
    formatedData[CIGS_DATA_LEN*4 + 7] = temperature[1];
}

void setup()
{
    Serial.begin(9600);
    radiation[1] = 5;
    temperature[1] = 6;
    Wire.begin();
    initEthernet();
    initSDCard();
}

void loop()
{
    setTimeStamp();
    setFrameNumber();
    Serial.print("Time past: ");
    Serial.println(timeStamp[1]);
    delay(1000);
    formatData();
    writeToSD();
//    Udp.beginPacket(remoteIP, REMOTE_PORT);
//    Udp.write(formatedData, 88);
//    Udp.endPacket();
      getCigsData(NANO_1_ADDRESS);

}
    
