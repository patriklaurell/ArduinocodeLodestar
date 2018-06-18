#include "megacode.h"

void setup()
{
    Serial.begin(9600);
    Serial.println("Start");
    !Thermometer.begin(); 
    Wire.begin();
    //initEthernet();
    //initSDCard();
    initWatchDog();
}

void loop()
{
    Serial.println("Waiting for cigs data..");

    // Wait until recieved cigs data from both nanos.
    while(!recievedFromNano1 && !recievedFromNano2)
    {
        wdt_reset();
        if(!recievedFromNano1)
        {
            recievedFromNano1 = getCigsData(NANO_1_ADDRESS);
        }
        if(!recievedFromNano2)
        {
            recievedFromNano2 = getCigsData(NANO_2_ADDRESS);
        }
        wdt_reset();
    }
    Serial.println(" done!");
    setTimeStamp();
    setFrameNumber();
    formatData();

    
    //wdt_reset();
    //writeToSD();
    //wtd_reset();

    printData();
    // wtd_reset();
    // Udp.beginPacket(remoteIP, REMOTE_PORT);
    // Udp.write(formatedData, 88);
    // Udp.endPacket();
    // wtd_reset();

    // ----  Resetting  ---- //
    recievedFromNano1 = false;
    recievedFromNano2 = false;
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
    //Serial.print("Requesting data from device ");
    //Serial.print(slave); 
    Wire.requestFrom(slave, 1);

    // Slaves will return false if they are busy making measurments.
    if (Wire.available() && !Wire.read())
    {
       //Serial.println("..Slave busy.");
       return false;
    }
    
    //Serial.print("..Slave not busy. ");
    //Serial.print("Bytes recieved: ");
    int it = 0;
    
    // Read data on wire.
    for(int i = 0; i < CIGS_DATA_LEN; i++)
    {
        Wire.requestFrom(slave, 1);
        if(slave == 1)
        {
            cigsData1[i] = Wire.read();
        }
        else // slave == 2
        {
            cigsData2[i] = Wire.read();
        }
        it++;
    }
    Serial.println(it);

    //Serial.println("Successfully recieved data!");
    return true;
}

void getTemperatureData()
{
    double celciusAboveMinus40 = 40 + Thermometer.readTemperature();
    int tempUnitsAboveMinus40 = celciusAboveMinus40 * 65536 / 125;
    temperature[0] = highByte(tempUnitsAboveMinus40);
    temperature[1] = lowByte(tempUnitsAboveMinus40);
    
}

void getRadiationData()
{
    Wire.requestFrom(UNO_ADDRESS, 2);
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
    Serial.print("Number of bytes written: ");
    Serial.println(dataFile.write(formatedData, CIGS_DATA_LEN*4 + 8));
    dataFile.close();
}

void formatData()
{
    // Time stamp
    formatedData[0] = timeStamp[0];
    formatedData[1] = timeStamp[1];

    // Frame number
    formatedData[2] = frameNumber[0];
    formatedData[3] = frameNumber[1];

    // Radaition and temperature
    formatedData[4] = radiation[0];
    formatedData[5] = radiation[1];
    formatedData[6] = temperature[0];
    formatedData[7] = temperature[1];

    // Voltage and current
    for(int i = 0; i < 2*CIGS_DATA_LEN; i++)
    {
        formatedData[8 +                 i] = cigsData1[i];
        formatedData[8 + CIGS_DATA_LEN + i] = cigsData2[i];
    }
} 


    
