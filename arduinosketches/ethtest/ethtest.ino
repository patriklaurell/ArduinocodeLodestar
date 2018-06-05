#include <Ethernet.h>
#include <SPI.h>
#include <EthernetUdp.h>
#include <SD.h>

#define REMOTE_PORT    8888
#define LOCAL_PORT     8888

// These depends on which arduino in in use.
#define SD_SS_PIN         4
#define ETHERNET_SS_PIN  10
#define DATA_BUFFER_LEN  10

// ---- Things for ethernet ---- //
EthernetUDP Udp;
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress remoteIP(192, 168, 0, 2);
IPAddress localIP(192, 168, 0, 200);

// -------- Data stuff -------- //
uint16_t voltageBuffer1[DATA_BUFFER_LEN];
uint16_t currentBuffer1[DATA_BUFFER_LEN];
uint16_t voltageBuffer2[DATA_BUFFER_LEN];
uint16_t currentBuffer2[DATA_BUFFER_LEN];
uint8_t  formatedData[DATA_BUFFER_LEN*4];

void initEthernet()
{
    Ethernet.begin(mac, localIP);
    Udp.begin(LOCAL_PORT);
}

void initSDCard()
{
    if(!SD.begin(SD_SS_PIN))
    {
        Serial.println("Failed initializing SD-card.");
        while(1){}
    }
}

void writeToSD()
{
    File dataFile = SD.open("data", FILE_WRITE);
    Serial.println(dataFile.write(formatedData, DATA_BUFFER_LEN*4));
}

void formatData()
{
    Serial.println("Formating...");
    formatedData[0] = (uint8_t) (millis()/1000);
    Serial.println(formatedData[0]);
    for(int i = 1; i < DATA_BUFFER_LEN; i++)
    {
        formatedData[i] = voltageBuffer1[i];
        formatedData[DATA_BUFFER_LEN+i] = currentBuffer1[i];
        formatedData[2*DATA_BUFFER_LEN+i] = voltageBuffer2[i];
        formatedData[3*DATA_BUFFER_LEN+i] = currentBuffer2[i];
    }
}

void setup()
{
    Serial.begin(9600);
    initEthernet();
    initSDCard();
}


void loop()
{
    Udp.beginPacket(remoteIP, REMOTE_PORT);
    Udp.write("Hello");
    Udp.endPacket();
    delay(1000);
    formatData();
    writeToSD();
}
    
