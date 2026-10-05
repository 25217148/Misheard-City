![Arduino logo](images/Arduino_Logo.svg.png)

# Introduction to Arduino
## Misheard City: Day 1

**Tuesday 6 October · Kunpeng Lei**

[Workshop homepage](../README.md) · [Our board](#our-board) · [Installation](#installation) · [AI-assisted programming](#ai-assisted-programming) · [Camera practical](04_Camera_and_SD/README.md) · [Research task](05_Research/README.md)

**You need:** a laptop, Arduino IDE, a USB-C data cable, a XIAO ESP32S3 Sense, a prepared microSD card and a card reader.

---

## Opening tutorial: interests and observations

Show a photograph or describe a place you would like to investigate: one detail you notice there and one thing you would like to understand better.

## What is Arduino, and where does it come from?

**Arduino** is an open-source electronics platform for interactive projects. It combines **hardware** (the board and its components) and **software** (the code that runs on it).

Arduino grew out of interaction-design education in Ivrea, Italy. It made microcontrollers more accessible to designers, artists and students who wanted to experiment with physical objects and environments.

![The Arduino bar in Ivrea](images/BarArduinoReal.jpg)

*The Arduino bar in Ivrea, where the project got its name.*

![An early Arduino prototype](images/FirstArduino.jpg)

*An early Arduino prototype.*

Because the platform is open, you can learn from existing examples, adapt them and share the result.

[Arduino website](https://www.arduino.cc/) · [What is Arduino?](https://docs.arduino.cc/learn/starting-guide/whats-arduino/)

## What can Arduino do?

A programmable board can **sense** something, process the information and produce an **output**. For example:

- A light sensor changes the brightness of an LED.
- A temperature sensor produces a record over time.
- A motor responds to a button.
- A camera captures an image and saves it to a card.
- A trained model produces category scores from a camera image.

### Microcontrollers and computers

A **microcontroller** combines a processor, memory and connections to other components. A **development board** makes the chip easier to power, program and connect.

| Your laptop | Our XIAO board |
|---|---|
| Edit and compile a program | Run the uploaded program |
| Prepare datasets and work with training tools | Capture images and later run a small trained model |
| View images, analyse records and edit a film | Save images and measurements |
| Has a desktop interface | Communicates through USB, connected components or a network |

Uploading installs the program in the board's flash memory. Once programmed, the board can run from a USB power supply without the laptop.

## The Arduino language and ecosystem

We write **C++ using the Arduino framework**. Its functions make common tasks approachable: `digitalWrite()` changes a pin's electrical level; `Serial.println()` sends a line of text.

A program is called a **sketch**. We write it in the **Arduino IDE**, the application that compiles and uploads it.

| Name | What it means here |
|---|---|
| Arduino IDE | The application on your computer |
| Arduino framework | The functions and conventions used in the sketch |
| Board package | The tools and definitions needed to build for a particular board |
| Library | Reusable code for a camera, sensor or other task |
| Firmware | The program installed on the board |

Many manufacturers make Arduino-compatible boards, including Seeed Studio, Adafruit and SparkFun. An example written for an Uno often needs changes before it runs on a XIAO.

---

# Our Board

## Seeed Studio XIAO ESP32S3 Sense

![Seeed Studio XIAO ESP32S3 Sense](images/sense-product.jpg)

We use the **XIAO ESP32S3 Sense**, a small board with a camera, microphone and microSD interface. The **base board** contains the ESP32-S3 processor; the **Sense expansion board** provides the camera and recording interfaces.

[Seeed Studio: Getting Started](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)

### Core specifications

| Feature | Specification / use |
|---|---|
| Processor | ESP32-S3, dual-core, up to 240 MHz |
| Flash | 8 MB: stores the firmware and, later, the deployed model |
| PSRAM | 8 MB: additional working memory for images and other data |
| Base-board dimensions | Approximately 21 × 17.8 mm |
| USB-C | Power, programming and serial communication |
| GPIO logic | **3.3 V** |
| Connectivity | Wi-Fi and Bluetooth LE |
| Sense expansion | Camera, digital microphone and microSD socket |

The camera module can differ between production versions.

### Pins, power and buttons

![XIAO ESP32S3 Sense front pinout](images/sense-front.png)

*Front pinout of the XIAO ESP32S3 Sense.*

Find the **USB-C socket**, **BOOT**, **RESET**, **GND**, **3V3** and **user LED**.

A **GPIO** is a general-purpose input/output connection. The board's printed labels and processor GPIO numbers are not always the same: **D0 corresponds to GPIO1**. A bare pin number in our ESP32 code refers to the GPIO number.

- **RESET** restarts the installed program.
- **BOOT** enters download mode when uploading fails.
- **GND** is the common electrical reference.
- **3V3** is the regulated 3.3 V supply.

> [!IMPORTANT]
> USB supplies 5 V power, but GPIO signals use **3.3 V**. Do not connect a 5 V signal directly to a GPIO.

### Camera, microphone and microSD

![Sense expansion-board connections](images/sense-back.png)

*The Sense expansion board.*

The Sense SD interface uses:

| Connection | GPIO |
|---|---:|
| SCK (clock) | 7 |
| MISO (data from the card) | 8 |
| MOSI (data to the card) | 9 |
| CS (chip select) | **21** |

For SD chip select, follow the [Sense filesystem instructions](https://wiki.seeedstudio.com/xiao_esp32s3_sense_filesystem/), which specify `SD.begin(21)`. Some reference drawings show a different CS label.

> [!IMPORTANT]
> The user LED also uses **GPIO21**. Do not blink the LED in a sketch that uses the SD card.

### Memory and files

**Flash** keeps the program when power is disconnected. **RAM and PSRAM** hold temporary working data, which is lost when power is removed. The **microSD card** holds files deliberately saved by the program.

---

# Installation

Download **Arduino IDE 2** from the [official Arduino software page](https://www.arduino.cc/en/software).

## Working folder

Use a folder you can find again, such as `Misheard_City/Group_01`. Arduino's **Sketchbook location** can be set in Preferences.

![Arduino Preferences showing the sketchbook location](images/workingDirectory.png)

*The sketchbook location in Preferences.*

Keep each sketch inside a folder with the same name:

`01_Blink/01_Blink.ino`

## Install the ESP32 board package

1. Open **Preferences** (`File → Preferences` on Windows; the Arduino application menu on macOS).
2. Add this address to **Additional Boards Manager URLs**, keeping any existing entries:

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

3. Open **Boards Manager** and search for `esp32`.
4. Install **esp32 by Espressif Systems**, choosing version **2.0.17** from the version list. Keep this version for all workshop examples; other versions may need changes to the code.

![ESP32 package in Boards Manager](images/boards-manager.png)

*The ESP32 package in Boards Manager. Install 2.0.17, not the version shown.*

## Board packages, libraries and header files

| What you need | Where to install it | Example |
|---|---|---|
| Support for a family of boards, including its core and bundled libraries | **Boards Manager** | **esp32 by Espressif Systems** |
| An additional library for a particular device or task | **Library Manager** | A sensor library specified in a later practical |

**For Day 1, the ESP32 board package supplies every header our examples use** (`Arduino.h`, `esp_camera.h`, `FS.h`, `SD.h`, `SPI.h`). Do not install them through Library Manager or copy `.h` files into the sketch folder.

References: [Espressif installation guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html), [bundled ESP32 libraries](https://github.com/espressif/arduino-esp32/tree/master/libraries), and [camera driver installation for Arduino IDE](https://github.com/espressif/esp32-camera#arduino-ide).

## Connect and select the board

Connect the XIAO with a **USB data cable**. A charging-only cable may power the board without allowing programming.

Select **XIAO_ESP32S3** and the connected **port**.

![XIAO ESP32S3 board selection](images/board-selection.png)

*Board selection. The Sense version also uses XIAO_ESP32S3.*

On Windows the port may look like `COM5`; on macOS it contains `usbmodem`.

### Settings

| Setting | Selection |
|---|---|
| Board | XIAO_ESP32S3 |
| USB CDC On Boot | Enabled |
| PSRAM | OPI PSRAM, for the camera practical |
| Serial Monitor | 115200 baud |
| Other settings | Board defaults |

If the port is missing or uploading fails, disconnect USB, hold **BOOT**, reconnect USB and release BOOT. Select the port again and upload. Press RESET afterwards if needed. See [Seeed's recovery instructions](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#bootloader-mode).

---

# Coding in Arduino (C/C++)

A program (called a *sketch*) usually has this structure:

1. **Libraries:** extra code you include, for example for the camera or the SD card.
2. **Variables and constants:** names for the numbers, pins and settings the sketch uses.
3. **`setup()`:** runs once when the board starts or is reset.
4. **`loop()`:** repeats for as long as the board is running.
5. **Functions:** named blocks of code that you can reuse.

```cpp
void setup() {
  // Prepare the board.
}

void loop() {
  // Repeat an action.
}
```

![Diagram of compilation and upload](images/Compiling.png)

*The IDE compiles your sketch and uploads it to the board.*

**Verify** compiles the sketch. **Upload** compiles it and sends it to the board. Uploading a new sketch replaces the previous program.

[Arduino language reference](https://docs.arduino.cc/language-reference/)

---

## 001 - Basic LED Blink

[Open the complete sketch](01_Blink/01_Blink.ino)

The built-in **user LED is active-low**: `LOW` switches it on and `HIGH` switches it off. With XIAO_ESP32S3 selected, `LED_BUILTIN` refers to its user LED on GPIO21.

```cpp
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
```

Upload, then find the blinking user LED (not the separate charging light).

**Try:** change the timings to 100 and 900. Predict the difference, then upload and compare.

### Read 001 line by line

| Code | Meaning |
|---|---|
| `// MISHEARD CITY ...` | A **comment**, ignored by the compiler |
| `const int ledPin = LED_BUILTIN;` | A **constant** of **type** `int` (whole number) named `ledPin`, holding `LED_BUILTIN`. `;` ends the instruction |
| `int onTimeMs = 300;` | A **variable**: a value that can change, here 300 ms |
| `void setup() { ... }` | **Defines** the function `setup`. `void`: no result; `( )`: no inputs; `{ }`: its instructions |
| `pinMode(ledPin, OUTPUT);` | **Calls** `pinMode` with two **arguments**: the pin and its mode |
| `digitalWrite(ledPin, HIGH);` | Sets the pin `HIGH`: LED off |
| `void loop() { ... }` | Runs again and again |
| `delay(onTimeMs);` | Waits for the current value of `onTimeMs` |

`LED_BUILTIN`, `OUTPUT`, `HIGH` and `LOW` are constants defined by Arduino and the board package.

Common first errors:

- Names are **case-sensitive**: `digitalWrite` works, `digitalwrite` does not.
- Names have no spaces and do not start with a digit: `onTimeMs`, not `on time ms`.
- Most instructions end with `;`. Function definitions and `if` / `for` blocks end with `}` instead.
- Every `(` and `{` needs its closing `)` and `}`.

## 002 - Variables and serial feedback

[Open the complete sketch](02_Blink_and_Serial/02_Blink_and_Serial.ino)

| Type | Example | Use |
|---|---|---|
| `bool` | `true`, `false` | A state or condition |
| `char` | `'c'` | One character |
| `int` | `300` | A whole number |
| `unsigned long` | `cycleCount` | A non-negative counter or time value |
| `float` | `0.72f` | A decimal value |
| `String` | `"moss"` | Text, such as a label |

A declaration always has the same form: **type, name, value**.

```cpp
int onTimeMs = 300;
float threshold = 0.6;
bool fast = false;
String label = "moss";
```

`=` stores a value: `cycleCount = cycleCount + 1;` adds 1 to the current value. Whole numbers divide without a remainder:

```cpp
Serial.println(7 / 2);      // int division: prints 3
Serial.println(7.0 / 2.0);  // decimal division: prints 3.50
```

Three additions send each cycle to the Serial Monitor:

```cpp
// Above setup(): a value we will update.
unsigned long cycleCount = 0;
```

```cpp
// Inside setup(): start the USB serial connection.
Serial.begin(115200);
```

```cpp
// Inside loop(): update and report the counter.
cycleCount = cycleCount + 1;
Serial.print("cycle=");
Serial.println(cycleCount);
```

The complete sketch:

```cpp
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
```

Open **Serial Monitor** and choose **115200 baud**. Change `GROUP_01` to your group's name.

`Serial.print()` continues the current line; `Serial.println()` ends it.

**Try:** change a timing value and compare the printed message with the light. Press RESET: the counter restarts.

## Reading an error message

When **Verify** fails, the output panel lists errors in this form:

```text
01_Blink.ino:7:1: error: expected ',' or ';' before 'int'
```

| Part | Meaning |
|---|---|
| `01_Blink.ino` | The file |
| `7:1` | Line 7, character 1. The IDE also highlights the line |
| `error: ...` | What the compiler expected or could not find |

Fix the **first** error first; later messages often disappear with it. Lines starting with `note:` belong to the error above.

| Message | Usual cause |
|---|---|
| `expected ',' or ';' before ...` | A missing `;` at the end of the **previous** line. In the example above, it is missing on line 6 |
| `'digitalwrite' was not declared in this scope` | A misspelled or wrongly capitalised name. Check the `suggested alternative` below it |
| `'ontimeMs' was not declared in this scope` | A variable used under a different name, or never created |
| `expected '}' at end of input` | A `{` without its closing `}`. The `note:` shows the opening bracket |

When asking an AI assistant for help, paste the first error message exactly, with the code around the line it names.

<details>
<summary>003–005: functions, conditions and repetition</summary>

## 003 - Blink using a function

A **function** gives an action a name. Copy this sketch into a new file.

```cpp
const int ledPin = LED_BUILTIN;

void flash(unsigned long periodMs) {
  digitalWrite(ledPin, LOW);
  delay(periodMs);
  digitalWrite(ledPin, HIGH);
  delay(periodMs);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
}

void loop() {
  flash(100);
  flash(500);
}
```

`periodMs` is a **parameter**: in `flash(100)` it receives the value 100. `pinMode()`, `digitalWrite()` and `delay()` are functions too, written by Arduino.

**Try:** describe the sequence before uploading, then swap the two calls. Keep this sketch for the next examples.

## 004 - Control structure: `if`

An `if` statement chooses an action according to a condition.

In example 003, add this variable **above `setup()`**:

```cpp
bool fast = false;
```

Then **replace its `loop()`** with:

```cpp
void loop() {
  if (fast == true) {
    flash(100);
  } else {
    flash(500);
  }
}
```

Keep the existing `flash()` and `setup()`. Do not add a second `loop()`.

**Try:** change `fast` to `true` and upload again. `=` assigns a value; `==` compares two values.

## 005 - Repetition with `for`

A `for` loop repeats an action a specified number of times.

Using the function from example 003, **replace `loop()`** with:

```cpp
void loop() {
  for (int i = 0; i < 3; i++) {
    flash(100);
  }
  delay(1000);
}
```

`i` starts at 0 and increases by 1 while it is below 3: three pulses, then a pause.

**Try:** change the number of pulses.

### Put 003–005 together

Function, condition and repetition in one sketch. Replace the previous sketch with it.

```cpp
const int ledPin = LED_BUILTIN;
bool fast = false;

void flash(unsigned long periodMs) {
  digitalWrite(ledPin, LOW);
  delay(periodMs);
  digitalWrite(ledPin, HIGH);
  delay(periodMs);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
}

void loop() {
  for (int i = 0; i < 3; i++) {
    if (fast) {
      flash(100);
    } else {
      flash(500);
    }
  }
  delay(1000);
}
```

**Try:** change `fast` or the number of repetitions. Predict the result first.

---

</details>

# AI-Assisted Programming

Change your blink sketch with an AI assistant. Decide on one change, describe it clearly and compare the result with your intention.

For example, create **two short pulses, one longer pulse and a pause**.

> I am using Arduino C++ with a Seeed Studio XIAO ESP32S3 Sense. Its built-in user LED is GPIO21 and active-low. This exercise does not use the SD card or external components.
>
> Here is our current sketch: [paste the code].
>
> Change the rhythm to two short pulses, one longer pulse and a pause. Keep the pin assignment and use the functions already introduced. Explain the timing, identify the changed lines and provide the complete revised sketch. Explain any assumptions.

Read the changed lines together before uploading.

Keep a short record:

| Intended change | Changed code | Observed behaviour | Next revision |
|---|---|---|---|
| | | | |

If something fails, give the assistant the exact error, board selection and package version, and ask for a small correction.

Always test a suggestion on the board: an explanation from AI does not prove that the code works.

**Next:** [Camera and microSD practical](04_Camera_and_SD/README.md)

---

# Further Coding Examples

## 006 - Loops with arrays

An **array** stores several values under one name. Its first index is zero.

Starting with example 003, add this array above `setup()`:

```cpp
unsigned long durationsMs[] = {100, 100, 500};
```

Replace `loop()` with:

```cpp
void loop() {
  for (int i = 0; i < 3; i++) {
    flash(durationsMs[i]);
  }
  delay(1000);
}
```

Keep `flash()`. If you change the number of entries, change the loop bound `3` too.

## 007 - Communicating via Serial Monitor

[Open the complete serial-control sketch](03_Serial_Control_Optional/03_Serial_Control_Optional.ino)

You can send a command from the computer to control the light.

```cpp
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
```

Open Serial Monitor at 115200 baud. Send **1** to switch the LED on and **0** to switch it off.

`Serial.available()` indicates that data is waiting; `Serial.read()` receives one character. `'1'` is a character, not the numerical value `1`.

## 008 - Sending and receiving messages

A **String** can hold several characters. This complete sketch collects characters until you send a newline, then echoes the message.

```cpp
String message = "";

void setup() {
  Serial.begin(115200);
}

void loop() {
  while (Serial.available() > 0) {
    char incoming = Serial.read();

    if (incoming == '\n') {
      Serial.print("You typed: ");
      Serial.println(message);
      message = "";
    } else if (incoming != '\r') {
      if (message.length() < 80) {
        message += incoming;
      }
    }
  }
}
```

Set Serial Monitor to **115200 baud** and **Newline**, type a short message and send it.

---

## Bring to the next tutorial

Show two camera photographs that made you notice a difference in framing, light or distance. Together, prepare the short [research task](05_Research/README.md): an urban interest, a passage from one reading, and two or three provisional categories with an ambiguous example.

Save the sketch that worked on your board and one sentence explaining your change.

## If something does not work

| Symptom | First thing to check |
|---|---|
| No port appears | A data-capable cable, the USB connection and BOOT recovery |
| Upload cannot connect | XIAO_ESP32S3 selection, correct port and any other app using it |
| Two `setup()` or `loop()` definitions | Replace the previous example rather than appending another complete sketch |
| No serial output | USB CDC On Boot enabled, the running port selected and monitor open; press RESET |
| Unreadable serial text | 115200 baud |
| LED seems inverted | The XIAO user LED is active-low |
| Camera or SD issue | Follow the [camera troubleshooting notes](04_Camera_and_SD/README.md#troubleshooting) |

Keep the error message and current sketch when asking for help.

## Sources and image credits

The organisation, background material, introductory illustrations and numbered exercise sequence are adapted from [Pervasive Urbanism 2025–26 — Day 1 Arduino Coding](https://github.com/PervasiveUrbanism/PervasiveUrbanism_25-26/tree/main/Skills%20Module%202%20Prosthetic%20Clouds/Skills%202%20-%20Day%201%20Arduino%20Coding), reused with permission obtained by the workshop instructor. The Arduino logo, Ivrea photograph, early prototype photograph, Preferences screenshot and compilation diagram come from that course's image folder; original rights remain with their respective creators.

The board photographs, pin diagrams and ESP32 setup screenshots come from [Seeed Studio's XIAO ESP32S3 guide](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/), reproduced unchanged under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/), according to [Seeed's licence page](https://wiki.seeedstudio.com/License/). Hardware-specific explanations and exercises have been adapted for the XIAO ESP32S3 Sense.

[Continue to Camera and microSD](04_Camera_and_SD/README.md) · [Reading and research task](05_Research/README.md)
