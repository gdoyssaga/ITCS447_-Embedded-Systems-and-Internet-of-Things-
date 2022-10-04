
#include <WiFi.h> 
#include <time.h>
#include <DS3231.h>
#include <Wire.h>

//LCD
#include <SPI.h>
#include <TFT_eSPI.h>
#define TFT_GREY 0x5AEB // New colour
TFT_eSPI tft = TFT_eSPI();  // Invoke library
 

// The below are constants, which cannot be changed
#define LDR  36  // ESP32 pin GPIO36 (ADC0) connected to light sensor
#define LED_PIN 33 // ESP32 pin GPIO23 connected to LED
#define BUTTON_PIN 5//ESP32 pin GPIO connected to pushButton 
#define ANALOG_THRESHOLD  100
int button_state;       // the current state of button
int LED_state; 

//WIFI
const char* ssid       = "ฺฺBell";
const char* password   = "lovebella11025"; 


//RTC
const char* ntpServer = "th.pool.ntp.org";
const long  gmtOffset_sec = 3600 * 7; //UTC +7.00
const int   daylightOffset_sec = 0; //0 means no DST observed; otherwise, 3600.

DS3231  rtc;

bool h12Format;
bool ampm;
bool centuryRollover;
//bool LED_status;

struct tm timeinfo;
//End of RTC


void setup() {
  pinMode(LED_PIN, OUTPUT); // set ESP32 pin to output mode
  Serial.begin(9600);
  Wire.begin();
//BUTTON
 button_state = digitalRead(BUTTON_PIN);

//WIFI
#ifdef _SYNC_NTP
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

//RTC
 //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println("M:" + String(timeinfo.tm_mon) + ", Y:" + String(timeinfo.tm_year));
  rtc.enableOscillator(true, true, 1);
  rtc.setClockMode(h12Format); //24-h format
  rtc.setDoW(timeinfo.tm_wday);
  rtc.setHour(timeinfo.tm_hour);
  rtc.setMinute(timeinfo.tm_min);
  rtc.setSecond(timeinfo.tm_sec);
  rtc.setDate(timeinfo.tm_mday);
  rtc.setMonth(timeinfo.tm_mon + 1); //Month from NTP starts from zero
  rtc.setYear(timeinfo.tm_year - 100); //Year from NTP is an offset from 1900

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  #endif

 //LCD
  tft.init();
  tft.setRotation(2);
 
}

void loop() {
  
  int ldrValue = analogRead(LDR); // read the value on analog pin
  Serial.println(ldrValue);
//  delay(1000);
  if (ldrValue < ANALOG_THRESHOLD) {
    digitalWrite(LED_PIN, HIGH); // turn on LED 
    LED_state = 1;         
  } else {
    digitalWrite(LED_PIN, LOW);  // turn off LED
    LED_state = 0; }

//Button
  


//RTC_PART
  #ifdef _SYNC_NTP
  //Show time from NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println("M:" + String(timeinfo.tm_mon) + ", Y:" + String(timeinfo.tm_year));
  #endif
  // Send Day-of-Week
  Serial.print("DoW:");
  Serial.print(rtc.getDoW());
  Serial.print(" ");
  
  // Send date
  Serial.print("-- Date: ");
  Serial.print(rtc.getDate(), DEC);
  Serial.print("/");
  Serial.print(rtc.getMonth(centuryRollover), DEC);
  Serial.print("/");
  Serial.print("2"); //This program is still valid until almost the next 1000 years.
  if(centuryRollover)
    Serial.print("1");
  else
    Serial.print("0");
  Serial.print(rtc.getYear(), DEC);

  // Send time
  Serial.print(" -- Time: ");
  Serial.print(rtc.getHour(h12Format, ampm), DEC);
  Serial.print(":");
  Serial.print(rtc.getMinute(), DEC);
  Serial.print(":");
  Serial.print(rtc.getSecond(), DEC);

  Serial.println(ldrValue);


//LCD_PART
 // Fill screen with grey so we can see the effect of printing with and without 
  // a background colour defined
  tft.fillScreen(TFT_GREY);
  
  // Set "cursor" at top left corner of display (0,0) and select font 2
  // (cursor will move to next line automatically during printing with 'tft.println'
  //  or stay on the line is there is room for the text with tft.print)
  tft.setCursor(0, 0, 2);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);


 // Print LED_Status
 
  tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.setTextFont(4);
  tft.print("LED: ");
  if (LED_state == 1 )
    tft.println("ON");// turn on LED 
  else
    tft.println("OFF");// turn off LED
  
 // Set the font colour to be blue with no background, set to font 4
  tft.setTextColor(TFT_BLUE);    tft.setTextFont(4);
  tft.print("LDR Value: "); tft.println(ldrValue);           // Print LDR Value
  tft.print("DATE: "); tft.print(rtc.getDate(), DEC); tft.print("/"); tft.print(rtc.getMonth(centuryRollover), DEC); tft.print("/"); tft.print("20"); tft.println(rtc.getYear(), DEC);// Print DATE
  tft.print("Time: "); tft.print(rtc.getHour(h12Format, ampm), DEC); tft.print(":"); tft.print(rtc.getMinute(), DEC); tft.print(":"); tft.print(rtc.getSecond(), DEC); // Print Time
   
  delay(1000);



    
}
