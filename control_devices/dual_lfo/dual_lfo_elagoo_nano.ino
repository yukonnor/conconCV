/* 
 * BUTTON1 Seems to be stuck 'down'
 * 
 */ 
#include "wavetables.h"

const bool debug = 1;

// Declaring some Variables. All are unsigned, since you don't really need negative going values for now.
unsigned int delayTime;      

unsigned int shapeA;
unsigned int shapeB; 

uint8_t output_A;
uint8_t output_A_offset;
uint8_t output_B;
uint8_t output_B_offset;

uint8_t tableStepA;        //8-Bit unsigned integer, so it can only store a value between 0 and 255
uint8_t tableStepA_offset; //8-Bit unsigned integer, so it can only store a value between 0 and 255
uint8_t tableStepB;        //8-Bit unsigned integer, so it can only store a value between 0 and 255
uint8_t tableStepB_offset; //8-Bit unsigned integer, so it can only store a value between 0 and 255
uint8_t offset_a;          //amount of steps to offset LFO A to make LFO A Offset output
uint8_t offset_b;          //amount of steps to offset LFO B to make LFO B Offset output

// Define output pins
const int8_t lfo_a_pin = 11;
const int8_t lfo_a_offset_pin = 10;
const int8_t lfo_b_pin = 6;
const int8_t lfo_b_offset_pin = 5;

unsigned long time_A_previous = 0; //used to determine if LFO A should advance to next step
unsigned long time_B_previous = 0; //used to determine if LFO A should advance to next step

unsigned int pot_1; // A:  0-1023
unsigned int pot_2; // A': 0-1023
unsigned int pot_3; // B:
unsigned int pot_4; // B':

//Define Digial Button Pins
const int button1Pin = 12;
const int button2Pin = A4;

short LFO_A_rate;
short LFO_B_rate;

short drunk_sign_a = 0;
short drunk_change_a = 5;
short drunk_value_a = 127;
short drunk_sign_b = 0;
short drunk_change_b = 5;
short drunk_value_b = 127;

const short num_wavetables = 2; // Number of wavetables to cycle through
const short num_shapes = 4;     // Total number of shapes (including non-wavetables)

unsigned long current_time;
const int debounce = 200;   //20 for debug - 200 for production
unsigned long last_debounce_time;
                

// Cycle through shapes when button 1 is pressed for LFO A
int shape_A_switch(bool buttonRead1){
  
      if (buttonRead1 == LOW &&  millis() - last_debounce_time > debounce) {
         shapeA = shapeA + 1;
         if(shapeA == num_shapes){shapeA = 0;}      // Once at the last shape, start back at the beginning
      last_debounce_time = millis(); 
      }
      return shapeA;
    }

// Cycle through shapes when button 2 is pressed for LFO B
int shape_B_switch(bool buttonRead2){
  
      if (buttonRead2 == LOW &&  millis() - last_debounce_time > debounce) {
         shapeB = shapeB + 1;
         if(shapeB == num_shapes){shapeB = 0;}      // Once at the last shape, start back at the beginning
      last_debounce_time = millis(); 
      }
      return shapeB;
    }

void setup() {  
  
  pinMode(lfo_a_pin, OUTPUT);
  pinMode(lfo_a_offset_pin, OUTPUT);  
  pinMode(lfo_b_pin, OUTPUT);  
  pinMode(lfo_b_offset_pin, OUTPUT);      

  // Set button pins to use internal Pullup resistors
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);

  // Set initial output shapes
  shapeA = 0;
  shapeB = 0;

  if(debug == 1){Serial.begin(9600);}
} 


void loop() { 
  
  // Get the current time
  current_time = millis();

  // Record & map pot values to control the rates and offset amount of LFOs
  pot_1 = analogRead(A0);
    LFO_A_rate = map(pot_1, 0, 1023, 1, 150); // 1-100 microsecords
  pot_2= analogRead(A1);
    offset_a = map(pot_2, 0, 1023, 0, 200);   // 0-200 step offset. 127 = 180 deg offset of 255 wavetable 
  pot_3 = analogRead(A2);
    LFO_B_rate = map(pot_3, 0, 1023, 1, 150); // 1-100 microsecords
  pot_4= analogRead(A3);
    offset_b = map(pot_4, 0, 1023, 0, 200);   // 0-200 step offset. 
  
  //Read button values: (Button press = LOW)
  bool buttonRead1 = digitalRead(button1Pin); 
  bool buttonRead2 = digitalRead(button2Pin);
  
  // if shape button 1 pressed, cycle through shapes for LFO A
  shapeA = shape_A_switch(buttonRead1);
  
  // if shape button 2 pressed, cycle through wavetable shapes for LFO B
  shapeB = shape_B_switch(buttonRead2);
  
  //LFO A
  if((current_time - time_A_previous) > LFO_A_rate){  //once a certain amount of time passes, advance the table step and write the value

    // If shapeA is 0 or 1, cycle through WAVETABLES
    if(shapeA < num_wavetables){                                   
      
      tableStepA_offset = tableStepA + offset_a;                   //set LFO A Offset to be at 180 deg phase (half of table length)
  
      output_A = waveTable_255[shapeA][tableStepA];                // Sets the value of LFO A based on the current wavetable
      output_A_offset = waveTable_255[shapeA][tableStepA_offset];  // Sets that value of LFO A' based on an offset of current wavetable
      tableStepA++;                                                // Go to the next step for LFO A
      
    }
    
    // If shapeA is 2, do DRUNK
    if(shapeA == num_wavetables){
      
      drunk_sign_a = random(0,2);              //Selects either 0 or 1 to add or subtract
      if(drunk_sign_a == 1){
         drunk_change_a = -1*drunk_change_a;
         }   
  
      drunk_value_a = constrain(drunk_value_a + drunk_change_a,0,255);
      
      output_A = drunk_value_a;
      output_A_offset = output_A + offset_a;

    }

    //if shapeA is 3, do RANDOM
    if(shapeA == num_wavetables + 1){
      
      output_A = random(0,255);
      output_A_offset = output_A + offset_a;
    
    }

    //write the values for LFO A
    analogWrite(lfo_a_pin, output_A);          
    analogWrite(lfo_a_offset_pin, output_A_offset);  
    time_A_previous = current_time; 
      
  } // END LFO A
  
  //LFO B
  if((current_time - time_B_previous) > LFO_B_rate){  //once a certain amount of time passes, advance the table step and write the value

    // If shapeB is 0 or 1, cycle through WAVETABLES
    if(shapeB < num_wavetables){
      
      tableStepB_offset = tableStepB + offset_b;                   //set LFO B Offset to be at 180 deg phase (half of table length)
  
      output_B = waveTable_255[shapeB][tableStepB];                // Sets the value of LFO B based on the current wavetable
      output_B_offset = waveTable_255[shapeB][tableStepB_offset];  // Sets that value of LFO B' based on an offset of current wavetable
      tableStepB++;                                                // Go to the next step for LFO B
      
    }
    
    // If shapeB is 2, do DRUNK
    if(shapeB == 2){
      
      drunk_sign_b = random(0,2);            //Selects either 0 or 1 to add or subtract
      if(drunk_sign_b == 1){
         drunk_change_b = -1*drunk_change_b;
         }   
  
      drunk_value_b = constrain(drunk_value_b + drunk_change_b,0,255);
      
      output_B = drunk_value_b;
      output_B_offset = output_B + offset_b;

    }

    //if shapeB is 3, do RANDOM
    if(shapeB == 3){
      
      output_B = random(0,255);
      output_B_offset = output_B + offset_b;
    
    }

    //write the values for LFO B
    analogWrite(lfo_b_pin, output_B);          
    analogWrite(lfo_b_offset_pin, output_B_offset);  
    time_B_previous = current_time; 
      
  } // END LFO B
    
} // END LOOP

/*
if(debug == 1){
  Serial.print("LFO B Pot = ");
  Serial.print(pot_3);
  Serial.print("    B' Pot = ");
  Serial.print(pot_4);
  Serial.print("    LFO Value = ");
  Serial.println(waveTable_255[shapeB][tableStepB]);
  //Serial.println(" ");
  } 
 

if(debug == 1){
  Serial.print("drunk_sign_a = ");
  Serial.print(drunk_sign_a);
  Serial.print("    drunk_change_a= ");
  Serial.print(drunk_change_a);
  Serial.print("    drunk_value_a = ");
  Serial.println(drunk_value_a);
  } 
 */
