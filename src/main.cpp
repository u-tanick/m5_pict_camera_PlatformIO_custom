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
void printlnStr(String str)
{
  display.println(str);
  Serial.println(str);
}
void printlnInt(int16_t i)
{
  display.println(i);
  Serial.println(i);
}
void displayStr(String str)
{
  display.drawCenterString(str, 120, 60);
  Serial.println(str);
}

// ----------------------------------------------------
// 最大8色のカラーパレット デフォルト
int maxPaletteSize = 8;
uint32_t ColorPalettes[8][8] = {
    {// パレット0 slso8
     0x0D2B45, 0x203C56, 0x544E68, 0x8D697A, 0xD08159, 0xFFAA5E, 0xFFD4A3, 0xFFECD6},
    {// パレット1 都市伝説解体センター風
     0x000000, 0x000B22, 0x112B43, 0x437290, 0x437290, 0xE0D8D1, 0xE0D8D1, 0xFFFFFF},
    {// パレット2 ファミレスを享受せよ風
     0x010101, 0x33669F, 0x33669F, 0x33669F, 0x498DB7, 0x498DB7, 0xFBE379, 0xFBE379},
    {// パレット3 gothic-bit
     0x0E0E12, 0x1A1A24, 0x333346, 0x535373, 0x8080A4, 0xA6A6BF, 0xC1C1D2, 0xE6E6EC},
    {// パレット4 noire-truth
     0x1E1C32, 0x1E1C32, 0x1E1C32, 0x1E1C32, 0xC6BAAC, 0xC6BAAC, 0xC6BAAC, 0xC6BAAC},
    {// パレット5 2BIT DEMIBOY
     0x252525, 0x252525, 0x4B564D, 0x4B564D, 0x9AA57C, 0x9AA57C, 0xE0E9C4, 0xE0E9C4},
    {// パレット6 deep-maze
     0x001D2A, 0x085562, 0x009A98, 0x00BE91, 0x38D88E, 0x9AF089, 0xF2FF66, 0xF2FF66},
    {// パレット7 night-rain
     0x000000, 0x012036, 0x3A7BAA, 0x7D8FAE, 0xA1B4C1, 0xF0B9B9, 0xFFD159, 0xFFFFFF},
};

// カラーパレット値確認(デバッグ用、シリアル出力のみ)
void checkPalettes()
{
  for (int pi = 0; pi < maxPaletteSize; ++pi)
  {
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
bool loadPaletteFromSD(String filename)
{

  // SDカード内のファイルからカラーパレットを読み込む
  File file = SD.open(filename, FILE_READ);
  if (!file)
  {
    // ファイルオープン失敗
    return false;
  }

  // 8パターンのカラーパレットを順に読み込みColorPalettes[paletteIndex][colorCode]にセット
  int paletteIndex = 0;
  while (file.available() && paletteIndex < maxPaletteSize)
  {
    String line = file.readStringUntil('\n');
    line.trim();

    if (line.startsWith("#") || line.length() == 0)
    {
      // コメント行（#始まり）・空行はスキップ
      continue;
    }

    uint32_t colors[8];
    int colorCount = 0;
    int lastIndex = 0;

    while (colorCount < 8)
    {
      int commaIndex = line.indexOf(',', lastIndex);
      String colorStr;

      if (commaIndex == -1)
      {
        colorStr = line.substring(lastIndex);
      }
      else
      {
        colorStr = line.substring(lastIndex, commaIndex);
        lastIndex = commaIndex + 1;
      }

      colorStr.trim();
      char hexBuffer[16];
      colorStr.toCharArray(hexBuffer, sizeof(hexBuffer));
      uint32_t color = strtoul(hexBuffer, NULL, 0); // ← base = 0 により0x(16進)を自動判別

      colors[colorCount++] = color;

      if (commaIndex == -1)
      {
        break;
      }
    }
    if (colorCount != 8)
    {
      // カラーパレット定義が8色ではない不正な色数として読み込み失敗とする
      file.close();
      return false;
    }

    for (int i = 0; i < 8; ++i)
    {
      ColorPalettes[paletteIndex][i] = colors[i];
    }

    paletteIndex++;
  }
  file.close();

  // カラーパレット定義の上書き成功またはデフォルト定義のままの場合trueを返す
  return true;
}

// ファイルの読み書きテスト用関数
bool testReadWriteSD()
{
  // ファイル読み書きテスト用ファイル名
  const char *filename = "/_writeTest.txt";

  // ファイル作成と書き込み
  File writeFile = SD.open(filename, FILE_WRITE);
  if (writeFile)
  {
    writeFile.println("test");
    writeFile.close();
    Serial.println("ファイルに書き込みました。");
  }
  else
  {
    Serial.println("ファイルの作成または書き込みに失敗しました。");
    return false;
  }

  // ファイル読み込み
  File readFile = SD.open(filename);
  if (readFile)
  {
    String content = readFile.readStringUntil('\n');
    content.trim();
    Serial.print("読み込んだ文字列: ");
    Serial.println(content);
    readFile.close();

    // 読み込んだ内容が "test" ならファイルの読み書き成功としてテスト用ファイルも削除
    if (content == "test")
    {
      if (SD.remove(filename))
      {
        Serial.println("ファイルを削除しました。");
      }
      else
      {
        Serial.println("ファイルの削除に失敗しました。");
        return false;
      }
    }
    else
    {
      Serial.println("書き込んだ内容と読み込んだ内容が一致しないため、ファイルを削除しません。");
      return false;
    }
  }
  else
  {
    Serial.println("ファイルの読み込みに失敗しました。");
    return false;
  }

  // 削除確認
  if (!SD.exists(filename))
  {
    Serial.println("ファイルが正常に削除されています。");
  }
  else
  {
    Serial.println("ファイルがまだ存在しています。");
    return false;
  }

  return true;
}

// ----------------------------------------------------
// カメラ・PSRAM関連
#include <esp_camera.h>
#define POWER_GPIO_NUM 18
camera_fb_t *fb;
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

bool CameraBegin()
{
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    printlnStr(" Error Code");
    printlnInt(err);
    return false;
  }

  // カメラ追加設定
  sensor_t *s = esp_camera_sensor_get();

  // AtomS3R Cam のときはこちら
  s->set_hmirror(s, 1); // 左右反転 0無効 1有効
  s->set_vflip(s, 1);   // 上下反転 0無効 1有効

  // AtomS3R M12 のときはこちら
  // s->set_lenc(s, 1);     // レンズ補正？ 効いてるか微妙
  // s->set_hmirror(s, 1);  // 左右反転 0無効 1有効
  // s->set_vflip(s, 0);    // 上下反転 0無効 1有効

  return true;
}

bool CameraGet()
{
  fb = esp_camera_fb_get();
  if (!fb)
  {
    return false;
  }
  return true;
}

bool CameraFree()
{
  if (fb)
  {
    esp_camera_fb_return(fb);
    return true;
  }
  return false;
}

// ----------------------------------------------------
// カメラ画像保存関連
char filename_bmp[64];             // SDカード保存ファイル名
int filecounter = 1;           // ファイルカウンターは電源を入れるたびにリセットされる　極稀にファイル名が被るかも
int selectedPalettelndex = -1; // 選択したパレットの番号（0-8：個別指定, -1：未指定（デフォルト））
uint8_t graydata[240 * 176];   // 輝度情報保存
uint32_t btnOnTime = 0;        // キースイッチを操作した時間

// カメラが撮像したオリジナル画像を保存
bool saveToSD_OriginalBMP()
{
  sprintf(filename_bmp, "/%010d_%04d_Original.bmp", btnOnTime, filecounter);
  File file_bmp = SD.open(filename_bmp, "w");
  if (file_bmp)
  {
    uint8_t *out_bmp = NULL;
    size_t out_bmp_len = 0;
    frame2bmp(fb, &out_bmp, &out_bmp_len);
    file_bmp.write(out_bmp, out_bmp_len);
    file_bmp.close();
    free(out_bmp);
  }
  else
  {
    // ファイルの作成・オープンに失敗した場合falseを返す
    return false;
  }
  return true;
}

// LCDディスプレイに表示された画像を保存
bool saveToSD_DisplayBMP()
{
  sprintf(filename_bmp, "/%010d_%04d_LCD.bmp", btnOnTime, filecounter);
  File file_bmp = SD.open(filename_bmp, "w");
  if (file_bmp)
  {
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

    file_bmp.write((std::uint8_t *)&bmpheader, sizeof(bmpheader));
    std::uint8_t buffer[rowSize];
    memset(&buffer[rowSize - 4], 0, 4);
    for (int y = height - 1; y >= 0; y--)
    {
      display.readRect(0, y, width, 1, (lgfx::rgb888_t *)buffer);
      file_bmp.write(buffer, rowSize);
    }
    file_bmp.close();
  }
  else
  {
    // ファイルの作成・オープンに失敗した場合falseを返す
    return false;
  }
  return true;
}

// bool saveToSD_BMP()
// {
//   sprintf(filename_bmp, "/%010d_%04d_Original.bmp", btnOnTime, filecounter);
//   File file_bmp = SD.open(filename_bmp, "w");
//   sprintf(filename_bmp, "/%010d_%04d_LCD.bmp", btnOnTime, filecounter);
//   File file_bmp = SD.open(filename_bmp, "w");




//   return true;
// }

// カラーパレットの色調に変換した画像を保存
bool saveToSD_ConvertBMP()
{
  int max_index;
  int palettelndex = 0;
  if (selectedPalettelndex == -1)
  {
    // 全カラーパレット保存モード（デフォルト）
    max_index = maxPaletteSize;
  }
  else
  {
    // 指定パレットのみ保存モード
    max_index = 1;
    palettelndex = selectedPalettelndex;
  }

  for (int i = 0; i < max_index; i++)
  {

    if (selectedPalettelndex == -1)
    {
      palettelndex = i;
    }

    sprintf(filename_bmp, "/%010d_%04d_palette%01d.bmp", btnOnTime, filecounter, i);
    File file_bmp = SD.open(filename_bmp, "w");
    if (file_bmp)
    {
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

      file_bmp.write((std::uint8_t *)&bmpheader, sizeof(bmpheader));
      std::uint8_t buffer[rowSize];
      memset(&buffer[rowSize - 4], 0, 4);
      for (int y = height - 1; y >= 0; y--)
      {
        for (int x = 0; x < width; x++)
        {

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
        file_bmp.write(buffer, rowSize);
      }
      file_bmp.close();
    }
    else
    {
      // ファイルの作成・オープンに失敗した場合falseを返す(処理を中断)
      return false;
    }
  }
  return true;
}

// 輝度情報の保存
void saveGraylevel_fb()
{
  uint8_t *fb_data = fb->buf;
  int width = fb->width;
  int height = fb->height;
  int i = 0;

  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < (width * 2); x = x + 2)
    {

      // 各ピクセルの色を取得
      uint32_t rgb565Color = (fb_data[y * width * 2 + x] << 8) | fb_data[y * width * 2 + x + 1];

      // RGB565からRGB888へ変換
      uint32_t rgb888Color = canvas.color16to24(rgb565Color);
      uint8_t r = (rgb888Color >> 16) & 0xFF;
      uint8_t g = (rgb888Color >> 8) & 0xFF;
      uint8_t b = rgb888Color & 0xFF;

      // 輝度の計算 BT.709の係数を使用
      uint16_t luminance = (uint16_t)(0.2126 * r + 0.7152 * g + 0.0722 * b);

      // 輝度を16階調のグレースケールに変換
      uint8_t grayLevel = luminance / 32; // 256/32 = 8

      // 輝度情報を保存
      graydata[i] = grayLevel;
      i++;
    }
  }
}

// ----------------------------------------------------
// カメラ画像を切り出してズーム表示する関数
void pushImageZoom(M5Canvas *canvas, int dst_x, int dst_y, int dst_w, int dst_h,
                   const uint16_t *src16, int src_w, int src_h,
                   int crop_x, int crop_y, int crop_w, int crop_h)
{
  const uint8_t *src = (const uint8_t *)src16; // ここでbyte配列として扱う！

  for (int y = 0; y < dst_h; ++y)
  {
    for (int x = 0; x < dst_w; ++x)
    {
      int src_x = crop_x + (x * crop_w) / dst_w;
      int src_y = crop_y + (y * crop_h) / dst_h;
      if (src_x >= 0 && src_x < src_w && src_y >= 0 && src_y < src_h)
      {
        int index = (src_y * src_w + src_x) * 2;
        uint16_t color565 = ((uint16_t)src[index] << 8) | src[index + 1];
        uint32_t color888 = canvas->color16to24(color565);
        canvas->drawPixel(dst_x + x, dst_y + y, color888);
      }
    }
  }
}

// ----------------------------------------------------
// Jpgファイル名のリストを作成する
#include <vector>

constexpr size_t MAX_FILES = 1500;       // 想定する最大jpgファイル数 1750あたりで確保領域がバッファオーバーフロー
constexpr size_t MAX_FILENAME_LEN = 128; // ファイル名の最大長（先頭に/を付けるのでちょっと余裕もたせる）

char fileNameStorage[MAX_FILES][MAX_FILENAME_LEN]; // ファイル名バッファ
std::vector<char*> jpgFileNames;                   // Jpgファイルだけを入れるリスト

// ファイル名が.jpgで終わっているかを判定する関数
bool isJpgFile(const char* filename) {
  size_t len = strlen(filename);
  return (len >= 4) && 
         (strcasecmp(filename + len - 4, ".bmp") == 0); // 大文字小文字を無視して比較

         // ★暫定　いったん実態としてゃbmpのリスト。あとで拡張子をjpgに変える
}

size_t MAX_FILE_NUMBER = 0;

void getAllJpgFileNames() {

  Serial.println("getAllJpgFileNames");

  jpgFileNames.clear();
  jpgFileNames.reserve(MAX_FILES);

  size_t fileCount = 0;

  SD.begin(15, SPI, 80000000);

  File root = SD.open("/");
  if (!root) {
    Serial.println("ルートディレクトリを開けませんでした！");
    return;
  }

  File entry = root.openNextFile();
  Serial.println(entry);
  while (entry && fileCount < MAX_FILES) {
    if (!entry.isDirectory()) {
      const char* filename = entry.name();
      Serial.println(filename);
      if (isJpgFile(filename)) {
        // 先頭に '/' を付加して保存
        snprintf(fileNameStorage[fileCount], MAX_FILENAME_LEN, "/%s", filename);
        jpgFileNames.push_back(fileNameStorage[fileCount]);
        fileCount++;
      }
    }
    entry.close();
    entry = root.openNextFile();
  }
  MAX_FILE_NUMBER = fileCount;
  root.close();
  SD.end();

  // Jpgファイル名を降順にソート
  std::sort(jpgFileNames.begin(), jpgFileNames.end(), [](const char* a, const char* b) {
    return strcmp(a, b) > 0;
  });

  Serial.println("\nルートディレクトリ内のJpgファイル一覧 (降順):");
  for (const char* fileName : jpgFileNames) {
    Serial.println(fileName);
  }
  Serial.println("Jpgファイル一覧の取得とソートが完了しました。");
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
  display.setBrightness(160);
  display.setTextSize(1.4);
  canvas.setColorDepth(24); // 24bitカラー（RGB888）
  canvas.createSprite(240, 176);
  printlnStr(" OK : UnitLCD");
  delay(1000);

  // ----------------------------------------------------
  // ユニットスクロール初期化
  scroll.begin(&Wire, SCROLL_ADDR, PIN_SDA, PIN_SCL, 400000U);
  printlnStr(" OK : UnitScroll");
  delay(1000);

  // ----------------------------------------------------
  // PSRAMチェック
  if (psramFound())
  {
    camera_config.pixel_format = PIXFORMAT_RGB565;
    camera_config.fb_location = CAMERA_FB_IN_PSRAM;
    camera_config.fb_count = 2;
    printlnStr(" OK : PSRAM");
  }
  else
  {
    printlnStr(" NG : PSRAM not found");
    delay(1500);
    ESP.restart();
  }
  delay(1000);

  // ----------------------------------------------------
  // カメラ起動チェック
  pinMode(POWER_GPIO_NUM, OUTPUT);
  digitalWrite(POWER_GPIO_NUM, LOW);
  if (CameraBegin())
  {
    printlnStr(" OK : Camera");
  }
  else
  {
    printlnStr(" NG : Camera init failed");
    delay(1500);
    ESP.restart();
  }
  delay(1000);

  // ----------------------------------------------------
  // SDカードチェック
  SPI.begin(7, 8, 6, -1);
  if (SD.begin(15, SPI, 80000000))
  {
    // SDカード読み書きテスト
    if (testReadWriteSD())
    {
      printlnStr(" OK : SD-Card Read & Write");
    }
    else
    {
      printlnStr(" NG : SD-Card Read or Write failed");
      delay(1500);
      ESP.restart();
    }
    delay(1000);

    // カラーパレット設定
    String filename = "/ColorPalettes.txt";
    if (SD.exists(filename))
    {
      // ColorPalettes.txtファイルが存在する場合、そのカラーパレットの値を使用する
      if (loadPaletteFromSD(filename))
      {
        printlnStr(" OK : Color Palette Setting");
        printlnStr(" Use ColorPalettes.txt");
      }
      else
      {
        printlnStr(" NG : Color Palette Setting failed");
        delay(1500);
        ESP.restart();
      }
    }
    else
    {
      // ColorPalettes.txtファイルがない場合、デフォルトのカラーパレットの値を使用する
      printlnStr(" No ColorPalettes.txt");
      printlnStr(" Use Default Color Palette");
    }
    // パレットデータ確認用(シリアル出力)
    checkPalettes();
    delay(1000);

  }
  else
  {
    printlnStr(" NG : SD-Card Init failed");
    delay(1500);
    ESP.restart();
  }
  SD.end(); // 一旦ENDしておく
  delay(1000);

  // ----------------------------------------------------
  // ディスプレイされるフォントサイズを変更
  // 初期化完了
  printlnStr(" -------------------------");
  printlnStr(" Complete Settings \n ");
  delay(1000);
  display.setTextSize(1.8);
  printlnStr(" Enjoy Photo Life!!");
  delay(2000);
}

// ----------------------------------------------------
// loop
bool previewMode = false;
bool initLoop = true;
signed short int newEncoderValue = 0;  // エンコーダの値　新
signed short int lastEncoderValue = 0; // エンコーダの値　旧

float zoomRate = 1.0; // ズーム倍率（1.0倍スタート）

void loop()
{
  // 初回起動時のEncoderValueを必ず0にセット
  if (initLoop) {
    int16_t iv = 0;
    scroll.setEncoderValue(iv);
    initLoop = false;
  }

  newEncoderValue = scroll.getEncoderValue();
  int diff = newEncoderValue - lastEncoderValue;

  if (!previewMode) {

    Serial.println(newEncoderValue);

    // ========== 通常モード ==========
    if (newEncoderValue <= -1) {
      // EncoderValueが-1以下の場合の処理

      // -1よりも小さい場合は強制的に-1にリセット
      if (newEncoderValue < -1) {
        scroll.setEncoderValue(-1);
        newEncoderValue = -1;
      }
      // Change Preview Mode表示
      canvas.fillSprite(LIGHTGREY);
      canvas.setTextColor(BLACK);
      canvas.setTextSize(2.4);
      canvas.drawCenterString("Press Button", 120, 45);
      canvas.drawCenterString("Preview Mode", 120, 75);
      canvas.pushSprite(&display, 0, 0);

      // ボタン押したらプレビューモードに入る
      if (scroll.getButtonStatus()) {

        Serial.println("Press button");

        canvas.fillSprite(PINK);
        canvas.setTextColor(BLACK);
        canvas.drawCenterString("Preview Mode", 120, 30);
        canvas.drawCenterString("Getting Ready", 120, 60);
        canvas.drawCenterString("Please Wait", 120, 90);
        canvas.pushSprite(&display, 0, 0);

        previewMode = true;
        // 念のため EncoderValue を -1 に設定
        scroll.setEncoderValue(-1);
        newEncoderValue = -1;

        // サムネイル画像ファイル名のリストを作成
        getAllJpgFileNames();
  
        delay(500);
      }
    }
    else {
      // EncoderValueが 0 以上の場合の処理

      canvas.setTextSize(1.8);

      // 40よりも大きい場合は強制的に40にリセット
      if (newEncoderValue > 40) {
        scroll.setEncoderValue(40);
        newEncoderValue = 40;
      }

      // 最大倍率 4.0 倍（EncoderValue = 40 が最大）
      if (diff != 0) {
        zoomRate += diff * 0.1;
        if (zoomRate < 1.0) zoomRate = 1.0;
        if (zoomRate > 4.0) zoomRate = 4.0;
        Serial.printf("ZoomRate: %.1f\n", zoomRate);
      }

      if (scroll.getButtonStatus()) {
        // カメラ画像の保存
        btnOnTime = millis();
        CameraGet();
        SD.end(); // 念のため一旦END
        delay(100);
        SD.begin(15, SPI, 80000000); // 保存失敗するときは速度を下げる

        // LCDディスプレイに表示された画像を保存
        // カメラが撮像した画像を保存
//        if (saveToSD_BMP()) {
        if (saveToSD_OriginalBMP() && saveToSD_DisplayBMP()) { ★
          printlnStr(" Image Saving...");
          delay(100);
        } else {
          printlnStr(" Image Save Failed");
          delay(100);
        }
        // 輝度情報の保存
        saveGraylevel_fb();
        // カラーパレットの色調に変換した画像を保存
        if (saveToSD_ConvertBMP()) {
          printlnStr(" Pict Converting...");
          delay(1500);
        } else {
          printlnStr(" Convert Failed");
          delay(1000);
        }
        CameraFree();
        SD.end();
        printlnStr(" Save Complete !!");
        printlnStr(" ");
        delay(2500);
        filecounter++;
      }

      // カメラからフレームを取得して表示
      if (CameraGet()) {
        int src_w = fb->width;
        int src_h = fb->height;
        int crop_w = src_w / zoomRate;
        int crop_h = src_h / zoomRate;
        int crop_x = (src_w - crop_w) / 2;
        int crop_y = (src_h - crop_h) / 2;

        canvas.fillSprite(0);
        pushImageZoom(&canvas, 0, -16, 240, 176, (uint16_t*)fb->buf, src_w, src_h, crop_x, crop_y, crop_w, crop_h);
        canvas.pushSprite(&display, 0, 0);
        CameraFree();
      }
    }
  }
  else {
    // ========== プレビューモード ==========
    Serial.println("Preview Mode");
    delay(200);

    // スクロールの値によって処理を分岐
    if (newEncoderValue <= -1) {
      // EncoderValue が -1 の場合は "Preview Mode" を表示
      scroll.setEncoderValue(-1);
      newEncoderValue = -1;
      canvas.fillSprite(SKYBLUE);
      canvas.setTextColor(WHITE);
      canvas.setTextSize(2.4);
      canvas.drawCenterString("Preview Mode", 120, 45);
      canvas.drawCenterString("<- Scrol ->", 120, 75);
      canvas.pushSprite(&display, 0, 0);

    } else {
      // -1以外の場合は、ファイルリストに従って画像を表示する
      // ファイルリストが空ならエラーメッセージ表示
      if (jpgFileNames.empty()) {
        canvas.fillSprite(RED);
        canvas.setTextColor(WHITE);
        canvas.setTextSize(2.0);
        canvas.drawCenterString("No BMP files found.", 120, 60);
        canvas.pushSprite(&display, 0, 0);
      } else {
        if (newEncoderValue >= MAX_FILE_NUMBER) {
          // EncoderValueの値はファイル数の最大を上限とする
          canvas.fillSprite(YELLOW);
          canvas.setTextColor(WHITE);
          canvas.setTextSize(2.4);
          canvas.drawCenterString("Preview File End", 120, 45);
          canvas.pushSprite(&display, 0, 0);
    
          newEncoderValue = MAX_FILE_NUMBER;
          scroll.setEncoderValue(MAX_FILE_NUMBER);
        } else {
          // 現在のEncoderValueから表示すべきインデックスを決める
          int fileIndex = jpgFileNames.size() - 1 - newEncoderValue; // 降順

          // 範囲外にならないように補正
          if (fileIndex < 0) fileIndex = 0;
          if (fileIndex >= jpgFileNames.size()) fileIndex = jpgFileNames.size() - 1;
  
          Serial.printf("Display file: %s\n", jpgFileNames[fileIndex]);
  
          // 画像をロードして表示
          canvas.fillSprite(BLACK);
          canvas.setTextColor(WHITE);
          canvas.setTextSize(1.4);
  
          SD.begin(15, SPI, 80000000);
  
          File jpgFile = SD.open(jpgFileNames[fileIndex]);
          if (jpgFile) {
            canvas.drawBmp(&jpgFile, 0, 0);  // あとでdrawJpgに変える
            jpgFile.close();
          } else {
            canvas.drawCenterString("Failed to load", 120, 60);
          }
          canvas.pushSprite(&display, 0, 0);
        }
      }
    }

    if (scroll.getButtonStatus()) {
      // プレビューモードでスクロールユニットのボタンが押されたら無条件で通常モードに戻る
      previewMode = false;
      scroll.setEncoderValue(-1);
      SD.end();
      delay(500);
    }
  }

  lastEncoderValue = newEncoderValue;
  delay(1);
}
