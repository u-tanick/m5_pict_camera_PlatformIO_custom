#include <M5Unified.h>

// ----------------------------------------------------
// Grove PIN
#define PIN_SCL 1
#define PIN_SDA 2

// ----------------------------------------------------
// ユニットスクロール関連
#include "M5UnitScroll.h"
M5UnitScroll scroll;
bool PRESSED_FLG = false;

// ----------------------------------------------------
// UnitLCD関連
#include <M5UnitLCD.h>
M5UnitLCD display;
M5Canvas canvas(&display);

// ----------------------------------------------------
// 汎用プリント関数
void printlnStr(String str) {
  display.println(str);
  Serial.println(str);
}
void printlnInt(int16_t i) {
  display.println(i);
  Serial.println(i);
}

// ----------------------------------------------------
// カメラ・PSRAM関連
#include <esp_camera.h>
#define POWER_GPIO_NUM 18
camera_fb_t* fb;
camera_config_t camera_config = {
  .pin_pwdn = -1,
  .pin_reset = -1,
  .pin_xclk = 21,
  .pin_sscb_sda = 12,
  .pin_sscb_scl = 9,
  .pin_d7 = 13,
  .pin_d6 = 11,
  .pin_d5 = 17,
  .pin_d4 = 4,
  .pin_d3 = 48,
  .pin_d2 = 46,
  .pin_d1 = 42,
  .pin_d0 = 3,

  .pin_vsync = 10,
  .pin_href = 14,
  .pin_pclk = 40,

  .xclk_freq_hz = 20000000,
  .ledc_timer = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,

  .pixel_format = PIXFORMAT_RGB565,
  .frame_size = FRAMESIZE_HQVGA,
  // FRAMESIZE_96X96,    // 96x96
  // FRAMESIZE_QQVGA,    // 160x120
  // FRAMESIZE_QCIF,     // 176x144
  // FRAMESIZE_HQVGA,    // 240x176
  // FRAMESIZE_240X240,  // 240x240
  // FRAMESIZE_QVGA,     // 320x240

  .jpeg_quality = 0,
  .fb_count = 2,
  .fb_location = CAMERA_FB_IN_PSRAM,
  .grab_mode = CAMERA_GRAB_LATEST,
  .sccb_i2c_port = 0,
};

bool CameraBegin() {
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    printlnStr(" Error Code");
    printlnInt(err);
    return false;
  }

  // カメラ追加設定
  sensor_t* s = esp_camera_sensor_get();

  // AtomS3R Cam のときはこちら
  s->set_hmirror(s, 1);  // 左右反転 0無効 1有効
  s->set_vflip(s, 1);    // 上下反転 0無効 1有効

  // AtomS3R M12 のときはこちら
  //s->set_lenc(s, 1);     // レンズ補正？ 効いてるか微妙
  //s->set_hmirror(s, 1);  // 左右反転 0無効 1有効
  //s->set_vflip(s, 0);    // 上下反転 0無効 1有効

  return true;
}

bool CameraGet() {
  fb = esp_camera_fb_get();
  if (!fb) {
    return false;
  }
  return true;
}

bool CameraFree() {
  if (fb) {
    esp_camera_fb_return(fb);
    return true;
  }
  return false;
}


// ----------------------------------------------------
// setup
void setup()
{
    Serial.begin(115200);

  // ----------------------------------------------------
  // UnitLCD初期化
  display.init(PIN_SDA, PIN_SCL);
  display.setTextScroll(true);
  display.setRotation(1);
  display.setColorDepth(8);
  display.setBrightness(128);
  display.setTextSize(1.8);
  canvas.createSprite(240, 176);
  // canvas.createSprite(240, 176);  // カメラ画像表示
  // canvas.createSprite(240, 135);  // 変換画像表示
  printlnStr(" OK : UnitLCD");
  delay(500);

  // ----------------------------------------------------
  // ユニットスクロール初期化
  scroll.begin(&Wire, SCROLL_ADDR, PIN_SDA, PIN_SCL, 400000U);
  printlnStr(" OK : UnitScroll");
  delay(500);

  // ----------------------------------------------------
  // PSRAMチェック
  if (psramFound()) {
    camera_config.pixel_format = PIXFORMAT_RGB565;
    camera_config.fb_location = CAMERA_FB_IN_PSRAM;
    camera_config.fb_count = 2;
    printlnStr(" OK : PSRAM");
  } else {
    printlnStr(" NG : PSRAM not found");
    delay(1500);
    ESP.restart();
  }
  delay(500);

  // ----------------------------------------------------
  // カメラ起動チェック
  pinMode(POWER_GPIO_NUM, OUTPUT);
  digitalWrite(POWER_GPIO_NUM, LOW);
  if (CameraBegin()) {
    printlnStr(" OK : Camera");
  } else {
    printlnStr(" NG : Camera init failed");
    delay(1500);
    ESP.restart();
  }
  delay(500);

  // ----------------------------------------------------
  // SDカード読み書きチェック
  printlnStr(" OK : SD-Read");
  delay(500);
  printlnStr(" OK : SD-Write");
  delay(500);

  // ----------------------------------------------------
  // 初期化完了
  printlnStr(" Complete Settings");
  delay(500);
  printlnStr("\n Enjoy Photo Life!!");
  delay(2000);

}

// ----------------------------------------------------
// loop
void loop()
{
  int16_t encoder_value = scroll.getEncoderValue();
  bool btn_stauts       = scroll.getButtonStatus();

  if (btn_stauts) {
    if (!PRESSED_FLG) {
      PRESSED_FLG = true;
      printlnStr("BTN PRESSED");
      printlnInt(encoder_value);
    }
  } else {
    PRESSED_FLG = false;
  }

  display.setBrightness(160);
  // カメラからフレームを取得して表示
  if (CameraGet()) {
    canvas.pushImage(0, -16, 240, 176, (uint16_t*)fb->buf);  // (x, y, w, h, *data)
    canvas.pushSprite(&display, 0, 0);
    CameraFree();  // 取得したフレームを解放
  }

  delay(5);
}



// 以下、A-Utaさんコード========================================================================

// /**************************************************
//  * ESPNowCam video Transmitter
//  * by @hpsaturn Copyright (C) 2024
//  * This file is part ESPNowCam project:
//  * https://github.com/hpsaturn/ESPNowCam
//  - AtomS3RCam
//  https://github.com/m5stack/M5AtomS3/blob/main/examples/Basics/camera/camera.ino
//  https://github.com/m5stack/M5AtomS3/blob/main/examples/Basics/camera/camera_pins.h
// **************************************************/

// #include <Arduino.h>
// #include <esp_camera.h>
// // #include <Utils.h>

// camera_fb_t* fb;

// bool has_psram = false;

// #define POWER_GPIO_NUM 18

// // Please change this to your Camera pins:
// camera_config_t camera_config = {
//     .pin_pwdn     = -1,
//     .pin_reset    = -1,
//     .pin_xclk     = 21,
//     .pin_sscb_sda = 12,
//     .pin_sscb_scl = 9,
//     .pin_d7       = 13,
//     .pin_d6       = 11,
//     .pin_d5       = 17,
//     .pin_d4       = 4,
//     .pin_d3       = 48,
//     .pin_d2       = 46,
//     .pin_d1       = 42,
//     .pin_d0       = 3,

//     .pin_vsync = 10,
//     .pin_href  = 14,
//     .pin_pclk  = 40,
    
//     .xclk_freq_hz = 20000000,
//     .ledc_timer   = LEDC_TIMER_0,
//     .ledc_channel = LEDC_CHANNEL_0,

//     .pixel_format  = PIXFORMAT_RGB565,
//     .frame_size    = FRAMESIZE_QQVGA,
//     // FRAMESIZE_96X96,    // 96x96 - OK
//     // FRAMESIZE_QQVGA,    // 160x120 - OK
//     // FRAMESIZE_QCIF,     // 176x144 - OK
//     // FRAMESIZE_HQVGA,    // 240x176 - OK
//     // FRAMESIZE_240X240,  // 240x240 - OK
//     // FRAMESIZE_QVGA,     // 320x240 - OK

//     .jpeg_quality  = 0,
//     .fb_count      = 2,
//     .fb_location   = CAMERA_FB_IN_PSRAM,
//     .grab_mode     = CAMERA_GRAB_LATEST,
//     .sccb_i2c_port = 0,
// };

// bool CameraBegin() {
//   esp_err_t err = esp_camera_init(&camera_config);
//   if (err != ESP_OK) {
//     return false;
//   }

//   // Add
//   sensor_t *s = esp_camera_sensor_get();
//   s->set_hmirror(s, 1);        // 左右反転
//   s->set_vflip(s, 1); //上下反転 0無効 1有効

//   //カメラ追加設定
//   // sensor_t * s = esp_camera_sensor_get();
//   // s->set_hmirror(s, 1); //左右反転 0無効 1有効
//   // s->set_vflip(s, 1); //上下反転 0無効 1有効
//   // s->set_colorbar(s, 1); //カラーバー 0無効 1有効
//   // s->set_brightness(s, 1);  // up the brightness just a bit
//   // s->set_saturation(s, 0);  // lower the saturation

//   return true;
// }

// bool CameraGet() {
//   fb = esp_camera_fb_get();
//   if (!fb) {
//     return false;
//   }
//   return true;
// }

// bool CameraFree() {
//   if (fb) {
//     esp_camera_fb_return(fb);
//     return true;
//   }
//   return false;
// }

// void processFrame() {
//   if (CameraGet()) {
//     if (has_psram) {
//       uint8_t *out_jpg = NULL;
//       size_t out_jpg_len = 0;
//       frame2jpg(fb, 12, &out_jpg, &out_jpg_len);
//       // radio.sendData(out_jpg, out_jpg_len);
//       free(out_jpg);
//     }
//     else{
//       // radio.sendData(fb->buf, fb->len);
//       delay(30); // ==> weird delay for cameras without PSRAM
//     }
//     // printFPS("CAM:");
//     CameraFree();
//   }
// }

// void setup() {
//   Serial.begin(115200);

//   // add - これが無いと動かなかった
//   pinMode(POWER_GPIO_NUM, OUTPUT);
//   digitalWrite(POWER_GPIO_NUM, LOW);
//   delay(500);  

//   delay(1000); // only for debugging 

//   if(psramFound()){
//     has_psram = true;
//     size_t psram_size = esp_spiram_get_size() / 1048576;
//     Serial.printf("PSRAM size: %dMb\r\n", psram_size);
//     // suggested config with PSRAM
//     camera_config.pixel_format = PIXFORMAT_RGB565;
//     camera_config.fb_location = CAMERA_FB_IN_PSRAM;
//     camera_config.fb_count = 2;
//   }
//   else{
//     Serial.println("PSRAM not found! Basic framebuffer setup.");
//   }
  
//   // radio.init();

//   if (!CameraBegin()) {
//     Serial.println("Camera Init Fail");
//     delay(1000);
//     ESP.restart();
//   }
//   delay(500);
// }

// void loop() {
//   processFrame();
// }

