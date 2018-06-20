#include <Wire.h>
#include <Lodestar-constants.h>

// One change this to NANO_1_ADDRESS or NANO_2_ADDRESS. Take note witch Nano is
// loaded with witch address. The cigs data from Nano 1 will be labled 1-3 and
// the data from Nano 2 will be labled 4-6.
#define ADDRESS    NANO_1_ADDRESS

// -------- DATA STUFF -------- //

uint8_t cigsDataBuffer[CIGS_DATA_LEN]; 
bool allowTransmission = false;
int bufferIndex = -1;

// Structs representing the three cigs cells connected to each Nano.
CigsCell cigs1{VOLTAGE_PIN_1, CURRENT_PIN_1, GATE_PIN_1, 1+(ADDRESS-1)*3};
CigsCell cigs2{VOLTAGE_PIN_2, CURRENT_PIN_2, GATE_PIN_2, 2+(ADDRESS-1)*3};
CigsCell cigs3{VOLTAGE_PIN_3, CURRENT_PIN_3, GATE_PIN_3, 3+(ADDRESS-1)*3};

// -------- FUNCTIONS -------- //

// Takes measurments from the cigsCell by increamenting the voltage over the
// panel and then measure the voltage and current over the panel. The data will be
// stored in cigsDataBuffer with the format:
// [voltage high byte, voltage low byte,
//  current high byte, current low byte,...., cigsNumber] 
void makeMeasurments(CigsCell* cigsCell);

// Runs when the Arduino Mega requets data. If allowTranmisson, a 1 will be
// sent to the mega followed by the contet of cigsDataBuffer.
void requestEvent();

