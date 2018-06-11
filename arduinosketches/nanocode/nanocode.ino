#include <Wire.h>
#include <Lodestar-constants.h>

#define ADDRESS    NANO_2_ADDRESS

uint8_t cigsDataBuffer[CIGS_DATA_LEN]; 
bool allowTransmission = false;
int bufferIndex = -1;

CigsCell cigs1{VOLTAGE_PIN_1, CURRENT_PIN_1, GATE_PIN_1, 1+(ADDRESS-1)*3};
CigsCell cigs2{VOLTAGE_PIN_2, CURRENT_PIN_2, GATE_PIN_2, 2+(ADDRESS-1)*3};
CigsCell cigs3{VOLTAGE_PIN_3, CURRENT_PIN_3, GATE_PIN_3, 3+(ADDRESS-1)*3};

void setup()
{
    Serial.begin(9600); 
    pinMode(LED_BUILTIN, OUTPUT);
    Wire.begin(ADDRESS);
    Wire.onRequest(requestEvent);
}

void makeMeasurments(CigsCell* cigsCell)
{
    for (int j = 0; j < NUMBER_OF_DATA_POINTS; j++)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        analogWrite(cigsCell->gatePin, j); //Creates square waves with the current duty
        delay(3);                         //Circuit is allowed to enter steadystate

        int averageV = 0;
        int averageA = 0;
        
        digitalWrite(LED_BUILTIN, LOW);
        for (int i = 0; i < 300; i++) 
        {
            averageV += analogRead(cigsCell->voltagePin); //Measures voltage.
            averageA += analogRead(cigsCell->currentPin); //Measures current.
            delayMicroseconds(100);              //Prevents interferance.   
        }
        // TODO: Maybe not good idea with heltalsdivision.
        cigsDataBuffer[4*j  ] = highByte(j);//averageV/300;
        cigsDataBuffer[4*j+1] = lowByte(j);//averageA/300;
        cigsDataBuffer[4*j+2] = highByte(j);//averageV/300;
        cigsDataBuffer[4*j+3] = lowByte(j);//averageA/300;

    }
    cigsDataBuffer[CIGS_DATA_LEN-1] = cigsCell->cellNumber;
    Serial.print(cigsDataBuffer[CIGS_DATA_LEN-1]);
    Serial.println("Done makeing measurment.");
}

void requestEvent()
{
    if (!allowTransmission)
    {
        Serial.println("Transmission is not allowed.");
        Wire.write(false);
    }
    else if(allowTransmission && bufferIndex == -1)
    {
        Serial.print("Transmitting data: ");
        Wire.write(true);
        bufferIndex++;
    }
    else if(allowTransmission && bufferIndex < CIGS_DATA_LEN)
    {
        Serial.print(cigsDataBuffer[bufferIndex]);
        Wire.write(cigsDataBuffer[bufferIndex]);
        bufferIndex++;
    }
    else
    {
        Serial.println(" DONE");
        bufferIndex = -1;
        allowTransmission = false; 
    }
}

void loop()
{
    makeMeasurments(&cigs1);

    // Waiting for data request.
    allowTransmission = true;
    digitalWrite(LED_BUILTIN, LOW);
    while(allowTransmission)
    {  
        Serial.println("waiting...");
        delay(100);
    }
}
