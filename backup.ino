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

// timer ---------------------------------------------
BlynkTimer timer;
BlynkTimer timer2;
unsigned long interval = 10000;
unsigned long end; 
unsigned long start;

//servo motor -----------------------------------------
#include <Servo.h>

Servo myservo;  // create servo object to control a servo
const int pin_servo = 13;
const int close_cover = 180;
const int open_cover = 0;

void setup() {
  Serial.begin(115200);

  //setup servo motor
  myservo.attach(pin_servo);  // attaches the servo on pin 13 to the servo object
  myservo.write(close_cover);
  
  //setup blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // Setup a function myTimerEvent to be called every second
  timer.setInterval(1000L, myTimerEvent);
  // Setup a function myTimerEvent2 to be called every 5 second
  timer.setInterval(5000L, myTimerEvent2);
  
  //setup time: Init and get the time (ESP32 internal RTC)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // printLocalTime();
}

void loop() {
  //run blynk and timer  
  Blynk.run();
  timer.run();
  timer2.run();

  // untuk debug
  // Serial.print("is feeding time?");
  // Serial.println(isFeedingTime());
  // Serial.print("is stop feeding?");
  // Serial.println(stopfeeding);
  
  if (isFeedingTime() && stopfeeding==false) {
    // Blynk.virtualWrite(V1, "Waktunya makanan diberikan");
    // myservo.write(open_cover);
    // unsigned long time_now = millis();
    // while(millis() < time_now + interval){
    //     //wait approx. [interval] ms
    // }
    // myservo.write(close_cover);
    // Blynk.virtualWrite(V1, "Selesai memberikan makanan");
    // time_now = millis();
    // while(millis() < time_now + interval){
    //     //wait approx. [interval] ms
    // }
    // Blynk.virtualWrite(V1, "Menunggu waktu makan berikutnya");
    // stopfeeding = true;
    
    // previousMiliis = millis();
    start = millis()+1;
    end = millis();
    stopfeeding = true; 
  }

  end = millis();
    if (start <= end && end<start+interval) {
      Blynk.virtualWrite(V1, "Waktunya makanan diberikan");
      myservo.write(open_cover);
    } else if (start+interval <= end && end<start+(interval*2)) {
      Blynk.virtualWrite(V1, "Selesai memberikan makanan");
      myservo.write(close_cover);
    } else if (end>=start+(interval*2)){
      Blynk.virtualWrite(V1, "Menunggu waktu makan berikutnya");
      // stopfeeding = true; 
    } else {
      // Blynk.virtualWrite(V1, "Menunggu waktu makan berikutnya");
      // stopfeeding = true;
      // start = millis();
    }
  
  if (!isFeedingTime())  {
    stopfeeding = false;
  }
}

//------------------------------BLYNK CONNECTED----------------------------
BLYNK_CONNECTED() { 
  // when device is conneceted to Blynk Cloud... 
  Blynk.syncAll(); 
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

void myTimerEvent2()
{
  //untuk debug saja
  Serial.print("Makan Pagi= ");
  Serial.print(pagi_hour);
  Serial.print(":");
  Serial.println(pagi_min);
  Serial.print("Makan Siang= ");
  Serial.print(siang_hour);
  Serial.print(":");
  Serial.println(siang_min);
  Serial.print("Makan Malam= ");
  Serial.print(malam_hour);
  Serial.print(":");
  Serial.println(malam_min);
  Serial.println();
}

//------------------------------FUNCTION WHEN VIRTUAL PIN STATE CHANGES----------------------------

// This function is called every time the Virtual Pin 3 state changes (jadwal makan pagi)
BLYNK_WRITE(V3)
{
  // Set incoming value from pin V3 to a variable
  TimeInputParam t(param);
  pagi_hour = t.getStartHour();
  pagi_min = t.getStartMinute();

  //untuk debug
  Serial.print("Update waktu Makan Pagi= ");
  Serial.print(pagi_hour);
  Serial.print(":");
  Serial.println(pagi_min);
  Serial.println();
}

// This function is called every time the Virtual Pin 4 state changes (jadwal makan siang)
BLYNK_WRITE(V4)
{
  // Set incoming value from pin V4 to a variable
  TimeInputParam t(param);
  siang_hour = t.getStartHour();
  siang_min = t.getStartMinute();

  //untuk debug
  Serial.print("Update waktu Makan Siang= ");
  Serial.print(siang_hour);
  Serial.print(":");
  Serial.println(siang_min);
  Serial.println();
}

// This function is called every time the Virtual Pin 5 state changes (jadwal makan malam)
BLYNK_WRITE(V5)
{
  // Set incoming value from pin V5 to a variable
  TimeInputParam t(param);
  malam_hour = t.getStartHour();
  malam_min = t.getStartMinute();

  //untuk debug
  Serial.print("Update waktu Makan Malam= ");
  Serial.print(malam_hour);
  Serial.print(":");
  Serial.println(malam_min);
  Serial.println();
}

//------------------------------OTHER FUNCTION----------------------------

bool isFeedingTime() {
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  bool isfeeding = 0;
  // cek apakah sudah masuk waktu jadwal pemberian makanan

  // untuk debug
  // Serial.println("current time= ");
  //   Serial.println(timeinfo.tm_hour);
  //   Serial.println(timeinfo.tm_min);
  //   Serial.println("jadwal siang= ");
  //   Serial.println(siang_hour);
  //   Serial.println(siang_min);
  //   Serial.print("apakah sama? ");
  //   Serial.println(timeinfo.tm_hour==siang_hour);
  //   Serial.println();
  
  if ((timeinfo.tm_hour == pagi_hour) && (timeinfo.tm_min == pagi_min)) {
    isfeeding = 1;
  } else if ((timeinfo.tm_hour == siang_hour) && (timeinfo.tm_min == siang_min)) {
    isfeeding = 1;
  } else if ((timeinfo.tm_hour == malam_hour) && (timeinfo.tm_min == malam_min)) {
    isfeeding = 1;
  }

  return isfeeding;
}

// void printLocalTime(){
//   struct tm timeinfo;
//   if(!getLocalTime(&timeinfo)){
//     Serial.println("Failed to obtain time");
//     return;
//   }
//   Serial.println(&timeinfo, "%A, %d %B %Y %H:%M:%S");
//   //%A=Monday, %B=April  %d=17 %Y=2023 %H:%M:%S=10:40:24  
// }
