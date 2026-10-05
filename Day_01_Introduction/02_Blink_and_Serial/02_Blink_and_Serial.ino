// MISHEARD CITY | Practical 1 | 02 - Blink with variables and serial feedback
// Board: Seeed Studio XIAO ESP32S3 Sense.
// Arduino IDE: USB CDC On Boot = Enabled; Serial Monitor = 115200.
// Serial Monitor output is live text, not a saved data file.

const int ledPin = LED_BUILTIN;
const char groupName[] = "GROUP_01";
int onTimeMs = 300;
int offTimeMs = 700;
unsigned long cycleCount = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  Serial.begin(115200);
  delay(1000);  // Brief startup pause; does not wait forever for a computer.
}

void loop() {
  cycleCount = cycleCount + 1;
  Serial.print("group=");
  Serial.print(groupName);
  Serial.print(", cycle=");
  Serial.print(cycleCount);
  Serial.print(", on_ms=");
  Serial.print(onTimeMs);
  Serial.print(", off_ms=");
  Serial.println(offTimeMs);

  digitalWrite(ledPin, LOW);
  delay(onTimeMs);
  digitalWrite(ledPin, HIGH);
  delay(offTimeMs);
}
