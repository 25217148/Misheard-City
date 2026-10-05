// MISHEARD CITY | Practical 1 | 01 - Blink
// Board: Seeed Studio XIAO ESP32S3 Sense.
// The built-in user LED is active-low: LOW = on, HIGH = off.

const int ledPin = LED_BUILTIN;
int onTimeMs = 300;
int offTimeMs = 700;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);  // Start with the user LED off.
}

void loop() {
  digitalWrite(ledPin, LOW);   // Turn the user LED on.
  delay(onTimeMs);
  digitalWrite(ledPin, HIGH);  // Turn the user LED off.
  delay(offTimeMs);
}
