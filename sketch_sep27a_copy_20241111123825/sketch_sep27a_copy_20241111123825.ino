#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>

// Pin definitions
#define DHTPIN 4        // Pin connected to DHT11 data pin
#define DHTTYPE DHT11
#define UV_PIN 35       // Pin connected to GYML8511 output pin
#define OLED_RESET -1   // OLED reset pin (we'll use -1 as it's not connected)
#define BUTTON_PIN 15   // Pin for button to cycle OLED display pages
#define SD_CS_PIN 5     // Chip select pin for SD card

// Sensor objects
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
Adafruit_SSD1306 display(OLED_RESET);
RTC_DS1307 rtc;

bool oledOn = true;      // OLED display state
File logFile;            // SD card file object
int currentPage = 0;     // To track which page we're displaying
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200; // Debounce delay

void setup() {
  Serial.begin(115200);

  // Initialize DHT11
  dht.begin();

  // Initialize BMP180
  if (!bmp.begin()) {
    Serial.println("BMP180 initialization failed!");
    while (1);
  }

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 0x3C is the I2C address of OLED
    Serial.println("SSD1306 OLED initialization failed!");
    while (1);
  }
  display.clearDisplay();

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("RTC initialization failed!");
    while (1);
  }

  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed! Check wiring and SD card format.");
    while (1);
  }

  // Set up push button for OLED control
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  // Debounced button press to cycle pages
  if (digitalRead(BUTTON_PIN) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    currentPage = (currentPage + 1) % 6; // Cycle through 6 pages (0-5)
    delay(200); // Additional debounce delay to avoid multiple triggers
  }

  // Collect sensor data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float pressure = bmp.readPressure();
  int uvIndex = analogRead(UV_PIN);

  // Get time from RTC
  DateTime now = rtc.now();
  String timestamp = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

  // Log data to SD card
  logFile = SD.open("data_log.txt", FILE_WRITE);
  if (logFile) {
    logFile.print("Time: "); logFile.print(timestamp);
    logFile.print(", Temp: "); logFile.print(temperature);
    logFile.print(", Humidity: "); logFile.print(humidity);
    logFile.print(", Pressure: "); logFile.print(pressure);
    logFile.print(", UV Index: "); logFile.println(uvIndex);
    logFile.close();
  } else {
    Serial.println("Error opening file for writing");
  }

  // Display data based on the current page
  display.clearDisplay();

  switch (currentPage) {
    case 0: // Display Temperature
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.print("Temp: ");
      display.print(temperature);
      display.println(" C");
      break;

    case 1: // Display Humidity
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.print("Humidity: ");
      display.print(humidity);
      display.println(" %");
      break;

    case 2: // Display Pressure
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.print("Pressure: ");
      display.print(pressure);
      display.println(" Pa");
      break;

    case 3: // Display UV Index
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.print("UV Index: ");
      display.print(uvIndex);
      break;

    case 4: // Display Time
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.print("Time: ");
      display.println(timestamp);
      break;

    case 5: // Off page (clear display)
      display.clearDisplay();
      oledOn = false;
      break;

    default:
      break;
  }

  if (currentPage != 5) {  // Keep display off on the "Off" page
    oledOn = true;
    display.display();
  }

  delay(1000); // Delay for sensor reading
}
