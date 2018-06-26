#include "nanocode.h"

void setup()
{
    Serial.begin(9600); 
    pinMode(LED_BUILTIN, OUTPUT);
    Wire.begin(ADDRESS);
    Wire.onRequest(requestEvent);
    initWatchDog();
}

void loop()
{
    wdt_reset();
    makeMeasurments(&cigs1);

    // Waiting for data request.
    allowTransmission = true;
    digitalWrite(LED_BUILTIN, LOW);

    while(allowTransmission)
    {  
        wdt_reset();
        Serial.println("waiting...");
        delay(100);
    }

    makeMeasurments(&cigs2);

    // Waiting for data request.
    allowTransmission = true;
    digitalWrite(LED_BUILTIN, LOW);
    while(allowTransmission)
    {  
        wdt_reset();
        Serial.println("waiting...");
        delay(100);
    }

    makeMeasurments(&cigs3);

    // Waiting for data request.
    allowTransmission = true;
    digitalWrite(LED_BUILTIN, LOW);
    while(allowTransmission)
    {  
        wdt_reset();
        Serial.println("waiting...");
        delay(100);
    }
}

void makeMeasurments(CigsCell* cigsCell)
{
    int32_t averageV = 0;
    int32_t averageA = 0;
    for (int j = 0; j < NUMBER_OF_DATA_POINTS; j++)
    {
        wdt_reset();
        digitalWrite(LED_BUILTIN, HIGH);
        analogWrite(cigsCell->gatePin, j); //Creates square waves with the current duty
        delay(SETTLING_TIME);              //Circuit is allowed to enter steadystate

        
        digitalWrite(LED_BUILTIN, LOW);
        for (int i = 0; i < 300; i++) 
        {
            wdt_reset();
            averageV += analogRead(cigsCell->voltagePin); //Measures voltage.
            averageA += analogRead(cigsCell->currentPin); //Measures current.
            delayMicroseconds(INTEGRATING_DELAY);         //Prevents interferance.   
        }
        averageV = (int) (round(averageV / 300.0));
        averageA = (int) (round(averageA / 300.0));
        cigsDataBuffer[4*j  ] = highByte(averageV);
        cigsDataBuffer[4*j+1] = lowByte(averageV);
        cigsDataBuffer[4*j+2] = highByte(averageA);
        cigsDataBuffer[4*j+3] = lowByte(averageA);

    }
    cigsDataBuffer[CIGS_DATA_LEN-1] = cigsCell->cellNumber;
    Serial.print(cigsDataBuffer[CIGS_DATA_LEN-1]);
    Serial.println("Done makeing measurment.");
}

void requestEvent()
{
    wdt_reset();
    if (!allowTransmission)
    {
        Serial.println("Transmission is not allowed.");
        Wire.write(TRANSMISSION_FORBIDDEN);
    }
    else if(allowTransmission && bufferIndex == -1)
    {
        Serial.print("Transmitting data: ");
        Wire.write(TRANSMISSION_ALLOWED);
        bufferIndex++;
    }
    else if(allowTransmission && bufferIndex < CIGS_DATA_LEN)
    {
        Serial.println(bufferIndex);
        Wire.write(cigsDataBuffer[bufferIndex]);
        bufferIndex++;
        if(bufferIndex == CIGS_DATA_LEN)
        {
            bufferIndex = -1;
            allowTransmission = false; 
        }
    }
    else
    {
        Serial.println(" DONE");
        bufferIndex = -1;
        allowTransmission = false; 
    }
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

