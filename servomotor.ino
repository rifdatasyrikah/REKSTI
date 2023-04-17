#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

void setup() {
  myservo.attach(13);  // attaches the servo on pin 13 to the servo object
  
  Serial.begin(115200);
  pos = 0;
}

void loop() {

  myservo.write(0); // nutup
  myservo.write(180); //buka
}
