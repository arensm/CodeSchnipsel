#include <Arduino.h>
#include <TM1637Display.h>

// Pin definitions for the 7-segment displays
#define CLK 2         // Clock pin (SCLK)
#define DIO 3         // Data pin (SDI)
#define LOAD 4        // Load pin (LOAD)
TM1637Display display(CLK, DIO);

// Pin definitions for the rotary encoder
#define ENCODER_PIN_A 5 // Encoder output A
#define ENCODER_PIN_B 6 // Encoder output B
#define ENCODER_SWITCH 7 // Encoder switch (button)

// Pin for the relay module
#define RELAY_PIN 8

// Variables for the encoder
int currentCount = 0; // Stores the current countdown value
bool counting = false; // Indicates if the countdown is active
unsigned long previousMillis = 0; // For timing the countdown
const int interval = 1000; // 1 second in milliseconds

void setup() {
  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Ensure the relay is off
  
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(ENCODER_SWITCH, INPUT_PULLUP);

  // Initialize the display
  display.setBrightness(7); // Set brightness (0-7)
  display.clear();          // Ensure the display is blank initially

  // Initialize Serial Communication
  Serial.begin(9600);
  Serial.println("Timer Ready. Waiting for input...");
}

void loop() {
  static int lastA = HIGH; // Previous state of ENCODER_PIN_A
  static bool lastSwitchState = HIGH; // Previous state of ENCODER_SWITCH

  if (!counting) {
    // Handle encoder for adjusting seconds
    int currentA = digitalRead(ENCODER_PIN_A);
    if (currentA == LOW && lastA == HIGH) {
      if (digitalRead(ENCODER_PIN_B) == HIGH) {
        // Clockwise turn
        currentCount = (currentCount + 1) % 100; // Increment seconds, wrap at 99
        Serial.print("Encoder turned clockwise. Current count: ");
        Serial.println(currentCount);
      } else {
        // Counter-clockwise turn
        currentCount = (currentCount - 1 + 100) % 100; // Decrement seconds, wrap at 0
        Serial.print("Encoder turned counter-clockwise. Current count: ");
        Serial.println(currentCount);
      }
      display.showNumberDecEx(currentCount, 0b01000000, true); // Update display
    }
    lastA = currentA;

    // Handle button press for starting countdown
    bool currentSwitchState = digitalRead(ENCODER_SWITCH);
    if (lastSwitchState == HIGH && currentSwitchState == LOW) {
      // Button pressed
      Serial.println("Encoder button pressed. Countdown started.");
      counting = true;
      digitalWrite(RELAY_PIN, HIGH); // Turn on the relay
      previousMillis = millis(); // Reset the countdown timer
    }
    lastSwitchState = currentSwitchState;
  } else {
    // Handle countdown
    if (millis() - previousMillis >= interval) {
      previousMillis = millis();
      currentCount--; // Decrement the counter
      Serial.print("Counting down. Current value: ");
      Serial.println(currentCount);
      display.showNumberDecEx(currentCount, 0b01000000, true); // Update display
      if (currentCount <= 0) {
        // Countdown finished
        Serial.println("Countdown finished. Relay off.");
        counting = false;
        digitalWrite(RELAY_PIN, LOW); // Turn off the relay
        display.clear(); // Clear the display
        currentCount = 0; // Reset count to 0
      }
    }
  }
}
/*
Circuit Diagram
Below is a description of how the components should be wired:

7-Segment Display (with TM1637 driver):

VCC → Arduino 5V
GND → Arduino GND
SDI (DIO) → Arduino Digital Pin 3
SCLK (CLK) → Arduino Digital Pin 2
LOAD (optional) → Not used in TM1637 but could be connected for multiplexing.

Rotary Encoder:

OUT A → Arduino Digital Pin 5
OUT B → Arduino Digital Pin 6
SWITCH → Arduino Digital Pin 7
GND → Arduino GND

////////////////////////////////////////
Relay Module:

VCC → Arduino 5V
GND → Arduino GND
DIN → Arduino Digital Pin 8
Power Supply:

All VCC pins are connected to Arduino 5V.
All GND pins are connected to Arduino GND.
*/