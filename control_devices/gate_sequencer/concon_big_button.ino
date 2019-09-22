// TO DO:
// - it'd be nice if you could see the beginning of the 'loop' with an LED flash - Beginning of Cycle
// - would be able to sync up w/ starting of other loops using the 'reset' button 
// - remove 'shift' 
// - quantize record button hits? Or figure out how to make record button hits better (hitting a bit late doesn't work now)
// - add external input to 'fill' button

// Available pins: 0, 1, 3, 7, 12, (A2), 17(A3), 18(A4), 


// Clock Input ... Pin 2 (available for "Interupts")

// Clear Button .. Pin 4 (clears the entire loop sequence for the current channel and bank)
// Bank Button ... Pin 3 (each channel has 2 banks where alternative patterns can be recored and stored. this button toggles the bank for the current channeL)
// Big Button .... Pin 19 (A5) (this is the record button!)
// Reset Button .. Pin 7 (resets the current channel and back to step 1)
// Fill Button ... Pin 5 (while held, it will continuously play the channel your on)

// Channel Select Switch .. Pin (A0) (selects the current channel, 1-6, to modify)
// Step Length Knob ....... Pin (A1) (sets the amount of steps for all channels, 1-32)

// Output 1 ..... Pin 8
// Output 2 ..... Pin 9
// Output 3 ..... Pin 10
// Output 4 ..... Pin 11
// Output BOC ... Pin 6 (PWM)

const byte    debug = 0;           // if 1, run the Serial Monitor and debug program

unsigned long time = 0;            
const byte    debounce = 200;

// CLOCK IN 
// Declared as volatile as it is shared between the ISR and the main program.
const byte    clkInPin = 2;
volatile byte clkInState;       

// BIG BUTTON aka Record Button
const byte recordButtonPin = 19;
byte       recordButtonState = 0;       

// RESET BUTTON
const byte resetButtonPin = 7; 
byte       resetButtonState = 0;  

// FILL BUTTON
const byte fillButtonPin = 5;
byte       fillButtonState = 0;
byte       fillState[6] = {0,0,0,0,0,0};   // store whether the fill is on or off for each channel

// CLEAR BUTTON
const byte clearButtonPin = 4; 
byte       clearButtonState = 0;

// BANK BUTTON
const byte bankButtonPin = 3;
byte       bankButtonState = 0;            
byte       bankState[6] = {0,0,0,0,0,0};   // stores whether the bank is off or on (0 or 6) for each channel (which then selects the pattern sequence). 

// CLOCK STUFF
byte       currentStep  = 0;    // sets the shared global 'current step', used in the 'delete' function. Previously 'looper'

// PATERN LENGTH STUFF
const byte patLengthPin = A1;   
int        patLengthPotRead = 0; 
byte       steps = 0;             // length of sequence (1 - 32 steps)

// CHANNEL SELECT STUFF
const byte channelPotReadPin = A0; 
int        channelPotRead = 0;
byte       currentChannel = 0;    // current channel (0-5)
byte       lastChannel = 0;          
          
// OUTPUTS
const byte outPin1 = 8;           // pins for trigger and LED outputs
const byte outPin2 = 9;
const byte outPin3 = 10;
const byte outPin4 = 11;
const byte outPinBoc  = 6;
const byte bankLEDPin = 13;               // pin TBD  for bank LED 

// SEQUENCE ARRAY
/*  First dimension is the Channel / Bank combo (6 channels, each with 2 banks = 12 sequences) 
 *  Second dimension is if the beat is on or off within that sequence. Each sequence can have up to 32 beats.
 *  Sequence[0]:  Channel 1, Bank Off
 *  Sequence[1]:  Channel 2, Bank Off  
 *  Sequence[2]:  Channel 3, Bank Off 
 *  Sequence[3]:  Channel 4, Bank Off  
 *  Sequence[4]:  Channel 1, Bank On  //Note: bank ON is "+ 4" as 'current channel' (0-3) is used to determine sequence number
 *  Sequence[5]:  Channel 2, Bank On  
 *  Sequence[6]:  Channel 3, Bank On  
 *  Sequence[7]:  Channel 4, Bank On

 */
byte Sequence[9][32] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {255,0,20,0,20,0,20,0,20,0,20,0,20,0,20,0,20,0,20,0,20,0,20,0,20,0,20,0,20,0,20,0,}, //BOC Timer LED
 };
 
// FUN BLINKY ON STARTUP
void startupLEDSequence(){
  digitalWrite(bankLEDPin , HIGH); //this could go in a function, be redesigned for channel LEDs
  delay(200);
  digitalWrite(bankLEDPin , LOW);
  delay(180);
  digitalWrite(bankLEDPin , HIGH);
  delay(160);
  digitalWrite(bankLEDPin , LOW);
  delay(140);
  digitalWrite(bankLEDPin , HIGH);
  delay(120);
  digitalWrite(bankLEDPin , LOW);
  delay(100);
  digitalWrite(bankLEDPin , HIGH);
  delay(80);
  digitalWrite(bankLEDPin , LOW);
  delay(60);
  digitalWrite(bankLEDPin , HIGH);
  delay(40);
  digitalWrite(bankLEDPin , LOW);
  delay(20);
  digitalWrite(bankLEDPin , HIGH);
  delay(60);
  digitalWrite(bankLEDPin , LOW);
}

//  isr() - or Interupt Service Routine - quickly handle interrupts from the clock input
//  See "attachInterrupt" in Setup
//  If clock input is triggered (rising edge), set the reading of clock in to be HIGH
void isr()
{
  clkInState = HIGH;
}

void debuger(){
  if (debug == 1){
  //Serial.print("Clock In = ");
  //Serial.println(clkInState);
  
  
   //Serial.print("Big Button = ");
   //Serial.print(recordButtonState);
   //Serial.print("  Bank Button = ");
   //Serial.print(ButtonBankSelectState[BankArrayNumber]);
   //Serial.print("  Clear Button = ");
   //Serial.print(clearButtonState);
   //Serial.print("  Fill Button = ");
   //Serial.print(fillButtonState);
   //Serial.print("  Reset Button = ");
   //Serial.print(resetButtonState);
   

  // POT TESTING
  Serial.print("Channel Pot = ");
  Serial.print(channelPotRead);
  Serial.print("  Current Channel = ");
  Serial.print(currentChannel); 
  Serial.print("  Step Pot = ");
  Serial.print(patLengthPotRead); 
  Serial.print("  Steps = ");
  Serial.println(steps);             // Should be between 1 - 32
  } 
}


void setup() { 
  pinMode(outPin1,OUTPUT);
  pinMode(outPin2,OUTPUT);
  pinMode(outPin3,OUTPUT);
  pinMode(outPin4,OUTPUT);
  //pinMode(outPinBoc,OUTPUT); // not needed for PWM out
  pinMode(bankLEDPin , OUTPUT);
  pinMode(clkInPin, INPUT);              // Clock input is high (use hardware pulldown)
  pinMode(recordButtonPin,INPUT_PULLUP);
  pinMode(clearButtonPin,INPUT_PULLUP);
  pinMode(bankButtonPin, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP); // if you want to use an external reset signal, would need to use a hardward pulldown
  pinMode(fillButtonPin, INPUT_PULLUP);
  pinMode(patLengthPin, INPUT);          

  // read pattern length pot value on start up
  // PUT THIS IN A FUNCTION
  patLengthPotRead = analogRead(patLengthPin);
  if(0<patLengthPotRead){steps= 1;}    // are step lengths 1,2,4 really necessary?
  if(150<patLengthPotRead){steps= 2;}
  if(300<patLengthPotRead){steps= 4;}
  if(500<patLengthPotRead){steps= 8;}
  if(750<patLengthPotRead){steps=16;}
  if(1000<patLengthPotRead){steps=32;}    

  startupLEDSequence();  // LED flashing start up sequence

  // Interupt Service Routine for Clock Input
  // Syntax: attachInterrupt(digitalPinToInterrupt(pin), ISR, mode)
  // ISR: the interupt service routine to call when the interupt occurs (when clock pin goes high)
  // Rising: trigger when the pin goes from low to high
  attachInterrupt(0, isr, RISING);

  // if debug mode is on, run the serial monitor
  if(debug == 1){Serial.begin(9600);}
  
} //END SETUP


void loop() {

  // Get button readings
  recordButtonState = digitalRead(recordButtonPin);
  clearButtonState = digitalRead(clearButtonPin); 
  resetButtonState = digitalRead(resetButtonPin);
  fillButtonState = digitalRead(fillButtonPin);
  bankButtonState = digitalRead(bankButtonPin);
  
  // OUTPUT - play the each channels recorded sequence
  if(clkInState == HIGH) {
    // Output each channels sequence
    // when I moved out of if statement, it created longer duty cycles (fill would run into itself for each beat, leading to constant ON state)
    digitalWrite(outPin1,Sequence[0 + bankState[0]] [currentStep] || fillState[0]); // Logical OR results in a true if either of the two operands is true.
    digitalWrite(outPin2,Sequence[1 + bankState[1]] [currentStep] || fillState[1]); 
    digitalWrite(outPin3,Sequence[2 + bankState[2]] [currentStep] || fillState[2]); 
    digitalWrite(outPin4,Sequence[3 + bankState[3]] [currentStep] || fillState[3]);
    analogWrite (outPinBoc,Sequence[8][currentStep]);
    delay(10);              //do this with a time diff?
    digitalWrite(outPin1,LOW);
    digitalWrite(outPin2,LOW);
    digitalWrite(outPin3,LOW);
    digitalWrite(outPin4,LOW);
    analogWrite (outPinBoc, 0);

    clkInState = LOW;      // isr triggers on rising edge of clock signal. isr sets clkInState to be HIGH, we so need to set to LOW here. 
    currentStep  = (currentStep+1);  
  } 
    
  
  // RECORD BUTTON - record the sequence of the current pattern
  if(recordButtonState == LOW && millis() - time > debounce) {
    Sequence[currentChannel + bankState[currentChannel]] [currentStep] = 1;  //changed from currentStep + 1
    /* old way:
    if(currentChannel == 0) {Sequence[currentChannel + bankState[currentChannel]] [currentStep + 1] = 1;}
    else if(currentChannel == 1) {Sequence[currentChannel + bankState[currentChannel]] [currentStep + 1] = 1;} 
    else if(currentChannel == 2) {Sequence[currentChannel + bankState[currentChannel]] [currentStep + 1] = 1;}
    else if(currentChannel == 3) {Sequence[currentChannel + bankState[currentChannel]] [currentStep + 1] = 1;}
    */                                                                                
    time = millis();    
  }

  // Turn on Bank LED if bank is on for the current channel
  digitalWrite(bankLEDPin, bankState[currentChannel]);

 
  // BANK BUTTON
  // If the bank button is pressed set the bank state for the current channel to 6 if 0 and vice versa
  // Bank is 0 or 6 to add 6 to get the correct pattern (current channel + 6)
  if(bankButtonState == LOW && millis() - time > debounce){
     if(bankState[currentChannel] == 4){
        bankState[currentChannel] = 0;
        }
     else {
       bankState[currentChannel] = 4;
       }
     time = millis();
  }                                                                                               

  // CHANNEL SELECT POT (0-1023)
  channelPotRead= analogRead(channelPotReadPin);
  if(channelPotRead < 255)  {currentChannel = 0;}
  else if(channelPotRead < 510)  {currentChannel = 1;}
  else if(channelPotRead < 765)  {currentChannel = 2;}
  else if(channelPotRead >= 765) {currentChannel = 3;}



  // CLEAR PATTERN BUTTON
  // If the clear button is pressed, remove all triggers from the current pattern
  if(clearButtonState == LOW){ 
    for(int i = 1; i<32; i++){
    Sequence[currentChannel + bankState[currentChannel]][i] = 0;
    }
  } 
  
  // FILL BUTTON
  // If the fill button pressed, the current channel should go in to fill mode
  // Fill can only be on one channel at a time
  if(fillButtonState == LOW){
    fillState[currentChannel] = 1;
    if(currentChannel != lastChannel){
        fillState[lastChannel] = 0;
        }
      } 
  else {fillState[currentChannel] = 0;}
                           

  // RESET BUTTON
  // If the reset button is pressed, set the current steps to 0 (start from step 1 of the patterns)
  if(resetButtonState == LOW && millis() - time > debounce) {
    currentStep = 0;  
    time = millis();   
   } 

  // Determine how many steps the looping pattern is
  patLengthPotRead = analogRead(patLengthPin);
  if(patLengthPotRead < 150){steps= 1;}
  else if(patLengthPotRead < 300){steps= 2;}
  else if(patLengthPotRead < 500){steps= 4;}
  else if(patLengthPotRead < 750){steps= 8;}
  else if(patLengthPotRead < 1000){steps=16;}
  else if(patLengthPotRead >= 1000){steps=32;}                                    

  //this bit starts the sequence over again
  if(currentStep >= steps) {currentStep = 0;}   

  lastChannel = currentChannel;              //update the lastChannel, used for the Fill button  
  
  debuger();
  
} // END LOOP
