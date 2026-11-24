#include <Arduino.h>

//----------Pin numbers----------
#define BUTTON_HELP 33


//----------Global variables----------
uint8_t statusByte = 0b00000000;

// These two variables are changed inside the ISR,...
// ...so they need the keyword for a shared ISR variable.
// buttonBFlag: raised (true) by the ISR when button B is pressed.
volatile bool buttonBFlag = false;

// lastInterruptB: the time of the last accepted press (used to debounce).
volatile unsigned long lastInterruptB = 0;

// pressCountB is only used in loop(), so it needs no keyword.
int pressCountB = 0;

// A Callback is a variable that holds a function (no inputs, returns nothing).
typedef void (*Callback)();
Callback buttonBCallback;   // what button B does


//----------Event handlers and callbacks----------
// Event handler for button B – called from loop(), never from the ISR,
// so Serial.print() is allowed here.
void onButtonBPressed()
{
  pressCountB++;                                         // add 1 to the count
  Serial.print("Help requested (interrupt). Count = ");  // print the text...
  Serial.println(pressCountB);                           // ...then the number
  statusByte = 0b00100000;                               // Bit 2
  Serial.print("Status Byte = ");
  Serial.println(statusByte, BIN);
}


// ---------- Interrupt Service Routines (ISRs) ----------
// ISR for button B – runs the instant the pin falls, even if loop() is busy.
// Blank before the name: the keyword that keeps an ISR in fast memory
// (Common Commands: "ISR Function"). Keep an ISR short: no Serial, no delay().
void IRAM_ATTR buttonB_ISR()
{
  // Ignore presses less than 200 ms apart (debounce inside the ISR).
  if (millis() - lastInterruptB > 200)
  {
    lastInterruptB = millis();  // remember when this press happened
    buttonBFlag = true;         // raise the flag for loop(): true or false?
  } 
} 


void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("PBL: Smart Study Room by Malick and Kyle");

  //---------Inputs and outputs----------
  pinMode(BUTTON_HELP, INPUT_PULLUP);
  // pinMode (,);

  //----------Interrupts----------
  attachInterrupt(digitalPinToInterrupt(BUTTON_HELP), buttonB_ISR, FALLING);

  // Callbacks
  // Under // Callbacks: choose which function each event calls.
  // No brackets here – you store the function, you do not run it.
  buttonBCallback = onButtonBPressed;    // button B must toggle HELP.
}


void loop() {
  // Button B (interrupt)
  // Flag pattern: did the ISR raise the flag?
  if (buttonBFlag == true)
  {
    buttonBFlag = false;   // lower the flag first, so the press is handled once
    buttonBCallback();    // then call the handler
  }
}