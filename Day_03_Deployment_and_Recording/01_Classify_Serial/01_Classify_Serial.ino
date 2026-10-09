// MISHEARD CITY | Day 3 | 01 - Classify camera images and report over Serial
// Board: XIAO_ESP32S3 with Sense camera. Board package: esp32 by Espressif 2.0.17.
// USB CDC On Boot: Enabled. PSRAM: OPI PSRAM. Serial Monitor: 115200.
// Adapted for teaching from the Edge Impulse esp32_camera example, as modified
// for the XIAO ESP32S3 Sense by Marcelo Rovai and Seeed Studio.
// See the source and licence notes in the Day 3 README.

// Replace this line with the header of YOUR group's exported library.
// Find the exact name in the first line of the library's own example sketch.
#include <Misheard_City_Group_01_inferencing.h>

#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"
#include "img_converters.h"

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "This sketch needs an Edge Impulse image model."
#endif
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
#error "This sketch expects image classification, not object detection."
#endif
#if defined(EI_CLASSIFIER_RESIZE_MODE) && EI_CLASSIFIER_RESIZE_MODE != EI_CLASSIFIER_RESIZE_FIT_SHORTEST
#error "Set Impulse design > Image data > Resize mode to Fit shortest axis, then retrain and export again."
#endif

// Size of each camera frame before it is reduced for the model.
const int FRAME_WIDTH = 320;   // QVGA
const int FRAME_HEIGHT = 240;

const float CONFIDENCE_THRESHOLD = 0.60f;      // Below this, report "uncertain".
const unsigned long CLASSIFY_INTERVAL_MS = 1000;

uint8_t *rgbBuffer = nullptr;  // One frame as RGB pixels, 3 bytes per pixel.
bool deviceReady = false;
unsigned long readingCount = 0;
unsigned long lastReadingAt = 0;

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
  config.grab_mode = CAMERA_GRAB_LATEST;  // Always classify the newest view.

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

// Take one photograph and turn it into the small RGB image the model expects.
bool captureForModel() {
  camera_fb_t *frame = esp_camera_fb_get();
  if (frame == nullptr) {
    Serial.println("No camera frame received.");
    return false;
  }

  bool converted = fmt2rgb888(frame->buf, frame->len, PIXFORMAT_JPEG, rgbBuffer);
  esp_camera_fb_return(frame);  // Release the camera buffer.

  if (!converted) {
    Serial.println("Could not convert the JPEG frame.");
    return false;
  }

  // Crop the centre of the frame and scale it to the model's input size,
  // e.g. 96 x 96. The model never sees the edges of the 320 x 240 view.
  if (EI_CLASSIFIER_INPUT_WIDTH != FRAME_WIDTH ||
      EI_CLASSIFIER_INPUT_HEIGHT != FRAME_HEIGHT) {
    ei::image::processing::crop_and_interpolate_rgb888(
        rgbBuffer, FRAME_WIDTH, FRAME_HEIGHT,
        rgbBuffer, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT);
  }
  return true;
}

// Edge Impulse asks for pixels in portions; each pixel is packed as 0xRRGGBB.
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

void classifyAndReport() {
  if (!captureForModel()) {
    return;
  }

  ei::signal_t signal;
  signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
  signal.get_data = &getModelData;

  ei_impulse_result_t result = { 0 };
  EI_IMPULSE_ERROR error = run_classifier(&signal, &result, false);
  if (error != EI_IMPULSE_OK) {
    Serial.printf("Classifier error: %d\n", error);
    return;
  }

  readingCount = readingCount + 1;
  Serial.printf("reading=%lu, dsp_ms=%d, classify_ms=%d\n",
                readingCount, result.timing.dsp, result.timing.classification);

  size_t top = 0;
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    Serial.printf("  %s: %.3f\n",
                  result.classification[i].label,
                  result.classification[i].value);
    if (result.classification[i].value > result.classification[top].value) {
      top = i;
    }
  }

  float topScore = result.classification[top].value;
  if (topScore >= CONFIDENCE_THRESHOLD) {
    Serial.printf("top=%s, score=%.2f\n\n", result.classification[top].label, topScore);
  } else {
    Serial.printf("top=uncertain, best_guess=%s, score=%.2f\n\n",
                  result.classification[top].label, topScore);
  }
}

void setup() {
  Serial.begin(115200);
  // Give USB serial a moment to connect without waiting forever.
  unsigned long waitStarted = millis();
  while (!Serial && millis() - waitStarted < 3000) {
    delay(10);
  }

  Serial.println("Misheard City: on-device classification");
  Serial.printf("Model input: %d x %d, labels: %d\n",
                EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT,
                EI_CLASSIFIER_LABEL_COUNT);

  if (!startCamera()) {
    Serial.println("Resolve the camera issue, then press RESET.");
    return;
  }

  rgbBuffer = (uint8_t *)ps_malloc(FRAME_WIDTH * FRAME_HEIGHT * 3);
  if (rgbBuffer == nullptr) {
    Serial.println("Could not reserve image memory. Check Tools > PSRAM.");
    return;
  }

  deviceReady = true;
  Serial.println("Ready. Classifying once per second.\n");
}

void loop() {
  if (!deviceReady) {
    delay(1000);
    return;
  }

  if (millis() - lastReadingAt >= CLASSIFY_INTERVAL_MS) {
    lastReadingAt = millis();
    classifyAndReport();
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
*/
