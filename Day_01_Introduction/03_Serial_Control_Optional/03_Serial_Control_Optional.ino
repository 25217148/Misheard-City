// MISHEARD CITY | Practical 1 | 03 - Serial control (optional)
// Board: Seeed Studio XIAO ESP32S3 Sense.
// Arduino IDE: USB CDC On Boot = Enabled; Serial Monitor = 115200.
// Send 1 for on, 0 for off. Newline/carriage-return characters are ignored.

const int ledPin = LED_BUILTIN;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  Serial.begin(115200);
  delay(1000);
  Serial.println("Send 1 for LED on; send 0 for LED off.");
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == '1') {
      digitalWrite(ledPin, LOW);
      Serial.println("User LED: ON");
    } else if (command == '0') {
      digitalWrite(ledPin, HIGH);
      Serial.println("User LED: OFF");
    } else if (command != '\n' && command != '\r') {
      Serial.println("Unknown command. Send 1 or 0.");
    }
  }
}
