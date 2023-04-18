#define BLYNK_TEMPLATE_ID "TMPL68IjsdhRS"
#define BLYNK_TEMPLATE_NAME "Feeder"
#define BLYNK_AUTH_TOKEN "x2HmUIMByZcdVxe2Gl7YN4mnoLzQj8PW"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// time
#include "time.h"
const char* ntpServer = "id.pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 25200;//GMT+7:00

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "HUAWEI-UkGS";
char pass[] = "selaludiganti";

// konfigurasi untuk jadwal pemberian makanan---------------
int pagi_hour, pagi_min;
int siang_hour, siang_min;
int malam_hour, malam_min;
bool stopfeeding = false;
bool feednow = false;

// timer ---------------------------------------------
BlynkTimer timer;
unsigned long interval = 10000;

//servo motor -----------------------------------------
#include <Servo.h>

Servo myservo;  // create servo object to control a servo
const int pin_servo = 13;
const int close_cover = 180;
const int open_cover = 0;

// load cell ------------
#include <Arduino.h>
#include "HX711.h"
#include "soc/rtc.h"

const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;
HX711 scale;

double reading;

void setup() {
  Serial.begin(115200);

  // setup load cell
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(-167.843);
  delay(1000);
  scale.tare();               // reset the scale to 0
  Serial.println("scale ready");

  //setup servo motor
  myservo.attach(pin_servo);  // attaches the servo on pin 13 to the servo object
  myservo.write(close_cover);
  
  //setup blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // Setup a function myTimerEvent to be called every second
  timer.setInterval(1000L, myTimerEvent);
  
  //setup time: Init and get the time (ESP32 internal RTC)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  //run blynk and timer  
  Blynk.run();
  timer.run();

  if ((isFeedingTime() && !stopfeeding) || feednow) {
    if (scale.get_units() >= 20) {
      unsigned long time_now = millis();
      while(millis() < time_now + interval){
          //wait approx. [interval] ms
          Blynk.virtualWrite(V7, "dibatalkan (wadah masih terisi)");
          timer.run();
      }      
    } else {
      unsigned long time_now = millis();
      while(millis() < time_now + interval){
          //wait approx. [interval] ms
          Blynk.virtualWrite(V6, "-");
          Blynk.virtualWrite(V7, "berhasil");
          Blynk.virtualWrite(V1, "Waktunya makanan diberikan");
          myservo.write(open_cover);
          timer.run();
      }
      time_now = millis();
      while(millis() < time_now + interval){
          //wait approx. [interval] ms
          myservo.write(close_cover);
          Blynk.virtualWrite(V1, "Selesai memberikan makanan");
          timer.run();
      }
      Blynk.virtualWrite(V1, "Menunggu waktu makan berikutnya");
      Blynk.virtualWrite(V7, "-");
      stopfeeding = true;
      feednow = false;
    }
    Blynk.virtualWrite(V1, "Menunggu waktu makan berikutnya");
    Blynk.virtualWrite(V7, "-");
    stopfeeding = true;
    feednow = false;
  }
  
  if (!isFeedingTime())  {
    stopfeeding = false;
    checkWadah();
  }

  if (!feednow) {
    checkWadah();    
  }
}

//------------------------------BLYNK CONNECTED----------------------------
BLYNK_CONNECTED() { 
  // when device is conneceted to Blynk Cloud... 
  Blynk.syncAll(); 
  Blynk.virtualWrite(V7, "-");
  checkWadah();
  // request the values for all datastreams that has "sync" setting enabled 
}

//------------------------------FUNCTION TIMER EVENT----------------------------
void myTimerEvent()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  char time_str[80]; 
  strftime(time_str,sizeof(time_str),"%A, %d %B %Y %H:%M:%S",&timeinfo);
  std::string str(time_str);
  Blynk.virtualWrite(V2, time_str);
}

//------------------------------FUNCTION WHEN VIRTUAL PIN STATE CHANGES----------------------------

// This function is called every time the Virtual Pin 3 state changes (feednow)
BLYNK_WRITE(V0) {
  int value = param.asInt();
  if (value==1) {
    feednow = true;
  }
}

// This function is called every time the Virtual Pin 3 state changes (jadwal makan pagi)
BLYNK_WRITE(V3)
{
  // Set incoming value from pin V3 to a variable
  TimeInputParam t(param);
  pagi_hour = t.getStartHour();
  pagi_min = t.getStartMinute();
}

// This function is called every time the Virtual Pin 4 state changes (jadwal makan siang)
BLYNK_WRITE(V4)
{
  // Set incoming value from pin V4 to a variable
  TimeInputParam t(param);
  siang_hour = t.getStartHour();
  siang_min = t.getStartMinute();
}

// This function is called every time the Virtual Pin 5 state changes (jadwal makan malam)
BLYNK_WRITE(V5)
{
  // Set incoming value from pin V5 to a variable
  TimeInputParam t(param);
  malam_hour = t.getStartHour();
  malam_min = t.getStartMinute();
}

//------------------------------OTHER FUNCTION----------------------------

bool isFeedingTime() {
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  bool isfeeding = 0;
  // cek apakah sudah masuk waktu jadwal pemberian makanan
  if ((timeinfo.tm_hour == pagi_hour) && (timeinfo.tm_min == pagi_min)) {
    isfeeding = 1;
  } else if ((timeinfo.tm_hour == siang_hour) && (timeinfo.tm_min == siang_min)) {
    isfeeding = 1;
  } else if ((timeinfo.tm_hour == malam_hour) && (timeinfo.tm_min == malam_min)) {
    isfeeding = 1;
  }

  return isfeeding;
}

void checkWadah() {
  if (scale.get_units() >= 20) {
    Blynk.virtualWrite(V6, "masih terisi (belum dimakan)");      
  } else {
    Blynk.virtualWrite(V6, "kosong (sudah dimakan)");
  }
}