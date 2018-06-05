#include <Wire.h>

int pinV = A1;  //Analog pin measuring voltage
int pinA = A0;  //Analog pin measuring current
int pinGate = 3; // digital pin controlling the gate voltage

//int period = 256; //Period time for a square sequence
int duty[256]; //Array filled with values for how long the square part will be in the pulse.
int tracker1=0;
int tracker = -1; //Keeping track of the current sent data series
int measuredV[256]; //Array storing the measured voltages for a series
int measuredA[256]; //Array storing the measured current for a series
byte data[4];
boolean flag=false;
boolean check=false;

void setup() {
 Serial.begin(9600); 
 Wire.begin(1);
  Wire.onRequest(request);
 // Wire.onReceive(event);

  for (int j = 0; j <= 255; j++) { // duty cycle of pwm wave.
    duty[j] = j;
  }
}
void loop() {


//void event(int howMany) {
  
  Serial.print("Measurment series ");
  Serial.print(tracker1);
  Serial.println(" initiated");


  for (int j = 0; j <= 255; j++) {
    
    analogWrite(pinGate, duty[j]); //Creates square waves with the current duty
    delay(3); //Circuit is allowed to enter steadystate

    //int32_t averageV = 0;
    //int32_t averageA = 0;
    int averageV = 0;
    int averageA = 0;
    
    
    for (int i = 0; i < 300; i++) {
    averageV = averageV + analogRead(pinV); //Measures V
    averageA = averageA + analogRead(pinA); //Measures A
    delayMicroseconds(100); //Prevents interferance when measuring  
    }
    measuredV[j] = averageV/300;
    measuredA[j] = averageA/300;

    Serial.println(j);
    if(j==255){
      flag=true;
      Serial.println("klar");
    }
    while(flag){
    }
    
    tracker1++;
    
  }
}


void intToByte(int a, int b) {
data[0]=highByte(a);
data[1]=lowByte(a);
data[2]=highByte(b);
data[3]=lowByte(b);
return;
}

void request() {
 if(flag && tracker==-1){
 check=true;
 tracker++;
 Wire.write('1');
 }
 else if(!check){
  Wire.write('0');
 }
 if(check && tracker>=0){
 intToByte(measuredV[tracker],measuredA[tracker]);
 tracker++;
 if (tracker==255){
 tracker=-1;
 check=false;
 }
 Wire.write(data,4);
}
}


