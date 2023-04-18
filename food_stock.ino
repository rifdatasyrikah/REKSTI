/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL603bYUN0L"
#define BLYNK_TEMPLATE_NAME "Food Stock"
#define BLYNK_AUTH_TOKEN "HPF8HIjdlDr3t8bNjTpD9mJzvtnMoDqv"

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

// load cell
#include <Arduino.h>
#include "HX711.h"
#include "soc/rtc.h"

const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;
HX711 scale;

double reading;

void setup()
{
  // Debug console
  Serial.begin(115200);

  // setup load cell
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(-161.015);
  delay(1000);
  scale.tare();               // reset the scale to 0
  Serial.println("scale ready");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);
}

void loop()
{
  Blynk.run();
  timer.run();
}

// FUNCTIONS ------------------------------------------------------------------
// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.asInt();
  
  if (value) {
    scale.power_up();
    reading = (double) scale.get_units();
    Blynk.virtualWrite(V1, reading);
    if (reading <=50.0) {
      Blynk.logEvent("stock_alert", "Stock less than 50g, please restock soon");
    }
    scale.power_down();
  } 
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED() {
  Blynk.syncAll();   
}

BLYNK_WRITE(V1) {
  reading = param.asDouble();
  Serial.println(reading, 1);
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
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