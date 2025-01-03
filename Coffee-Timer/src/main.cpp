// https://claude.ai/chat/f30de9e9-114e-40b3-a81f-32cd40f9b1ac

#include <Arduino.h>
#include <TM1637Display.h>

#define CLK 2
#define DIO 3
#define LOAD 4
TM1637Display display(CLK, DIO);

#define ENCODER_PIN_A 5
#define ENCODER_PIN_B 6
#define ENCODER_SWITCH 7
#define RELAY_PIN 8

const unsigned long LONG_PRESS_DURATION = 1000; // 1 Sekunde für langen Druck

int currentCount = 0;
bool counting = false;
unsigned long previousMillis = 0;
const int interval = 1000;

int lastEncoderA = HIGH;
int lastSwitchState = HIGH;
unsigned long buttonPressTime = 0;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Sicherstellen, dass Relais initial AUS ist
  
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(ENCODER_SWITCH, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();

  Serial.begin(9600);
  Serial.println("Timer Ready. Waiting for input...");
}

void loop() {
  int currentA = digitalRead(ENCODER_PIN_A);
  int currentSwitchState = digitalRead(ENCODER_SWITCH);
  
  if (!counting) {
    // Encoder rotation handling
    if (currentA != lastEncoderA) {
      delay(5); // Small delay for stability
      
      if (currentA == LOW) {
        if (digitalRead(ENCODER_PIN_B) == HIGH) {
          // Clockwise
          currentCount = (currentCount + 1) % 100;
          Serial.print("Encoder turned clockwise. Current count: ");
          Serial.println(currentCount);
        } else {
          // Counter-clockwise
          currentCount = (currentCount - 1 + 100) % 100;
          Serial.print("Encoder turned counter-clockwise. Current count: ");
          Serial.println(currentCount);
        }
        display.showNumberDecEx(currentCount, 0b01000000, true);
      }
      
      lastEncoderA = currentA;
    }

    // Button press handling with long press detection
    if (currentSwitchState == LOW) {
      if (lastSwitchState == HIGH) {
        // Button just pressed
        buttonPressTime = millis();
      }
      
      // Check for long press
      if (millis() - buttonPressTime >= LONG_PRESS_DURATION) {
        // Long press detected - reset to zero
        currentCount = 0;
        display.showNumberDecEx(currentCount, 0b01000000, true);
        Serial.println("Long press detected. Timer reset to zero.");
        
        // Wait for button release to prevent multiple resets
        while(digitalRead(ENCODER_SWITCH) == LOW) {
          delay(10);
        }
      }
    } else if (lastSwitchState == LOW && currentSwitchState == HIGH) {
      // Short press to start countdown
      if (millis() - buttonPressTime < LONG_PRESS_DURATION) {
        Serial.println("Encoder button pressed. Countdown started.");
        counting = true;
        digitalWrite(RELAY_PIN, LOW); // Relais EIN
        previousMillis = millis();
      }
    }
    
    lastSwitchState = currentSwitchState;
  } else {
    // Countdown logic
    if (millis() - previousMillis >= interval) {
      previousMillis = millis();
      currentCount--;
      
      Serial.print("Counting down. Current value: ");
      Serial.println(currentCount);
      
      display.showNumberDecEx(currentCount, 0b01000000, true);
      
      if (currentCount <= 0) {
        Serial.println("Countdown finished. Relay off.");
        counting = false;
        digitalWrite(RELAY_PIN, HIGH); // Relais AUS
        display.clear();
        currentCount = 0;
      }
    }

    // Option to stop countdown with button press
    if (currentSwitchState == LOW) {
      if (millis() - buttonPressTime >= LONG_PRESS_DURATION) {
        Serial.println("Countdown stopped.");
        counting = false;
        digitalWrite(RELAY_PIN, HIGH); // Relais AUS
        display.clear();
        currentCount = 0;
        
        // Wait for button release
        while(digitalRead(ENCODER_SWITCH) == LOW) {
          delay(10);
        }
      }
    }
    
    lastSwitchState = currentSwitchState;
  }
}