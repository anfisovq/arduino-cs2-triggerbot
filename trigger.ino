#include <Mouse.h>

const int ledPin = LED_BUILTIN;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Mouse.begin();
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == '1') {
      Mouse.click(MOUSE_LEFT); 
      blinkLED();
    }
  }
}

void blinkLED() {
  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin, LOW);
}
