#include <M5Unified.h>

// ======================================================================
// ユニットスクロールサンプル

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
// Common関数
void printlnStr(String str) {
  display.println(str);
  Serial.println(str);
}

void printlnInt(int16_t i) {
  display.println(i);
  Serial.println(i);
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
  canvas.createSprite(240, 135);
  // canvas.createSprite(240, 176);  // カメラ画像表示
  // canvas.createSprite(240, 135);  // 変換画像表示
  printlnStr(" OK UnitLCD");
  delay(500);

  // ----------------------------------------------------
  // ユニットスクロール初期化
  scroll.begin(&Wire, SCROLL_ADDR, PIN_SDA, PIN_SCL, 400000U);
  printlnStr(" OK UnitScroll");
  delay(500);

  // ----------------------------------------------------
  // PSRAMチェック
  printlnStr(" OK PSRAM");
  delay(500);

  // ----------------------------------------------------
  // カメラ起動チェック
  printlnStr(" OK Camera");
  delay(500);

  // ----------------------------------------------------
  // SDカード読み書きチェック
  printlnStr(" OK SD-Read");
  delay(500);
  printlnStr(" OK SD-Write");
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

  delay(20);
}
