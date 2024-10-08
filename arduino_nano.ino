#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
const int trigPin = 9;
const int echoPin = 10;
const int laserPin = 11;
const int buttonPin = 2;

// Variabel global
int distance;
char currentTime[9];
char currentDate[11];
char currentDay[10];

int lastButtonState = HIGH; 
int currentButtonState;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(laserPin, OUTPUT);
  digitalWrite(laserPin, HIGH); // Turn on laser permanently
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Distance:");

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1); // If RTC is not found, display message and stop
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
    // // Set RTC time manually (example: 2nd August 2024, 22:07:00)
    // rtc.adjust(DateTime(2024, 8, 10, 13, 35, 15)); // set time manually
  }

  // Test RTC
  DateTime testNow = rtc.now();
  Serial.print("Current time: ");
  Serial.print(testNow.hour());
  Serial.print(":");
  Serial.print(testNow.minute());
  Serial.print(":");
  Serial.print(testNow.second());
  Serial.print(" ");
  Serial.print(testNow.day());
  Serial.print("/");
  Serial.print(testNow.month());
  Serial.print("/");
  Serial.print(testNow.year());
  Serial.println();

  // Send PLX-DAQ headers to Serial Monitor
  Serial.println("CLEARDATA"); 
  Serial.println("LABEL,Time,Date,Day,Distance (cm)");
  Serial.println("RESETTIMER");
}

void loop() {
  // Read the state of the button and debounce
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if (millis() - lastDebounceTime > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      if (currentButtonState == LOW) {
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        long duration = pulseIn(echoPin, HIGH);

        // Convert duration to distance
        distance = duration * 0.03378/ 2;

        // get time and date from RTC
        DateTime now = rtc.now();
        sprintf(currentTime, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
        sprintf(currentDate, "%04d-%02d-%02d", now.year(), now.month(), now.day());
        sprintf(currentDay, "%s", dayStr(now.dayOfTheWeek()));

        // Debugging
        Serial.print("Current time: ");
        Serial.print(currentTime);
        Serial.print(", ");
        Serial.print(currentDate);
        Serial.print(", ");
        Serial.print(currentDay);
        Serial.print(", Distance: ");
        Serial.println(distance);

        // Display distance on LCD
        lcd.setCursor(0, 1);
        lcd.print("       ");
        lcd.setCursor(0, 1);
        lcd.print(distance);
        lcd.print(" cm");

        // send data to plx
        Serial.print("DATA,");
        Serial.print(currentTime);
        Serial.print(",");
        Serial.print(" ");
        Serial.print(currentDate);
        Serial.print(",");
        Serial.print(" ");
        Serial.print(currentDay);
        Serial.print(",");
        Serial.print(" ");
        Serial.println(distance);
        delay(500);
      }
    }
  }
  lastButtonState = reading;
}
// Function to convert day of the week to string
const char* dayStr(int dayOfTheWeek) {
  switch (dayOfTheWeek) {
    case 0: return "Sunday";
    case 1: return "Monday";
    case 2: return "Tuesday";
    case 3: return "Wednesday";
    case 4: return "Thursday";
    case 5: return "Friday";
    case 6: return "Saturday";
    default: return "Unknown";
  }
}
