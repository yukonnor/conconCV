/* 
 * Version 0.6
 * 
 * Pin A0 = Speed 
 * Pin A1 = Shape 
 * 
 * Pin 5 = PWM Out 
 * Pin 13 = LED / Square Out 
 * 
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

unsigned char pot_1;
unsigned char pot_2;
unsigned char pot_3;
unsigned char pot_4;

//Define Digial Button Pins
const int button1Pin = 13;
const int button2Pin = 12;

short LFO_A_rate;
short LFO_B_rate;

const short num_wavetables = 4; //number of wavetables to cycle through

unsigned long current_time;
const int debounce = 200;   //20 for debug - 200 for production
unsigned long last_debounce_time;
                
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

  //Set initial wavetable shapes
  shapeA = 0;
  shapeB = 0;

  if(debug == 1){Serial.begin(9600);}
} 



void loop() { 
  
  //Get the current time
  current_time = millis();

  //record & map pot values to control the rates of LFOs
  pot_1 = analogRead(A0);
    LFO_A_rate = map(pot_1, 0, 1023, 1, 200); // 1-100 microsecords
  pot_2= analogRead(A1);
    LFO_B_rate = map(pot_2, 0, 1023, 1, 200); // 1-100 microsecords
  pot_3 = analogRead(A2);
    offset_a = map(pot_3, 0, 1023, 0, 200);   // 0-200 step offset. 127 = 180 deg offset of 255 wavetable 
  pot_4= analogRead(A3);
    offset_b = map(pot_4, 0, 1023, 0, 200);   // 0-200 step offset. 

  // Set the offsets for testing:
  // offset_a = 127; //127 = 180 deg offset of 255 wavetable
  // offset_b = 63; //127 = 180 deg offset of 255 wavetable
  
  //Read button values: (NOTE: Button press == LOW)
  bool buttonRead1 = digitalRead(button1Pin);
  bool buttonRead2 = digitalRead(button2Pin);
  Serial.print("    Button 1 State = ");
  Serial.print(buttonRead1);
  
  // if shape button 1 pressed, cycle through wavetable shapes for LFO A
  if (buttonRead1 == LOW &&  millis() - last_debounce_time > debounce) {
      shapeA = shapeA + 1;
      if(shapeA==(num_wavetables)) //once at the last wavetable, start back at the beginning
        shapeA = 0;
      last_debounce_time = millis(); //check back on this.. may not need? 
   }
  
  // if shape button 2 pressed, cycle through wavetable shapes for LFO B
  if (buttonRead2 == LOW &&  millis() - last_debounce_time > debounce) {
      shapeB++;
      if(shapeB==(num_wavetables)) //once at the last wavetable, start back at the beginning
        shapeB = 0;
      last_debounce_time = millis(); //check back on this.. may not need? 
   }
  
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
      Serial.print("    Wavetable Shape A = ");
      Serial.print(shapeA);
      Serial.print("    LFO A Value = ");
      Serial.print(waveTable_255[shapeA][tableStepA]);
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
