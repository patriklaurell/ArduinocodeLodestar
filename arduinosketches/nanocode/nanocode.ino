#include <Wire.h>

#define VOLTAGE_PIN         A1
#define CURRENT_PIN         A0
#define ADDRESS              1
#define GATE_VOLTAGE_PIN     3
#define CIGS_DATA_LEN       20

int tracker1   = 0;
int tracker   = -1; //Keeping track of the current sent data series
int measuredV[256]; //Array storing the measured voltages for a series
int measuredA[256]; //Array storing the measured current for a series
byte data[4];
bool allowMeasurement = true;

void setup()
{
    Serial.begin(9600); 
    pinMode(LED_BUILTIN, OUTPUT);
    Wire.begin(ADDRESS);
    Wire.onRequest(request);
}

void loop()
{
    Serial.print("Measurment series ");
    Serial.print(tracker1);
    Serial.println(" initiated");
    
    cli(); 
    digitalWrite(LED_BUILTIN, HIGH);
    for (int j = 0; j <= 255; j++)
    {
        analogWrite(GATE_VOLTAGE_PIN, j); //Creates square waves with the current duty
        delay(3);                         //Circuit is allowed to enter steadystate

        int averageV = 0;
        int averageA = 0;

        for (int i = 0; i < 300; i++) 
        {
            averageV += analogRead(VOLTAGE_PIN); //Measures voltage.
            averageA += analogRead(CURRENT_PIN); //Measures current.
            delayMicroseconds(100);              //Prevents interferance.  
        }
        measuredV[j] = averageV/300;
        measuredA[j] = averageA/300;

    }
    Serial.println("Done makeing measurment.");
    digitalWrite(LED_BUILTIN, LOW);
    sei();
    allowMeasurement = false;
    while(!allowMeasurement)
    {
        delay(100);
    }
}

void request()
{
    // TODO: Make this shorter.
    Serial.println("Received request from master");
    uint8_t dataToSend[2*CIGS_DATA_LEN];
    
    // Splitting voltage data into bytes
    // and stores it into dataToSend. 
    int index = 0;
    for (int i = 0; i < CIGS_DATA_LEN; i++)
    {
        dataToSend[index]  = highByte(measuredV[i]);
        index++;
        dataToSend[index] = lowByte(measuredV[i]);
        index++;
    }

    // Splitting current data into bytes
    // and stores it into dataToSend. 
    for (int i = 0; i < CIGS_DATA_LEN; i++)
    {
        dataToSend[index]  = highByte(measuredA[i]);
        index++;
        dataToSend[index] = lowByte(measuredA[i]);
        index++;
    }
    Wire.write(dataToSend, 2*CIGS_DATA_LEN);
    allowMeasurement = true;
    tracker1++;
}

