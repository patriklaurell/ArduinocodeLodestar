#include <Wire.h>

#define VOLTAGE_PIN         A1
#define CURRENT_PIN         A0
#define ADDRESS              1
#define GATE_VOLTAGE_PIN     3
#define DATA_POINTS         20

int tracker1   = 0;
int tracker   = -1; //Keeping track of the current sent data series
uint8_t cigsDataBuffer[1024]; 
bool allowTransmission = false;
int bufferIndex = -1;

void setup()
{
    Serial.begin(9600); 
    pinMode(LED_BUILTIN, OUTPUT);
    Wire.begin(ADDRESS);
    Wire.onRequest(request);
}

void makeMeasurments()
{
    Serial.print("Measurment series ");
    Serial.print(tracker1);
    Serial.println(" initiated");
    
    for (int j = 0; j <= 255; j++)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        analogWrite(GATE_VOLTAGE_PIN, j); //Creates square waves with the current duty
        delay(3);                         //Circuit is allowed to enter steadystate

        int averageV = 0;
        int averageA = 0;
        
        digitalWrite(LED_BUILTIN, LOW);
        for (int i = 0; i < 300; i++) 
        {
            averageV += analogRead(VOLTAGE_PIN); //Measures voltage.
            averageA += analogRead(CURRENT_PIN); //Measures current.
            delayMicroseconds(100);              //Prevents interferance.   
        }
        // TODO: Maybe not good idea with heltalsdivision.
        cigsDataBuffer[4*j  ] = highByte(j);//averageV/300;
        cigsDataBuffer[4*j+1] = lowByte(j);//averageA/300;
        cigsDataBuffer[4*j+2] = highByte(j);//averageV/300;
        cigsDataBuffer[4*j+3] = lowByte(j);//averageA/300;

    }
    Serial.println("Done makeing measurment.");
}

void request()
{
    Serial.println("Received request from master");
    if (!allowTransmission)
    {
        Serial.println("Transmission is not allowed.");
        Wire.write(false);
    }
    if(allowTransmission && bufferIndex == -1)
    {   
        Serial.println("Transmission is allowed.");
        Wire.write(true);
        bufferIndex++;
    }
    else if(allowTransmission && bufferIndex < 40)
    {
        Serial.print("Transmitting data.");
        Wire.write(cigsDataBuffer[bufferIndex]);
        bufferIndex++;
    }
    else
    {
        bufferIndex = -1;
        allowTransmission = false; 
    }
}

void loop()
{
    makeMeasurments();

    // Waiting for data request.
    allowTransmission = true;
    digitalWrite(LED_BUILTIN, LOW);
    while(allowTransmission)
    {  
        Serial.println("waiting...");
        delay(100);
    }
}
