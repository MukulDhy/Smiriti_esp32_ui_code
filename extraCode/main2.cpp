#include <TFT_eSPI.h>
#include <WiFi.h>
#include "time.h"
#include <XPT2046_Touchscreen.h>

// Display configuration
TFT_eSPI tft = TFT_eSPI();
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Touch screen configuration - Changed to pin 32 which is more common for your board
#define TOUCH_CS_PIN 32
XPT2046_Touchscreen touch(TOUCH_CS_PIN);

enum ScreenState
{
  HOME_SCREEN,
  MENU_SCREEN
};
ScreenState currentScreen = HOME_SCREEN;

// Time management
unsigned long lastTimeUpdate = 0;
const unsigned long TIME_UPDATE_INTERVAL = 1000;

// WiFi Configuration
const char *WIFI_SSID = "Mukuldhy";
const char *WIFI_PASS = "12345678";
const unsigned long WIFI_TIMEOUT = 10000;

// NTP Configuration
const char *NTP_SERVERS[] = {
    "pool.ntp.org",
    "time.google.com",
    "time.nist.gov",
    "time.windows.com",
    "1.in.pool.ntp.org",
    NULL};
const long GMT_OFFSET = 19800;
const int DAYLIGHT_OFFSET = 0;

// Button structure
struct Button
{
  int x, y, width, height;
  const char *label;
  uint16_t color;
  void (*action)();
};

// Function prototypes
void connectToWiFi();
void handleTouch();
bool isTouched(Button btn, int x, int y);
void updateTime();
void drawHomeScreen();
void drawMenuScreen();
void drawButton(Button btn);
void checkWiFiStrength();

// Button instances - Adjusted positions for better touch response
Button menuButton = {
    220, 180, 80, 50, // Slightly larger and better positioned
    "Menu", TFT_BLUE,
    []()
    {
      currentScreen = MENU_SCREEN;
      drawMenuScreen();
    }};

Button wifiButton = {
    50, 80, 100, 50, // Increased height for better touch
    "WiFi", TFT_GREEN,
    checkWiFiStrength};

Button reminderButton = {
    50, 140, 100, 50, // Increased height
    "Reminder", TFT_RED,
    []() {}};

Button backButton = {
    220, 180, 80, 50, // Same as menu button
    "Back", TFT_BLUE,
    []()
    {
      currentScreen = HOME_SCREEN;
      drawHomeScreen();
    }};

void setup()
{
  Serial.begin(115200);

  // Initialize display
  tft.init();
  tft.setRotation(1); // Landscape mode
  tft.fillScreen(TFT_BLACK);

  // Initialize touch screen with SPI speed setting
  SPI.begin(); // Ensure SPI is initialized
  touch.begin();
  touch.setRotation(1); // Match display rotation

  // Connect to WiFi
  connectToWiFi();

  // Configure time
  configTime(GMT_OFFSET, DAYLIGHT_OFFSET, NTP_SERVERS[0]);

  drawHomeScreen();
}

void loop()
{
  if (currentScreen == HOME_SCREEN && millis() - lastTimeUpdate >= TIME_UPDATE_INTERVAL)
  {
    updateTime();
    lastTimeUpdate = millis();
  }
  handleTouch();
}

void connectToWiFi()
{
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Connecting to WiFi...", 80, 100);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT)
  {
    delay(250);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    tft.fillScreen(TFT_BLACK);
    tft.drawString("WiFi Connected!", 100, 100);
    delay(1000);
  }
  else
  {
    tft.fillScreen(TFT_BLACK);
    tft.drawString("WiFi Failed!", 100, 100);
    delay(2000);
  }
}

void handleTouch()
{
  if (touch.touched())
  {
    TS_Point p = touch.getPoint();

    // Improved touch calibration - these values may need adjustment
    int x = map(p.x, 200, 3700, 0, SCREEN_WIDTH);
    int y = map(p.y, 240, 3800, 0, SCREEN_HEIGHT);

    Serial.printf("Raw Touch: X=%d, Y=%d | Mapped: X=%d, Y=%d\n", p.x, p.y, x, y);

    if (currentScreen == HOME_SCREEN)
    {
      if (isTouched(menuButton, x, y))
      {
        Serial.println("Menu button pressed");
        menuButton.action();
      }
    }
    else if (currentScreen == MENU_SCREEN)
    {
      if (isTouched(wifiButton, x, y))
      {
        Serial.println("WiFi button pressed");
        wifiButton.action();
      }
      else if (isTouched(reminderButton, x, y))
      {
        Serial.println("Reminder button pressed");
        reminderButton.action();
      }
      else if (isTouched(backButton, x, y))
      {
        Serial.println("Back button pressed");
        backButton.action();
      }
    }

    delay(200); // Debounce delay
  }
}

bool isTouched(Button btn, int x, int y)
{
  bool touched = (x > btn.x && x < btn.x + btn.width &&
                  y > btn.y && y < btn.y + btn.height);
  if (touched)
  {
    Serial.printf("Button %s area: (%d-%d, %d-%d)\n",
                  btn.label, btn.x, btn.x + btn.width, btn.y, btn.y + btn.height);
  }
  return touched;
}

void updateTime()
{
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 100))
  {
    char timeString[20];
    strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);

    static char lastTimeString[20] = "";
    if (strcmp(timeString, lastTimeString) != 0)
    {
      strcpy(lastTimeString, timeString);
      tft.fillRect(100, 120, 120, 20, TFT_BLACK);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString(timeString, 100, 120);
    }
  }
  else
  {
    tft.fillRect(100, 120, 120, 20, TFT_RED);
    tft.drawString("--:--:--", 100, 120);
  }
}

void drawHomeScreen()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Smart Display", 100, 30);
  tft.drawString("Current Time:", 100, 90);
  drawButton(menuButton);
  updateTime();
}

void drawMenuScreen()
{
  tft.fillScreen(TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.drawString("Menu", 140, 30);
  drawButton(wifiButton);
  drawButton(reminderButton);
  drawButton(backButton);
}

void drawButton(Button btn)
{
  tft.fillRoundRect(btn.x, btn.y, btn.width, btn.height, 5, btn.color);
  tft.setTextColor(TFT_WHITE, btn.color);
  int textX = btn.x + (btn.width - strlen(btn.label) * 12) / 2;
  int textY = btn.y + (btn.height - 16) / 2;
  tft.drawString(btn.label, textX, textY);

  // Visual feedback for button boundaries (debug only)
  // tft.drawRect(btn.x, btn.y, btn.width, btn.height, TFT_WHITE);
}

void checkWiFiStrength()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);

  int32_t rssi = WiFi.RSSI();
  String strength = (rssi >= -50) ? "Excellent" : (rssi >= -60) ? "Very Good"
                                              : (rssi >= -70)   ? "Good"
                                              : (rssi >= -80)   ? "Low"
                                                                : "Very Weak";

  tft.drawString("WiFi Status:", 100, 50);
  tft.drawString("Strength: " + strength, 100, 80);
  tft.drawString("RSSI: " + String(rssi) + " dBm", 100, 110);

  Button okButton = {120, 160, 80, 50, "OK", TFT_BLUE, []()
                     { drawMenuScreen(); }};
  drawButton(okButton);

  while (true)
  {
    if (touch.touched())
    {
      TS_Point p = touch.getPoint();
      int x = map(p.x, 200, 3700, 0, SCREEN_WIDTH);
      int y = map(p.y, 240, 3800, 0, SCREEN_HEIGHT);
      if (isTouched(okButton, x, y))
      {
        okButton.action();
        break;
      }
    }
    delay(100);
  }
}


// #include <TFT_eSPI.h>

// TFT_eSPI tft = TFT_eSPI();  // Initialize TFT

// void setup() {
//   Serial.begin(115200);
//   tft.init();
//   tft.setRotation(1);  // Adjust based on your display
  
//   // Calibration values (adjust after testing)
//   uint16_t calData[5] = { 200, 3700, 240, 3800, 1 }; // Default values (replace with your calibration)
//   tft.setTouch(calData);
  
//   Serial.println("Touch Test - Touch the screen!");
// }

// void loop() {
//   uint16_t x, y;
  
//   // Check if touched
//   if (tft.getTouch(&x, &y)) {
//     Serial.printf("Touched at: X=%d, Y=%d\n", x, y);
//     tft.fillCircle(x, y, 5, TFT_RED);  // Draw a dot where touched
//   }
//   delay(50);  // Small delay to avoid flooding serial
// }