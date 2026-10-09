// MISHEARD CITY | Day 3 | 02 - Field recorder
// Saves a photograph and the model's reading of that same photograph
// as one row of a CSV file on the microSD card.
// Board: XIAO_ESP32S3 with Sense camera / microSD. esp32 by Espressif 2.0.17.
// USB CDC On Boot: Enabled. PSRAM: OPI PSRAM. Serial Monitor: 115200.
// Adapted for teaching from the Edge Impulse esp32_camera example (XIAO version
// by Marcelo Rovai and Seeed Studio) and the Day 1 camera / microSD sketch.
// See the source and licence notes in the accompanying README.

// Replace this line with the header of YOUR group's exported library.
#include <Misheard_City_Group_01_inferencing.h>

#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"
#include "img_converters.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "This sketch needs an Edge Impulse image model."
#endif
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
#error "This sketch expects image classification, not object detection."
#endif
#if defined(EI_CLASSIFIER_RESIZE_MODE) && EI_CLASSIFIER_RESIZE_MODE != EI_CLASSIFIER_RESIZE_FIT_SHORTEST
#error "Set Impulse design > Image data > Resize mode to Fit shortest axis, then retrain and export again."
#endif

// ---- Settings to change ----------------------------------------------------
const bool AUTO_CAPTURE = false;              // false: send c; true: timed records.
const unsigned long CAPTURE_INTERVAL_MS = 10000;
const bool SAVE_MODEL_VIEW = true;            // Also save the 96 x 96 image the model read.
// -----------------------------------------------------------------------------

const int SD_CS_PIN = 21;   // Shared with the user LED: no blinking here.
const int FRAME_WIDTH = 320;
const int FRAME_HEIGHT = 240;

uint8_t *rgbBuffer = nullptr;
bool deviceReady = false;
unsigned long nextImage = 1;
unsigned long lastRecordAt = 0;
char logPath[20];

bool startCamera() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  // Fixed connections on the XIAO ESP32S3 Sense camera (same as Day 1).
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
  config.frame_size = FRAMESIZE_QVGA;  // Must match FRAME_WIDTH x FRAME_HEIGHT.
  config.jpeg_quality = 12;
  config.fb_count = 2;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST;

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

int getModelData(size_t offset, size_t length, float *out) {
  size_t byteIndex = offset * 3;
  for (size_t i = 0; i < length; i++) {
    // fmt2rgb888 stores each pixel as B, G, R, so the order is swapped here.
    out[i] = (rgbBuffer[byteIndex + 2] << 16) +
             (rgbBuffer[byteIndex + 1] << 8) +
             rgbBuffer[byteIndex];
    byteIndex += 3;
  }
  return 0;
}

// Choose a new log file for each session, e.g. /log_03.csv.
bool startLog() {
  for (int session = 1; session < 100; session++) {
    snprintf(logPath, sizeof(logPath), "/log_%02d.csv", session);
    if (!SD.exists(logPath)) {
      File logFile = SD.open(logPath, FILE_WRITE);
      if (!logFile) {
        return false;
      }
      logFile.print("record,millis_ms,image,top_label,top_score");
      for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        logFile.print(",score_");
        logFile.print(ei_classifier_inferencing_categories[i]);
      }
      logFile.println();
      logFile.close();
      return true;
    }
  }
  return false;
}

// Save the model's input, still in rgbBuffer, as a BMP: /rec_0007_model.bmp.
// BMP stores pixels as B, G, R, the same order fmt2rgb888 produces.
void saveModelView(unsigned long recordNumber) {
  const int w = EI_CLASSIFIER_INPUT_WIDTH;
  const int h = EI_CLASSIFIER_INPUT_HEIGHT;
  const int rowSize = w * 3;
  const int padding = (4 - (rowSize % 4)) % 4;    // Each BMP row fills a multiple of 4 bytes.
  const uint32_t fileSize = 54 + (rowSize + padding) * h;
  const int32_t topDown = -h;                      // Negative height: first row is the top.

  uint8_t header[54] = {
    'B', 'M',
    (uint8_t)fileSize, (uint8_t)(fileSize >> 8), (uint8_t)(fileSize >> 16), (uint8_t)(fileSize >> 24),
    0, 0, 0, 0, 54, 0, 0, 0,                       // Pixel data starts at byte 54.
    40, 0, 0, 0,                                   // Size of the info header.
    (uint8_t)w, (uint8_t)(w >> 8), (uint8_t)(w >> 16), (uint8_t)(w >> 24),
    (uint8_t)topDown, (uint8_t)(topDown >> 8), (uint8_t)(topDown >> 16), (uint8_t)(topDown >> 24),
    1, 0, 24, 0                                    // One plane, 24 bits per pixel; the rest stays 0.
  };

  char path[28];
  snprintf(path, sizeof(path), "/rec_%04lu_model.bmp", recordNumber);
  File bmp = SD.open(path, FILE_WRITE);
  if (!bmp) {
    Serial.println("Could not open the model view file.");
    return;
  }
  const uint8_t zeros[3] = { 0, 0, 0 };
  bmp.write(header, sizeof(header));
  for (int y = 0; y < h; y++) {
    bmp.write(rgbBuffer + y * rowSize, rowSize);
    bmp.write(zeros, padding);
  }
  bmp.close();
}

void makeRecord() {
  unsigned long recordTime = millis();

  camera_fb_t *frame = esp_camera_fb_get();
  if (frame == nullptr) {
    Serial.println("No camera frame received.");
    return;
  }

  // 1. Save the photograph exactly as the camera produced it.
  unsigned long recordNumber;
  char imagePath[24];
  do {
    recordNumber = nextImage++;
    snprintf(imagePath, sizeof(imagePath), "/rec_%04lu.jpg", recordNumber);
  } while (SD.exists(imagePath));  // Do not overwrite earlier records.

  File photo = SD.open(imagePath, FILE_WRITE);
  if (!photo) {
    Serial.println("Could not open a new image file.");
    esp_camera_fb_return(frame);
    return;
  }
  size_t writtenBytes = photo.write(frame->buf, frame->len);
  bool photoComplete = (writtenBytes == frame->len);
  photo.close();

  // 2. Convert the same frame for the model, then release the camera buffer.
  bool converted = fmt2rgb888(frame->buf, frame->len, PIXFORMAT_JPEG, rgbBuffer);
  esp_camera_fb_return(frame);

  if (!photoComplete) {
    Serial.printf("Incomplete file: %s. Check the card.\n", imagePath);
    return;
  }
  if (!converted) {
    Serial.println("Could not convert the JPEG frame.");
    return;
  }

  // Crop the centre and scale it to the model's input size, as in 01.
  if (EI_CLASSIFIER_INPUT_WIDTH != FRAME_WIDTH ||
      EI_CLASSIFIER_INPUT_HEIGHT != FRAME_HEIGHT) {
    ei::image::processing::crop_and_interpolate_rgb888(
        rgbBuffer, FRAME_WIDTH, FRAME_HEIGHT,
        rgbBuffer, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT);
  }

  if (SAVE_MODEL_VIEW) {
    saveModelView(recordNumber);
  }

  // 3. Classify.
  ei::signal_t signal;
  signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
  signal.get_data = &getModelData;

  ei_impulse_result_t result = { 0 };
  EI_IMPULSE_ERROR error = run_classifier(&signal, &result, false);
  if (error != EI_IMPULSE_OK) {
    Serial.printf("Classifier error: %d\n", error);
    return;
  }

  size_t top = 0;
  for (size_t i = 1; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (result.classification[i].value > result.classification[top].value) {
      top = i;
    }
  }

  // 4. Build one CSV row and append it to the session log.
  String row = String(recordNumber) + "," + String(recordTime) + "," +
               String(imagePath + 1) + "," +            // Without the leading "/".
               result.classification[top].label + "," +
               String(result.classification[top].value, 3);
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    row += "," + String(result.classification[i].value, 3);
  }

  File logFile = SD.open(logPath, FILE_APPEND);  // FILE_WRITE would erase the log.
  if (!logFile) {
    Serial.println("Could not open the log file.");
    return;
  }
  logFile.println(row);
  logFile.close();

  Serial.println(row);
}

void setup() {
  Serial.begin(115200);
  unsigned long waitStarted = millis();
  while (!Serial && millis() - waitStarted < 3000) {
    delay(10);
  }

  Serial.println("Misheard City: field recorder");

  if (!startCamera()) {
    Serial.println("Resolve the camera issue, then press RESET.");
    return;
  }

  rgbBuffer = (uint8_t *)ps_malloc(FRAME_WIDTH * FRAME_HEIGHT * 3);
  if (rgbBuffer == nullptr) {
    Serial.println("Could not reserve image memory. Check Tools > PSRAM.");
    return;
  }

  SPI.begin(7, 8, 9, SD_CS_PIN);  // SCK, MISO, MOSI, CS.
  if (!SD.begin(SD_CS_PIN) || SD.cardType() == CARD_NONE) {
    Serial.println("SD card unavailable. Check the card and its format.");
    Serial.println("Disconnect power before adjusting the card.");
    return;
  }

  if (!startLog()) {
    Serial.println("Could not create a log file on the card.");
    return;
  }

  delay(1000);
  deviceReady = true;
  Serial.printf("Ready. Logging to %s\n", logPath);
  if (AUTO_CAPTURE) {
    Serial.printf("Recording every %lu s. Send c for an extra record.\n",
                  CAPTURE_INTERVAL_MS / 1000);
  } else {
    Serial.println("Send c to make a record.");
  }
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'c' || command == 'C') {
      if (deviceReady) {
        makeRecord();
      } else {
        Serial.println("Device not ready. Read the setup messages.");
      }
    }
  }

  if (deviceReady && AUTO_CAPTURE &&
      millis() - lastRecordAt >= CAPTURE_INTERVAL_MS) {
    lastRecordAt = millis();
    makeRecord();
  }
  delay(10);
}

/*
Edge Impulse Arduino examples
Copyright (c) 2022 EdgeImpulse Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

XIAO ESP32S3 adaptation: Marcelo Rovai, XIAO-ESP32S3-Sense
(https://github.com/Mjrovai/XIAO-ESP32S3-Sense), Apache License 2.0.

Day 1 camera / microSD sketch: adapted from Seeed Studio examples,
MIT License, Copyright (c) 2008-2016 Seeed Development Limited.

Model view BMP writer: adapted from Kunpeng Lei, City Sensory AI - The Eye
(XIAO ESP32S3 Sense field installation, Venice).
*/
