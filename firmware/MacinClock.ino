#include <Adafruit_GFX.h>    
#include <Adafruit_ST7789.h> 
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include "HappyMac.h" 

// --- RP2040 EXACT PINS ---
#define TFT_MOSI 3
#define TFT_SCLK 2
#define TFT_CS   5
#define TFT_DC   4
#define TFT_RST  1
const int backlightPin = 0;   

// --- MAC CONSTANTS ---
#define MAC_BLACK ST77XX_BLACK
#define MAC_WHITE ST77XX_WHITE

// --- BITMAPS ---
const unsigned char epd_bitmap_apple_uwu [] PROGMEM = {
  0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 
  0x00, 0x1f, 0xe0, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x3f, 0xc0, 0x00, 
  0x00, 0x3f, 0xc0, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x1f, 0xe0, 0x00, 
  0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xf8
};

const unsigned char docIcon [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xF8, 0x00, 0x04, 0x00, 0x04, 0x00, 
  0x04, 0x00, 0x06, 0x00, 0x04, 0x00, 0x05, 0x00, 0x04, 0x00, 0x04, 0x80, 
  0x04, 0x00, 0x04, 0x40, 0x04, 0x00, 0x04, 0x20, 0x04, 0x00, 0x04, 0x10, 
  0x04, 0x00, 0x07, 0xF0, 0x04, 0x00, 0x00, 0x10, 0x04, 0x1F, 0x00, 0x10, 
  0x04, 0x00, 0x00, 0x10, 0x04, 0x7F, 0xE0, 0x10, 0x04, 0x00, 0x00, 0x10, 
  0x04, 0x7F, 0xE0, 0x10, 0x04, 0x00, 0x00, 0x10, 0x04, 0x7F, 0xE0, 0x10, 
  0x04, 0x00, 0x00, 0x10, 0x04, 0x7F, 0xE0, 0x10, 0x04, 0x00, 0x00, 0x10, 
  0x04, 0x7F, 0xE0, 0x10, 0x04, 0x00, 0x00, 0x10, 0x04, 0x1F, 0x80, 0x10, 
  0x04, 0x00, 0x00, 0x10, 0x07, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00
};

const unsigned char mac_dither [] PROGMEM = {
  0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA
};

// --- THE FERRARI HARDWARE ENGINE ---
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI, TFT_CS, TFT_DC, TFT_RST);

// 🧠 THE MINI-CANVAS (Zero flickering, tiny memory footprint!)
GFXcanvas16 clockCanvas(230, 110); 
RTC_DS3231 rtc;

// --- Function Declarations ---
void updateClockDisplay(DateTime t);
void drawMacDesktop();
void drawMenuBar();
void drawDesktopFile(int x, int y, const char* label);
void drawWindow(int x, int y, int w, int h, const char* title);
bool isDaylightSaving(int month, int day, int dow);

void setup() {
  Serial.begin(115200);

  pinMode(backlightPin, OUTPUT);
  digitalWrite(backlightPin, HIGH); 

  Wire1.setSDA(6);
  Wire1.setSCL(7);
  Wire1.begin(); 
  
  if (!rtc.begin(&Wire1)) {
    Serial.println("Error: Couldn't find RTC!");
  }

  // NOTE: If you ever need to fix your time, remove the // below, upload once, then replace the // and upload again!
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  rtc.adjust(rtc.now() - TimeSpan(0, 1, 0, 0)); 

  SPI.setSCK(TFT_SCLK);
  SPI.setTX(TFT_MOSI);

  tft.init(240, 320);
  tft.setRotation(1);       
  
  tft.fillScreen(MAC_WHITE);
  tft.drawBitmap(0, 0, epd_bitmap_Happy_Mac, 320, 240, MAC_BLACK);
  delay(2000); 
  
  drawMacDesktop();
  drawMenuBar();
  drawDesktopFile(270, 190, "By Grant");
  drawWindow(40, 35, 240, 140, "System Clock");
}

void loop() {
  DateTime now = rtc.now();

  if (isDaylightSaving(now.month(), now.day(), now.dayOfTheWeek())) {
    now = now + TimeSpan(0, 1, 0, 0); 
  }

  updateClockDisplay(now);
  delay(50); 
}

bool isDaylightSaving(int month, int day, int dow) {
  if (month < 3 || month > 11) return false;
  if (month > 3 && month < 11) return true;
  int previousSunday = day - dow; 
  if (month == 3) return previousSunday >= 8;
  if (month == 11) return previousSunday <= 0;
  return false;
}

void updateClockDisplay(DateTime t) {
  static int lastSecond = -1;
  if (t.second() == lastSecond) return; 
  lastSecond = t.second();

  // 1. Wipe the mini-canvas memory
  clockCanvas.fillScreen(MAC_WHITE);

  int displayHour = t.hour();
  const char* ampm = "AM";

  if (displayHour >= 12) {
    ampm = "PM";
    if (displayHour > 12) displayHour -= 12;
  }
  if (displayHour == 0) displayHour = 12;

  char myTimeBuffer[20]; 
  snprintf(myTimeBuffer, sizeof(myTimeBuffer), "%2d:%02d:%02d", displayHour, t.minute(), t.second());
  char myDateBuffer[30]; 
  snprintf(myDateBuffer, sizeof(myDateBuffer), "%02d-%02d-%04d", t.month(), t.day(), t.year()); 

  clockCanvas.setTextColor(MAC_BLACK, MAC_WHITE);
  
// Time (Large Pixel Font with Custom Squeezed Spacing!)
  clockCanvas.setTextSize(4);
  int cursorX = 23; // Centered starting position for the tighter text
  
  for (int i = 0; i < strlen(myTimeBuffer); i++) {
    clockCanvas.setCursor(cursorX, 20);
    clockCanvas.print(myTimeBuffer[i]);
    
    // Manually push the next letter closer!
    if (myTimeBuffer[i] == ':') {
      cursorX += 16; // Colons are super skinny, squish them way in!
    } else {
      cursorX += 22; // Numbers are pushed closer (default is 24, we cut it down!)
    }
  }

  // AM/PM (Medium Pixel Font)
  clockCanvas.setTextSize(2);
  clockCanvas.setCursor(190, 35);
  clockCanvas.print(ampm);

  // Date (Medium Pixel Font)
  clockCanvas.setTextSize(2);
  clockCanvas.setCursor(55, 75);
  clockCanvas.print(myDateBuffer);

  // 2. Slap the fully drawn mini-canvas directly to the glass (Zero flickering!)
  tft.drawRGBBitmap(45, 60, clockCanvas.getBuffer(), 230, 110);
}

void drawMacDesktop() {
  for (int y = 20; y < 240; y += 8) {
    for (int x = 0; x < 320; x += 8) {
      tft.drawBitmap(x, y, mac_dither, 8, 8, MAC_BLACK, MAC_WHITE);
    }
  }
}

void drawMenuBar() {
  tft.fillRect(0, 0, 320, 20, MAC_WHITE);
  tft.drawLine(0, 20, 320, 20, MAC_BLACK);
  tft.drawBitmap(2, -2, epd_bitmap_apple_uwu, 29, 23, MAC_BLACK);
  
  tft.setTextColor(MAC_BLACK, MAC_WHITE);
  tft.setTextSize(1);
  tft.setCursor(45, 6); tft.print("File");
  tft.setCursor(85, 6); tft.print("Edit");
  tft.setCursor(125, 6); tft.print("View");
  tft.setCursor(165, 6); tft.print("Tool");
}

void drawDesktopFile(int x, int y, const char* label) {
  // Explicitly draw the icon with a white background so it perfectly overrides the checkerboard
  tft.drawBitmap(x, y, docIcon, 32, 32, MAC_BLACK, MAC_WHITE);
  
  tft.setTextSize(1);
  tft.setTextColor(MAC_BLACK, MAC_WHITE); // Snug text highlighting instead of a massive box
  
  int textW = strlen(label) * 6; // approximate width of pixel font
  int textX = x + 16 - (textW / 2);
  
  tft.setCursor(textX, y + 35);
  tft.print(label);
}

void drawWindow(int x, int y, int w, int h, const char* title) {
  tft.fillRect(x + 3, y + 3, w, h, MAC_BLACK); 
  tft.fillRect(x, y, w, h, MAC_WHITE);
  tft.drawRect(x, y, w, h, MAC_BLACK);
  tft.fillRect(x, y, w, 20, MAC_WHITE);
  tft.drawRect(x, y, w, 20, MAC_BLACK);
  for(int i=4; i<16; i+=2) {
      tft.drawLine(x+2, y+i, x+w-2, y+i, MAC_BLACK);
  }
  tft.fillRect(x + 5, y + 4, 12, 12, MAC_WHITE);
  tft.drawRect(x + 5, y + 4, 12, 12, MAC_BLACK);
  tft.drawLine(x + 7, y + 6, x + 14, y + 13, MAC_BLACK);
  tft.drawLine(x + 14, y + 6, x + 7, y + 13, MAC_BLACK);
  tft.fillRect(x + w - 30, y + 4, 12, 12, MAC_WHITE);
  tft.drawRect(x + w - 30, y + 4, 12, 12, MAC_BLACK);
  tft.drawFastHLine(x + w - 27, y + 12, 6, MAC_BLACK);
  tft.fillRect(x + w - 17, y + 4, 12, 12, MAC_WHITE);
  tft.drawRect(x + w - 17, y + 4, 12, 12, MAC_BLACK);
  tft.drawRect(x + w - 14, y + 7, 6, 6, MAC_BLACK);
  
  tft.fillRect(x + (w/2) - 45, y + 1, 90, 18, MAC_WHITE);
  tft.setTextColor(MAC_BLACK, MAC_WHITE);
  tft.setTextSize(1);
  tft.setCursor(x + (w/2) - 35, y + 6);
  tft.print(title);
}