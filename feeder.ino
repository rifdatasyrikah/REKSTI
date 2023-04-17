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

BlynkTimer timer;

#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

// int pos = 0;    // variable to store the servo position

void setup() {
  myservo.attach(13);  // attaches the servo on pin 13 to the servo object
  
  Serial.begin(115200);
  //awal ditutup
  myservo.write(180);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  // Setup a function myTimerEvent to be called every second
  timer.setInterval(1000L, myTimerEvent);
}

void loop() {

  //run blynk  
  Blynk.run();
  timer.run();

  struct tm timeinfo;
  getLocalTime(&timeinfo);

  //belum ditambah kondisi hasil pembacaan load cell
  if ((timeinfo.tm_hour == hour_feeding) && (timeinfo.tm_min == minute_feeding)) {
    Blynk.virtualWrite(V1, "Feeder dibuka, makanan dikeluarkan");
    myservo.write(0);
  } 

  bool stopfeeding = 1;
  // stopfeeding adalah kondisi selesai mengeluarkan makanan --> bisa berdasarkan pembacaan load cell mungkin
  if (stopfeeding) {
    Blynk.virtualWrite(V1, "Feeder ditutup, makanan selesai dikeluarkan");
    myservo.write(180);
  }

}

void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  char time_str[80]; 
  strftime(time_str,sizeof(time_str),"%A, %d %B %Y %H:%M:%S",&timeinfo);
  std::string str(time_str);

  // std::cout << str;

  Blynk.virtualWrite(V2, time_str);
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %d %B %Y %H:%M:%S");
  //%A=Monday, %B=April  %d=17 %Y=2023 %H:%M:%S=10:40:24  
}
