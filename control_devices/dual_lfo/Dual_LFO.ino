/* 
 * Pin A0 = Speed 
 * Pin A1 = Shape 
 * 
 * Pin 5 = PWM Out 
 * Pin 13 = LED / Square Out 
 * 
 * TODO:
 * -learn how to put wavetables in memory
 * -add pots to control to offset
 * -add buttons to control LFO A and LFO B wavetable shapes (1,2,3)
 */ 
#include "wavetables.h"

const bool debug = 0;

// Declaring some Variables. All are unsigned, since you don't really need negative going values for now.
unsigned int delayTime;      

unsigned int shapeA;
unsigned int shapeB; 

uint8_t tableStepA;        //8-Bit unsigned integer, so it can only store a value between 0 and 255
uint8_t tableStepA_offset; //8-Bit unsigned integer, so it can only store a value between 0 and 255
uint8_t tableStepB;        //8-Bit unsigned integer, so it can only store a value between 0 and 255
uint8_t tableStepB_offset; //8-Bit unsigned integer, so it can only store a value between 0 and 255
uint8_t offset_a;          //amount of steps to offset LFO A to make LFO A Offset output
uint8_t offset_b;          //amount of steps to offset LFO B to make LFO B Offset output

const int8_t lfo_a_pin = 11;
const int8_t lfo_a_offset_pin = 10;
const int8_t lfo_b_pin = 6;
const int8_t lfo_b_offset_pin = 5;

unsigned long time_A_previous = 0; //used to determine if LFO A should advance to next step
unsigned long time_B_previous = 0; //used to determine if LFO A should advance to next step

unsigned char pot_value_A;
unsigned char pot_value_B;

//Define Digial Button Pins
const int button1Pin = 13;
const int button2Pin = 12;
int button1 = 0;
int button2 = 0;

short LFO_A_rate;
short LFO_B_rate;
                
void setup() {  
  
  //for(int i=0; i<129; i++) {    // Precalculates a simple Tri-Wavetable in the first wavetable. 
  //  waveTable[0][i]=i*2;        // Delete these 2 'for loops' if you have filled up your wavetables 
  //  } 
  //for(int i=0; i<128; i++)  {   
  //  waveTable[0][i+128]=256-i*2; 
  //} 
  //waveTable[0][128]=255; //256 was overflowing to "0" for step 128 above
  
  pinMode(lfo_a_pin, OUTPUT);
  pinMode(lfo_a_offset_pin, OUTPUT);  
  pinMode(lfo_b_pin, OUTPUT);  
  pinMode(lfo_b_offset_pin, OUTPUT);      
  pinMode(13, OUTPUT);          // Pin 13 as LED for Tempo, you can also use this as a square-LFO 

  //Set button pins to use internal Pullup resistors
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);

  if(debug == 1){Serial.begin(9600);}
} 



void loop() { 

  
  if(tableStepA<128) {          // Turn LED on for first half of the cycle, indicate Tempo 
    digitalWrite(13, HIGH); 
  } 
    else {                       // Turn it off for the second half 
      digitalWrite (13, LOW); 
    } 
   
  //Get the current time
  unsigned long current_time = millis();

  //Set the offsets (testing only - will have pot set offsets eventually)
  offset_a = 127; //127 = 180 deg offset of 255 wavetable
  offset_b = 63; //127 = 180 deg offset of 255 wavetable

  
  //record & map pot values to control the rates of LFOs
  pot_value_A = analogRead(A0);
  LFO_A_rate = map(pot_value_A, 0, 1023, 1, 200); //1-100 microsecords
  pot_value_B = analogRead(A1);
  LFO_B_rate = map(pot_value_B, 0, 1023, 1, 200); //1-100 microsecords

  //Read button values:
  //NOTE: Button press == LOW
  int buttonRead1 = digitalRead(button1Pin);
  int buttonRead2 = digitalRead(button2Pin);
  
  //if shape buttons pressed, change shape (1,2,3)
  if (buttonRead1 == LOW) {
     button1 = button1 + 1;
     if(button1==3)
        button1 = 0;
  }

  if (buttonRead2 == LOW) {
     button2 = button2 + 1;
     if(button2==3)
        button2 = 0;
  }
  
  shapeA = 0; //(analogRead(A2) >> 8);   // Reads the voltage at pin A2 and divides by 256 to get a Value between 0 and 3 to select the waveshape. 
  shapeB = 0; //(analogRead(A3) >> 8);   // Reads the voltage at pin A3 and divides by 256 to get a Value between 0 and 3 to select the waveshape. 

  //LFO A
  if((current_time - time_A_previous) > LFO_A_rate){                    //once a certain amount of time passes, advance the table step and write the value
    tableStepA_offset = tableStepA + offset_a;                          //set LFO A Offset to be at 180 deg phase (half of table length)
    analogWrite(lfo_a_pin, waveTable_255[shapeA][tableStepA]);          //writes the value at the current step to Pin A (PWM) 
    analogWrite(lfo_a_offset_pin, waveTable_255[shapeA][tableStepA_offset]);  //writes at 180 deg phase of A to Pin A Inverse
    tableStepA++;                                                               // Jumps to the next step for LFO A 
    
    time_A_previous = current_time; 

    if(debug == 1){
      //Serial.print("LFO A Rate = ");
      //Serial.print(LFO_A_rate);
      //Serial.print("    Table Step = ");
      //Serial.print(tableStepA);
      //Serial.print("    LFO Value = ");
      //Serial.print(waveTable_255[shapeA][tableStepA]);
      } 
    }

  //LFO B
  if((current_time - time_B_previous) > LFO_B_rate){ //once a certain amount of time passes, advance the table step and write the value
    tableStepB_offset = tableStepB + offset_b;  
    analogWrite(lfo_b_pin, waveTable_255[shapeB][tableStepB]);              // Writes the value at the current step in the table to Pin 5 as PWM-Signal.  
    analogWrite(lfo_b_offset_pin, waveTable_255[shapeB][tableStepB_offset]);       
    tableStepB++;  
    
    time_B_previous = current_time;

    if(debug == 1){
      Serial.print("       LFO B Rate = ");
      Serial.print(LFO_B_rate);
      Serial.print("    Table Step = ");
      Serial.print(tableStepB);
      Serial.print("    LFO Value = ");
      Serial.println(waveTable_255[shapeB][tableStepB]);
      //Serial.println(" ");
      } 
    }


}
