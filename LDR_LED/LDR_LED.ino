#define _SYNC_NTP
#include <WiFi.h> 
#include <time.h>
#include <DS3231.h> 
#include <Wire.h>

#include <SPI.h>  
#include <TFT_eSPI.h> 
TFT_eSPI tft = TFT_eSPI();  

//ESP32 pin GPIO12,GPIO13,GPIO14 connected to RGB LED
#define LED_Red 12 
#define LED_Green 13 
#define LED_Blue 14 
#define BUTTON 5 //ESP32 pin GPI5 connected to pushButton
#define LDR  36  // ESP32 pin GPIO36 connected to light sensor 
#define ANALOG_THRESHOLD  100 //Threshold for controlling the LDR sensoring

#define STATE0 0
#define STATE1 1

#define TFT_GREY 0x5AEB // colour

int button_state;       // the current state of button
int last_button_state;  // the previous state of button
int mode = 0; //define color mode
unsigned char state; // setup RGB LED State

//------------ RTC ------------
const char* ssid       = "";
const char* password   = ""; 
const char* ntpServer = "th.pool.ntp.org";
const long  gmtOffset_sec = 3600 * 7; //UTC +7.00
const int   daylightOffset_sec = 0; //0 means no DST observed; otherwise, 3600.

DS3231  rtc;

bool h12Format;
bool ampm;
bool centuryRollover;

struct tm timeinfo;
//------------ End of RTC ------------

void setup() { 
  pinMode(LDR, INPUT);  //set LDR pin to input mode
   //set ESP32 pin to output mode
  pinMode(LED_Red, OUTPUT); 
  pinMode(LED_Green, OUTPUT); 
  pinMode(LED_Blue, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);  // set button pin to input pull-up mode
  Wire.begin();
  Serial.begin(9600);
  int state = 0; //Deflaut state setup
  
  //------------- WIFI SETUP ------------
  #ifdef _SYNC_NTP
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("CONNECTED");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  //------------- RTC SETUP ------------
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

  //TFT_LCD
   tft.init();
   tft.setRotation(2);

 }

// Set up Color of RGB LED
void RGBled(int i,int j,int k){
  digitalWrite(LED_Red, i);
  digitalWrite(LED_Green, j);
  digitalWrite(LED_Blue, k);
  }


void loop() {
  int ldrValue = analogRead(LDR); // read the value on analog pin
  Serial.println("LDR: " + String(ldrValue));
  last_button_state = button_state;   // save the last state
  button_state = digitalRead(BUTTON); // read new state
  if (ldrValue >= 0) {//To Start the SMART LED LAMP system HERE Check the light intensity value by using LDR sensor
  switch(state) {
    case STATE0://turn off 
      RGBled(0,0,0);
      if (ldrValue < ANALOG_THRESHOLD ) 
        state = STATE1;
        break;
      break;
    case STATE1://turn on  
      RGBled(1,1,1);
      //To Change the color mode start when press the switch button
      if(last_button_state == HIGH && button_state == LOW){
        mode = mode + 1;  
      }          
      if (mode == 0){
        RGBled(1,1,1); //White
      }
      if (mode == 1){
        RGBled(1,0,0); //RED
      }
      if (mode == 2){
        RGBled(0,1,0);//GREEN
      }
      if (mode == 3){
        RGBled(0,0,1);//BLUE
      }
      if (mode == 4){
        RGBled(0,0,0);//OFF
      }
      if (mode == 5){
        mode = 0; //Reset mode
      }
      if (ldrValue > ANALOG_THRESHOLD ) 
        state = STATE0;
        break;  
      break;
    }
 }

  Serial.println("MODE " + String(mode));
  Serial.println("STATE " + String(state));
  Serial.println(" ");

  //-------------------------- RTC PART -------------------------- 
  
  #ifdef _SYNC_NTP
  //Show time from NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println("M:" + String(timeinfo.tm_mon) + ", Y:" + String(timeinfo.tm_year));
  #endif
   
// Send Date and Time
  int day = rtc.getDate();
  int month = rtc.getMonth(centuryRollover); 
  int year = rtc.getYear();
  int hr = rtc.getHour(h12Format, ampm);
  int mins = rtc.getMinute();
  
  //-------------------------- LCD_PART -------------------------- 
  
  tft.fillScreen(TFT_GREY);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_BLUE);
  tft.setTextFont(4);
   // TO print Intensity Value, Date and Time on LCD Screen
  tft.print("Intensity Value: "); tft.println(ldrValue); 
  tft.println("DATE: "+String(day)+"/"+String(month)+"/"+String(year)); 
  tft.print("Time: ");
    if( hr < 10 && mins < 10)
    {
     tft.print("0"+String(hr)+":"+"0"+String(mins)); 
    }else if(hr > 10 && mins < 10)
    {
     tft.print(String(hr)+":"+"0"+String(mins));
    } else {
      tft.print(String(hr)+":"+String(mins));
     }


   // TO print LED Status on LCD Screen
  String LEDStatusText = "";
  if (state == 1 ){
    tft.setTextColor(TFT_GREEN,TFT_BLACK);
    
    if (mode == 0){
        tft.setTextColor(TFT_WHITE);
        LEDStatusText = "ON (White)"; //White
      }
      if (mode == 1){
         tft.setTextColor(TFT_RED);
        LEDStatusText = "ON (RED)"; //RED
      }
      if (mode == 2){
        tft.setTextColor(TFT_GREEN);
        LEDStatusText = "ON (GREEN)"; //GREEN
      }
      if (mode == 3){
        tft.setTextColor(TFT_BLUE);
        LEDStatusText = "ON (BLUE)"; //BLUE
      }
      if(mode == 4 ){
      tft.setTextColor(TFT_PINK,TFT_BLACK);
      LEDStatusText = "OFF";// turn off LED
    }   
  }
  else if (state == 0){
    tft.setTextColor(TFT_PINK,TFT_BLACK);
      LEDStatusText = "OFF";// turn off LED
    }
  tft.drawCentreString("LED: ",120,100,4);
  tft.drawCentreString(LEDStatusText,120,150,4);
   
delay(1000);
  
}
