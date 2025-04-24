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
void displayStr(String str) {
  display.drawCenterString(str, 120, 60);
  Serial.println(str);
}

// ----------------------------------------------------
// 最大8色のカラーパレット デフォルト
int maxPaletteSize = 9;
uint32_t ColorPalettes[9][8] = {
  { // パレット0 slso8
    0x0D2B45, 0x203C56, 0x544E68, 0x8D697A, 0xD08159, 0xFFAA5E, 0xFFD4A3, 0xFFECD6 },
  { // パレット1 都市伝説解体センター風
    0x000000, 0x000B22, 0x112B43, 0x437290, 0x437290, 0xE0D8D1, 0xE0D8D1, 0xFFFFFF },
  { // パレット2 ファミレスを享受せよ風
    0x010101, 0x33669F, 0x33669F, 0x33669F, 0x498DB7, 0x498DB7, 0xFBE379, 0xFBE379 },
  { // パレット3 gothic-bit
    0x0E0E12, 0x1A1A24, 0x333346, 0x535373, 0x8080A4, 0xA6A6BF, 0xC1C1D2, 0xE6E6EC },
  { // パレット4 noire-truth
    0x1E1C32, 0x1E1C32, 0x1E1C32, 0x1E1C32, 0xC6BAAC, 0xC6BAAC, 0xC6BAAC, 0xC6BAAC },
  { // パレット5 2BIT DEMIBOY
    0x252525, 0x252525, 0x4B564D, 0x4B564D, 0x9AA57C, 0x9AA57C, 0xE0E9C4, 0xE0E9C4 },
  { // パレット6 deep-maze
    0x001D2A, 0x085562, 0x009A98, 0x00BE91, 0x38D88E, 0x9AF089, 0xF2FF66, 0xF2FF66 },
  { // パレット7 night-rain
    0x000000, 0x012036, 0x3A7BAA, 0x7D8FAE, 0xA1B4C1, 0xF0B9B9, 0xFFD159, 0xFFFFFF },
  { // パレット8 スペア用
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000 },
};

// カラーパレット値確認(デバッグ用、シリアル出力のみ)
void checkPalettes() {
  for (int pi = 0; pi < maxPaletteSize; ++pi) {
    Serial.printf("%X,%X,%X,%X,%X,%X,%X,%X\n", ColorPalettes[pi][0], ColorPalettes[pi][1], ColorPalettes[pi][2], ColorPalettes[pi][3], ColorPalettes[pi][4], ColorPalettes[pi][5], ColorPalettes[pi][6], ColorPalettes[pi][7]);
  }
}

// ----------------------------------------------------
// SDカード関連
#include <SPI.h>
#include <SD.h>

// SDカードにカラーパレットファイルがあれば読み込む
// src/フォルダにサンプルを置いてあるため使用する場合は事前にSDカードに保存しておく
// サンプルの設定自体は ColorPalettes の定義と同様
bool loadPaletteFromSD(String filename) {

  // SDカード内のファイルからカラーパレットを読み込む
  File file = SD.open(filename, FILE_READ);
  if (!file) {
    // ファイルオープン失敗
    return false;
  }

  // 8パターンのカラーパレットを順に読み込みColorPalettes[paletteIndex][colorCode]にセット
  int paletteIndex = 0;
  while (file.available() && paletteIndex < maxPaletteSize) {
    String line = file.readStringUntil('\n');
    line.trim();

    if (line.startsWith("#") || line.length() == 0) {
      // コメント行（#始まり）・空行はスキップ
      continue;
    }

    uint32_t colors[8];
    int colorCount = 0;
    int lastIndex = 0;

    while (colorCount < 8) {
      int commaIndex = line.indexOf(',', lastIndex);
      String colorStr;

      if (commaIndex == -1) {
        colorStr = line.substring(lastIndex);
      } else {
        colorStr = line.substring(lastIndex, commaIndex);
        lastIndex = commaIndex + 1;
      }

      colorStr.trim();
      char hexBuffer[16];
      colorStr.toCharArray(hexBuffer, sizeof(hexBuffer));
      uint32_t color = strtoul(hexBuffer, NULL, 0);  // ← base = 0 により0x(16進)を自動判別

      colors[colorCount++] = color;

      if (commaIndex == -1) {
        break;
      }
    }
    if (colorCount != 8) {
      // カラーパレット定義が8色ではない不正な色数として読み込み失敗とする
      file.close();
      return false;
    }

    for (int i = 0; i < 8; ++i) {
      ColorPalettes[paletteIndex][i] = colors[i];
    }

    paletteIndex++;
  }
  file.close();

  // カラーパレット定義の上書き成功またはデフォルト定義のままの場合trueを返す
  return true;

}

// ファイルの読み書きテスト用関数
bool testReadWriteSD(){
  // ファイル読み書きテスト用ファイル名
  const char* filename = "/_writeTest.txt";

  // ファイル作成と書き込み
  File writeFile = SD.open(filename, FILE_WRITE);
  if (writeFile) {
    writeFile.println("test");
    writeFile.close();
    Serial.println("ファイルに書き込みました。");
  } else {
    Serial.println("ファイルの作成または書き込みに失敗しました。");
    return false;
  }

  // ファイル読み込み
  File readFile = SD.open(filename);
  if (readFile) {
    String content = readFile.readStringUntil('\n');
    content.trim();
    Serial.print("読み込んだ文字列: ");
    Serial.println(content);
    readFile.close();

    // 読み込んだ内容が "test" ならファイルの読み書き成功としてテスト用ファイルも削除
    if (content == "test") {
      if (SD.remove(filename)) {
        Serial.println("ファイルを削除しました。");
      } else {
        Serial.println("ファイルの削除に失敗しました。");
        return false;
      }
    } else {
      Serial.println("書き込んだ内容と読み込んだ内容が一致しないため、ファイルを削除しません。");
      return false;
    }
  } else {
    Serial.println("ファイルの読み込みに失敗しました。");
    return false;
  }

  // 削除確認
  if (!SD.exists(filename)) {
    Serial.println("ファイルが正常に削除されています。");
  } else {
    Serial.println("ファイルがまだ存在しています。");
    return false;
  }

  return true;
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
// カメラ画像保存関連
char filename[64];                      // SDカード保存ファイル名
int filecounter = 1;                    // ファイルカウンターは電源を入れるたびにリセットされる　極稀にファイル名が被るかも
int selectedPalettelndex = -1;          // 選択したパレットの番号（0-9：個別指定, -1：未指定（デフォルト））
uint8_t graydata[240 * 176];            // 輝度情報保存
uint32_t btnOnTime = 0;                 // キースイッチを操作した時間

// カメラが撮像したオリジナル画像を保存
bool saveToSD_OriginalBMP() {
  sprintf(filename, "/%010d_%04d_Original.bmp", btnOnTime, filecounter);
  File file = SD.open(filename, "w");
  if (file) {
    uint8_t* out_bmp = NULL;
    size_t out_bmp_len = 0;
    frame2bmp(fb, &out_bmp, &out_bmp_len);
    file.write(out_bmp, out_bmp_len);
    file.close();
    free(out_bmp);
  } else {
    // ファイルの作成・オープンに失敗した場合falseを返す
    return false;
  }
  return true;
}

// LCDディスプレイに表示された画像を保存
bool saveToSD_DisplayBMP() {
  sprintf(filename, "/%010d_%04d_OLED.bmp", btnOnTime, filecounter);
  File file = SD.open(filename, "w");
  if (file) {
    int width = display.width();
    int height = display.height();
    int rowSize = (3 * width + 3) & ~3;

    lgfx::bitmap_header_t bmpheader;
    bmpheader.bfType = 0x4D42;
    bmpheader.bfSize = rowSize * height + sizeof(bmpheader);
    bmpheader.bfOffBits = sizeof(bmpheader);
    bmpheader.biSize = 40;
    bmpheader.biWidth = width;
    bmpheader.biHeight = height;
    bmpheader.biPlanes = 1;
    bmpheader.biBitCount = 24;
    bmpheader.biCompression = 0;
    bmpheader.biSizeImage = 0;
    bmpheader.biXPelsPerMeter = 2835;
    bmpheader.biYPelsPerMeter = 2835;
    bmpheader.biClrUsed = 0;
    bmpheader.biClrImportant = 0;

    file.write((std::uint8_t*)&bmpheader, sizeof(bmpheader));
    std::uint8_t buffer[rowSize];
    memset(&buffer[rowSize - 4], 0, 4);
    for (int y = height - 1; y >= 0; y--) {
      display.readRect(0, y, width, 1, (lgfx::rgb888_t*)buffer);
      file.write(buffer, rowSize);
    }
    file.close();
  } else {
    // ファイルの作成・オープンに失敗した場合falseを返す
    return false;
  }
  return true;
}

// カラーパレットの色調に変換した画像を保存
bool saveToSD_ConvertBMP() {
  int max_index;
  int palettelndex = 0;
  if (selectedPalettelndex == -1) {
    // 全カラーパレット保存モード（デフォルト）
    max_index = maxPaletteSize;
  } else {
    // 指定パレットのみ保存モード
    max_index = 1;
    palettelndex = selectedPalettelndex;
  }

  for (int i = 0; i < max_index; i++) {

    if (selectedPalettelndex == -1) {
      palettelndex = i;
    }

    sprintf(filename, "/%010d_%04d_palette%01d.bmp", btnOnTime, filecounter, i);
    File file = SD.open(filename, "w");
    if (file) {
      int width = fb->width;
      int height = fb->height;
      int rowSize = (3 * width + 3) & ~3;
  
      lgfx::bitmap_header_t bmpheader;
      bmpheader.bfType = 0x4D42;
      bmpheader.bfSize = rowSize * height + sizeof(bmpheader);
      bmpheader.bfOffBits = sizeof(bmpheader);
      bmpheader.biSize = 40;
      bmpheader.biWidth = width;
      bmpheader.biHeight = height;
      bmpheader.biPlanes = 1;
      bmpheader.biBitCount = 24;
      bmpheader.biCompression = 0;
      bmpheader.biSizeImage = 0;
      bmpheader.biXPelsPerMeter = 2835;
      bmpheader.biYPelsPerMeter = 2835;
      bmpheader.biClrUsed = 0;
      bmpheader.biClrImportant = 0;
  
      file.write((std::uint8_t*)&bmpheader, sizeof(bmpheader));
      std::uint8_t buffer[rowSize];
      memset(&buffer[rowSize - 4], 0, 4);
      for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
  
          // グレイデータを読み出す
          int i_gray = y * width + x;
          uint8_t gray = graydata[i_gray];
  
          // カラーパレットから色を取得
          uint32_t newColor = ColorPalettes[palettelndex][gray];
          uint8_t r = (newColor >> 16) & 0xFF;
          uint8_t g = (newColor >> 8) & 0xFF;
          uint8_t b = newColor & 0xFF;
  
          // バッファに書き込み BGRの順になる
          int i_buffer = x * 3;
          buffer[i_buffer] = b;
          buffer[i_buffer + 1] = g;
          buffer[i_buffer + 2] = r;
        }
        file.write(buffer, rowSize);
      }
      file.close();
    } else {
      // ファイルの作成・オープンに失敗した場合falseを返す(処理を中断)
      return false;
    }
  }  
  return true;
}


// 輝度情報の保存
void saveGraylevel_fb() {
  uint8_t* fb_data = fb->buf;
  int width = fb->width;
  int height = fb->height;
  int i = 0;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < (width * 2); x = x + 2) {

      // 各ピクセルの色を取得
      uint32_t rgb565Color = (fb_data[y * width * 2 + x] << 8) | fb_data[y * width * 2 + x + 1];

      // RGB565からRGB888へ変換
      // ★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
      // オリジナルではここでcanvas0を指定している
      // ★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
      uint32_t rgb888Color = canvas.color16to24(rgb565Color);
      uint8_t r = (rgb888Color >> 16) & 0xFF;
      uint8_t g = (rgb888Color >> 8) & 0xFF;
      uint8_t b = rgb888Color & 0xFF;

      // 輝度の計算 BT.709の係数を使用
      uint16_t luminance = (uint16_t)(0.2126 * r + 0.7152 * g + 0.0722 * b);

      // 輝度を16階調のグレースケールに変換
      uint8_t grayLevel = luminance / 32;  // 256/32 = 8

      // 輝度情報を保存
      graydata[i] = grayLevel;
      i++;
    }
  }
}

// ----------------------------------------------------
// 撮影モード切替関連
// ★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
bool modeChange = false;
void selectMode(int encoderValue) {

  // selectedPalettelndex = encoderValue;
  // if (encoderValue == -1) {
  //   // モード切替トグル

  // } else if (encoderValue < -1) {
  //   // エンコーダー値を強制的に -1に変更

  // } else {
  //   selectedPalettelndex = -1;
  // }

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
  display.setTextSize(1.4);
  canvas.createSprite(240, 176);
  // canvas.createSprite(240, 176);  // カメラ画像表示
  // canvas.createSprite(240, 135);  // 変換画像表示
  printlnStr(" OK : UnitLCD");
  delay(1000);

  // ----------------------------------------------------
  // ユニットスクロール初期化
  scroll.begin(&Wire, SCROLL_ADDR, PIN_SDA, PIN_SCL, 400000U);
  printlnStr(" OK : UnitScroll");
  delay(1000);

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
  delay(1000);

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
  delay(1000);

  // ----------------------------------------------------
  // SDカードチェック
  SPI.begin(7, 8, 6, -1);
  if (SD.begin(15, SPI, 80000000)) {
    // SDカード読み書きテスト
    if (testReadWriteSD()) {
      printlnStr(" OK : SD-Card Read & Write");
    } else {
      printlnStr(" NG : SD-Card Read or Write failed");
      delay(1500);
      ESP.restart();
    }
    delay(1000);

    // カラーパレット設定
    String filename = "/ColorPalettes.txt";
    if (SD.exists(filename)) {
      // ColorPalettes.txtファイルが存在する場合、そのカラーパレットの値を使用する
      if (loadPaletteFromSD(filename)) {
        printlnStr(" OK : Color Palette Setting");
        printlnStr(" -------------------------");
        printlnStr(" Use ColorPalettes.txt");
        printlnStr(" -------------------------");
        } else {
        printlnStr(" NG : Color Palette Setting failed");
        delay(1500);
        ESP.restart();
      }
    } else {
      // ColorPalettes.txtファイルがない場合、デフォルトのカラーパレットの値を使用する
      printlnStr(" -------------------------");
      printlnStr(" No ColorPalettes.txt");
      printlnStr(" Use Default Color Palette");
      printlnStr(" -------------------------");
    }
    // パレットデータ確認用(シリアル出力)
    checkPalettes();
    delay(100);
  } else {
    printlnStr(" NG : SD-Card Init failed");
    delay(1500);
    ESP.restart();
  }
  SD.end();  // 一旦ENDしておく
  delay(1000);

  // ----------------------------------------------------
  // ディスプレイされるフォントサイズを変更
  display.setTextSize(1.8);
  // 初期化完了
  printlnStr(" Complete Settings");
  delay(1000);
  printlnStr("\n Enjoy Photo Life!!");
  delay(2000);

}

// ----------------------------------------------------
// loop
signed short int newEncoderValue = 0;   // エンコーダの値　新
signed short int lastEncoderValue = 0;  // エンコーダの値　旧

void loop()
{
  newEncoderValue = scroll.getEncoderValue();
  bool btn_stauts       = scroll.getButtonStatus();

  int diff = newEncoderValue - lastEncoderValue;

  if (btn_stauts) {
    if (!PRESSED_FLG) {
      PRESSED_FLG = true;
      // printlnStr("BTN PRESSED");
      // printlnInt(newEncoderValue);

      // ボタン操作した時間
      btnOnTime = millis();

      // 撮影
      CameraGet();

      SD.end();  // 念のため一旦END
      delay(100);
      SD.begin(15, SPI, 80000000);  // 保存失敗するときは速度を下げる

      // LCDディスプレイに表示された画像を保存
      // カメラが撮像した画像を保存
      if (saveToSD_OriginalBMP() && saveToSD_DisplayBMP()) {
        printlnStr("Original Image Save OK");
        display.print("\n");
        delay(100);
      } else {
        printlnStr("Original Image Save Failed");
        delay(100);
      }

      // 輝度情報の保存
      saveGraylevel_fb();

      // カラーパレットの色調に変換した画像を保存
      if (saveToSD_ConvertBMP()) {
        printlnStr("Now Converting...");
        display.print("\n");
        delay(1500);
      } else {
        printlnStr("Converte Failed");
        delay(1000);
      }

      // フレームバッファを解放
      CameraFree();
      SD.end();
      printlnStr("Finish Converte");
      delay(2000);
      // ファイル連番を更新
      filecounter++;
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

