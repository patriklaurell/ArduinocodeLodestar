#include "megacode.h"

uint16_t test = 10;
void setup()
{
    Serial.begin(9600);
    Serial.println("Start");
    pinMode(LED_BUILTIN, OUTPUT);
    Wire.begin();
    if(Thermometer.begin())
    {
        Serial.println("Initializing thermometer successful.");
    }
    else
    {
        Serial.println("Initializing thermometer failed.");
    }
    initEthernet();
    initSDCard();
    initWatchDog();
}

void loop()
{
    // Wait until recieved cigs data from both nanos or 
    // until time out.
    Serial.println("Waiting for cigs data..");
    uint32_t initTime = millis();
    uint32_t timeSpentWaiting{};
    while(!(recievedFromNano1 && recievedFromNano2))
    {
        timeSpentWaiting = millis() - initTime;
        if(timeSpentWaiting > CIGS_DATA_TIMEOUT)
        {
            Serial.print("Cigs data time out!");
            break;
        }

        wdt_reset();
        if(!recievedFromNano1)
        {
            recievedFromNano1 = getCigsData(NANO_1_ADDRESS);
        }
        wdt_reset();
        if(!recievedFromNano2)
        {
            recievedFromNano2 = getCigsData(NANO_2_ADDRESS);
        }
        delay(3000);
    }
    setTimeStamp();
    wdt_reset();
    setFrameNumber();
    wdt_reset();
    getTemperatureData();
    wdt_reset();
    getRadiationData();
    wdt_reset();
    writeToSD();
    wdt_reset();
    sendToGS(); 
    wdt_reset();

    // ----  Resetting  ---- //
    recievedFromNano1 = false;
    recievedFromNano2 = false;
    memset(cigsData1, 0 ,CIGS_DATA_LEN);
    memset(cigsData2, 0 ,CIGS_DATA_LEN);
}

ISR(WDT_vect)
{
    Serial.println("Timeout! Resetting.");
}

void initWatchDog()
{
    cli();
    wdt_reset();
    // Enter config mode.
    WDTCSR |= (1 << WDCE) | (1 << WDE);

    // Set watchdog settings.
    WDTCSR = (1<<WDIE) | (1<<WDE) | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0);
    sei();
}

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

bool getCigsData(int slave)
{
    Serial.print("Requesting data from device ");
    Serial.print(slave); 
    Wire.requestFrom(slave, 1);
    delay(5);
    uint8_t callBack = Wire.read();
    Serial.print(" Callback: ");
    Serial.print(callBack);

    // Slaves will return false if they are busy making measurments.
    if (callBack == TRANSMISSION_FORBIDDEN)
    {
       Serial.println("..Slave busy, transmssion forbidden.");
       return false;
    }
    else if (callBack == TRANSMISSION_ALLOWED)
    {
        Serial.print("..Slave not busy, transmission allowed. ");

        // Read data on wire.
        for(int i = 0; i < CIGS_DATA_LEN; i++)
        {
            wdt_reset();
            digitalWrite(LED_BUILTIN, HIGH);
            Wire.requestFrom(slave, 1);
            if(slave == 1)
            {
                cigsData1[i] = Wire.read();
            }
            if(slave == 2)
            {
                cigsData2[i] = Wire.read();
            }
            digitalWrite(LED_BUILTIN, LOW);
        }

        Serial.print("Successfully recieved data from Nano ");
        Serial.println(slave);

        return true;
    }
    else
    {
       Serial.println("..Slave busy, unknown callback.");
       return false;
    }
}

void getTemperatureData()
{
    double celciusAboveMinus40 = 40 + Thermometer.readTemperature();
    uint16_t tempUnitsAboveMinus40 = celciusAboveMinus40 * 65536 / 125;
    temperature[0] = highByte(tempUnitsAboveMinus40);
    temperature[1] = lowByte(tempUnitsAboveMinus40);
}

void getRadiationData()
{
    Wire.requestFrom(GEIGER_ADDRESS, 2);
    radiation[0] = Wire.read();
    radiation[1] = Wire.read();
}

void printData()
{
    Serial.print("--------    CIGS #");
    Serial.print(cigsData1[CIGS_DATA_LEN-1]);
    Serial.println(" DATA    --------");
    Serial.print("V: ");
    for(int i = 0; i < CIGS_DATA_LEN-1; i+=4)
    {
       Serial.print(word(cigsData1[i], cigsData1[i + 1])); 
       Serial.print(", ");
    }

    Serial.println(" ");
    Serial.print("A: ");
    for(int i = 2; i < CIGS_DATA_LEN-1; i+=4)
    {
       Serial.print(word(cigsData1[i], cigsData1[i + 1])); 
       Serial.print(", ");
    }

    Serial.println(" ");
    Serial.print("--------    CIGS #");
    Serial.print(cigsData2[CIGS_DATA_LEN-1]);
    Serial.println(" DATA    --------");
    Serial.print("V: ");
    for(int i = 0; i < CIGS_DATA_LEN-1; i+=4)
    {
       Serial.print(word(cigsData2[i], cigsData2[i + 1])); 
       Serial.print(", ");
    }

    Serial.println(" ");
    Serial.print("A: ");
    for(int i = 2; i < CIGS_DATA_LEN-1; i+=4)
    {
       Serial.print(word(cigsData2[i], cigsData2[i + 1])); 
       Serial.print(", ");
    }
    Serial.println(" ");
    setTimeStamp();
    Serial.print("Time: ");
    Serial.print(word(timeStamp[0], timeStamp[1]));
    Serial.println(" seconds.");
}

void writeToSD()
{
    dataFile = SD.open("data", FILE_WRITE);
    dataFile.write(timeStamp, 2);
    dataFile.write(frameNumber, 2);
    dataFile.write(radiation, 2);
    dataFile.write(temperature, 2);
    dataFile.write(cigsData1, CIGS_DATA_LEN);
    dataFile.write(cigsData2, CIGS_DATA_LEN);
    dataFile.close();
}

void sendToGS()
{
    Udp.beginPacket(remoteIP, REMOTE_PORT);
    Udp.write(timeStamp, 2);
    Udp.write(frameNumber, 2);
    Udp.write(radiation, 2);
    Udp.write(temperature, 2);
    Udp.endPacket();
    
    Udp.beginPacket(remoteIP, REMOTE_PORT);
    Udp.write(cigsData1, CIGS_DATA_LEN);
    Udp.endPacket();

    Udp.beginPacket(remoteIP, REMOTE_PORT);
    Udp.write(cigsData2, CIGS_DATA_LEN);
    Udp.endPacket();
}


    
