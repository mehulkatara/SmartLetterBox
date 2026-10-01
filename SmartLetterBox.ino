#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SinricPro.h"
#include "SinricProContactSensor.h"

// --- Wi-Fi & Sinric Pro Credentials ---
#define WIFI_SSID         ""
#define WIFI_PASS         ""
#define APP_KEY           ""
#define APP_SECRET        ""
#define CONTACT_SENSOR_ID ""

// --- Hardware Pins ---
const int IR_SENSOR_PIN = D5; // IR Sensor signal wire
const int RESET_BTN_PIN = D6; // Physical Reset Push Button

// --- OLED Display Setup ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Logic & Animation Variables ---
int mailCount = 0;
bool lastSensorState = HIGH;
bool lastBtnState    = HIGH;

int scanX = 0;
int scanDir = 2;
unsigned long lastAnimTime = 0;
bool iconState = false;

// Custom 16x12 pixel Mail Icons
const unsigned char PROGMEM envelope_closed[] = {
  0xFF, 0xFF, 0x80, 0x01, 0x80, 0x01, 0x98, 0x19,
  0x94, 0x29, 0x92, 0x49, 0x91, 0x89, 0x90, 0x09,
  0x80, 0x01, 0x80, 0x01, 0xFF, 0xFF, 0x00, 0x00
};

const unsigned char PROGMEM envelope_open[] = {
  0x01, 0x80, 0x03, 0xC0, 0x06, 0x60, 0x0C, 0x30,
  0x18, 0x18, 0x30, 0x0C, 0xFF, 0xFF, 0x80, 0x01,
  0x80, 0x01, 0x80, 0x01, 0xFF, 0xFF, 0x00, 0x00
};

void drawIdleAnimation();
void playNewMailAnimation();
void resetMailCount();
void sendAlexaAlert();

void setup() {
  Serial.begin(115200);

  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);

  // Initialize OLED Screen
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println(F("CONNECTING"));
  display.display();

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Initialize Sinric Pro
  SinricProContactsensor &mySensor = SinricPro[CONTACT_SENSOR_ID];
  SinricPro.begin(APP_KEY, APP_SECRET);

  // Automatic reset on boot
  resetMailCount();
}

void loop() {
  SinricPro.handle(); // Maintain connection to Alexa service
  
  bool currentSensorState = digitalRead(IR_SENSOR_PIN);
  bool currentBtnState    = digitalRead(RESET_BTN_PIN);

  // 1. MANUAL RESET BUTTON
  if (lastBtnState == HIGH && currentBtnState == LOW) {
    resetMailCount();
    delay(200); // Debounce
  }
  lastBtnState = currentBtnState;

  // 2. NEW MAIL DETECTION
  if (lastSensorState == HIGH && currentSensorState == LOW) {
    mailCount++;
    Serial.print("New mail detected! Total: ");
    Serial.println(mailCount);

    // Trigger Alexa Announcement
    sendAlexaAlert();

    // Play OLED Animation
    playNewMailAnimation();
  }
  lastSensorState = currentSensorState;

  // 3. CONTINUOUS IDLE ANIMATION
  if (millis() - lastAnimTime > 80) {
    lastAnimTime = millis();
    drawIdleAnimation();
  }
}

// Send Contact State to Sinric Pro to trip Alexa Routine
void sendAlexaAlert() {
  SinricProContactsensor &mySensor = SinricPro[CONTACT_SENSOR_ID];
  mySensor.sendContactEvent(true);  // OPEN state -> triggers Alexa Routine
  delay(1000);
  mySensor.sendContactEvent(false); // Reset back to CLOSED
}

// Reset mail count to 0 and show screen confirmation
void resetMailCount() {
  mailCount = 0;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(16, 16);
  display.println(F("MAIL COUNT"));
  display.setCursor(32, 40);
  display.println(F("RESET!"));
  display.display();
  delay(1200);
}

// OLED Idle Dashboard Animation
void drawIdleAnimation() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, 2);
  display.println(F("SMART MAILBOX"));

  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
  display.fillRect(scanX, 10, 16, 3, SSD1306_WHITE);
  scanX += scanDir;
  if (scanX >= 112 || scanX <= 0) scanDir = -scanDir;

  if (scanX % 20 == 0) iconState = !iconState;
  display.drawBitmap(8, 26, iconState ? envelope_open : envelope_closed, 16, 12, SSD1306_WHITE);

  display.setTextSize(3); 
  display.setCursor(38, 22);
  if (mailCount < 10) display.print(F("0"));
  display.print(mailCount);

  display.setTextSize(1);
  display.setCursor(92, 36);
  display.println(F("PCS"));

  display.drawFastHLine(0, 54, 128, SSD1306_WHITE);
  for (int i = 0; i < 128; i += 8) {
    int height = (i / 8 + scanX / 4) % 2 == 0 ? 4 : 1;
    display.drawFastVLine(i, 64 - height, height, SSD1306_WHITE);
  }

  display.display();
}

// OLED New Mail Pop Animation
void playNewMailAnimation() {
  for (int r = 2; r < 60; r += 6) {
    display.clearDisplay();
    display.drawRoundRect(64 - r, 32 - (r / 2), r * 2, r, 4, SSD1306_WHITE);
    display.display();
    delay(20);
  }

  for (int flash = 0; flash < 4; flash++) {
    display.clearDisplay();
    display.fillRect(0, 0, 128, 64, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setTextSize(2);
    display.setCursor(12, 12);
    display.println(F("NEW MAIL!"));
    display.setCursor(20, 38);
    display.print(F("DROP #"));
    display.print(mailCount);
    display.display();
    delay(300);

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(12, 12);
    display.println(F("NEW MAIL!"));
    display.setCursor(20, 38);
    display.print(F("DROP #"));
    display.print(mailCount);
    display.display();
    delay(200);
  }
}