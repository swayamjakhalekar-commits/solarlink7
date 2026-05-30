// ======================================================
// SERVO INIT TEST
// Horizontal → 0°  (pin 9)
// Vertical   → CENTER_V + 0 = 90° (pin 10, upright)
// ======================================================

#include <Servo.h>

Servo servoH;
Servo servoV;

const int CENTER_V = 90;
int currentV = 0;

void setup() {

  servoH.attach(9);
  servoV.attach(10);

  servoH.write(0);
  servoV.write(CENTER_V + currentV);  // 90 + 0 = 90

}

void loop() {
  // Hold position — nothing to do
}