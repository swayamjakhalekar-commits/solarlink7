// ======================================================
// SERVO INIT TEST
// Horizontal → 0° then +20° = 20°  (pin 9)
// Vertical   → 90° then +20° = 110° (pin 10)
// ======================================================

#include <Servo.h>

Servo servoH;
Servo servoV;

const int CENTER_V = 90;
int currentV = 0;

void setup() {

  servoH.attach(9);
  servoV.attach(10);

  // Init position
  servoH.write(0);
  servoV.write(CENTER_V + currentV);  // 90 + 0 = 90

  delay(2000);  // Hold init position for 2 seconds

  // Move both by +20 degrees
  currentV = 20;

  servoH.write(20);
  servoV.write(CENTER_V + currentV);  // 90 + 20 = 110

}

void loop() {
  // Hold position — nothing to do
}