// Misheard City — Day 1: capture a JPEG when Serial Monitor sends 'c'.
// Board: XIAO_ESP32S3 with Sense camera / microSD expansion.
// USB CDC On Boot: Enabled. PSRAM: OPI PSRAM.
// Adapted for teaching from Seeed's camera and microSD examples.
// See the source and licence notes in the accompanying README.

#include <Arduino.h>
#include "esp_camera.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

const int SD_CS_PIN = 21;  // Shared with the user LED: no blinking here.
bool deviceReady = false;
unsigned long nextImage = 1;

bool startCamera() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  // Fixed connections on the XIAO ESP32S3 Sense camera.
  config.pin_d0 = 15;
  config.pin_d1 = 17;
  config.pin_d2 = 18;
  config.pin_d3 = 16;
  config.pin_d4 = 14;
  config.pin_d5 = 12;
  config.pin_d6 = 11;
  config.pin_d7 = 48;
  config.pin_xclk = 10;
  config.pin_pclk = 13;
  config.pin_vsync = 38;
  config.pin_href = 47;
  config.pin_sccb_sda = 40;
  config.pin_sccb_scl = 39;
  config.pin_pwdn = -1;
  config.pin_reset = -1;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;  // 320 x 240, for a small first image.
  config.jpeg_quality = 12;           // Lower number = higher JPEG quality.
  config.fb_count = 1;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  if (!psramFound()) {
    Serial.println("PSRAM unavailable. Check Tools > PSRAM.");
    return false;
  }

  esp_err_t result = esp_camera_init(&config);
  if (result != ESP_OK) {
    Serial.printf("Camera initialisation failed: 0x%x\n", result);
    return false;
  }
  return true;
}

void capturePhoto() {
  // Discard a queued frame so the next capture better reflects the new view.
  camera_fb_t *oldFrame = esp_camera_fb_get();
  if (oldFrame != nullptr) {
    esp_camera_fb_return(oldFrame);
  }
  delay(150);

  camera_fb_t *frame = esp_camera_fb_get();
  if (frame == nullptr) {
    Serial.println("No camera frame received.");
    return;
  }

  char filename[40];
  do {
    snprintf(filename, sizeof(filename), "/image_%04lu.jpg", nextImage++);
  } while (SD.exists(filename));  // Do not overwrite earlier photographs.

  File photo = SD.open(filename, FILE_WRITE);
  if (!photo) {
    Serial.println("Could not open a new image file.");
    esp_camera_fb_return(frame);
    return;
  }

  size_t expectedBytes = frame->len;
  size_t writtenBytes = photo.write(frame->buf, expectedBytes);
  photo.close();
  esp_camera_fb_return(frame);   // Release the camera buffer.

  if (writtenBytes != expectedBytes) {
    Serial.printf("Incomplete file: %s (%u of %u bytes).\n",
                  filename,
                  (unsigned int)writtenBytes,
                  (unsigned int)expectedBytes);
    Serial.println("Record this filename as incomplete; check the card.");
    return;
  }

  Serial.printf("Saved %s (%u bytes).\n",
                filename, (unsigned int)writtenBytes);
}

void setup() {
  Serial.begin(115200);
  // Give USB serial a moment to connect without waiting forever.
  unsigned long waitStarted = millis();
  while (!Serial && millis() - waitStarted < 3000) {
    delay(10);
  }

  Serial.println("Misheard City: camera to microSD");

  if (!startCamera()) {
    Serial.println("Resolve the camera issue, then press RESET.");
    return;
  }

  SPI.begin(7, 8, 9, SD_CS_PIN);  // SCK, MISO, MOSI, CS.
  if (!SD.begin(SD_CS_PIN) || SD.cardType() == CARD_NONE) {
    Serial.println("SD card unavailable. Check the card and its format.");
    Serial.println("Disconnect power before adjusting the card.");
    return;
  }

  delay(1000);
  deviceReady = true;
  Serial.println("Ready. Send c to capture a photograph.");
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 'c' || command == 'C') {
      if (deviceReady) {
        capturePhoto();
      } else {
        Serial.println("Device not ready. Read the setup messages.");
      }
    }
  }
  delay(10);
}

/*
MIT License

Copyright (c) 2008-2016 Seeed Development Limited (www.seeedstudio.com / www.seeed.cc)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
