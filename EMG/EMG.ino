#include <ESP32Servo.h>

Servo left;
Servo right;
Servo s1;
Servo s2;

int value = 0;

bool isUp = false;

void closeGripper(){
  for (int i = 0; i <= 90; i++){
    left.write(90 - i);
    right.write(90 + i);
    delay(20);
  }
}

void openGripper(){
  for (int i = 0; i <= 90; i++){
    left.write(i);
    right.write(180 - i);
    delay(20);
  }
}

void up(){
  for (int i = 0; i <= 80; i++){
    s1.write(180 - i);
    s2.write(i);
    delay(10);
  }
}

void down(){
  for (int i = 0; i <= 80; i++){
    s1.write(100 + i);
    s2.write(80 - i);
    delay(10);
  }
}

void updateValues(){
  value = analogRead(2);
  Serial.println(value);
}

void setup() {
  Serial.begin(115200);
  updateValues();

  left.attach(4);
  left.write(90);

  right.attach(23);
  right.write(90);

  s1.attach(19);
  s2.attach(21);

  s1.write(180);
  s2.write(0);
}

void loop() {
  updateValues();
  delay(100);

  if (isUp){
    if (value < 200){
      down();
      openGripper();
      isUp = false;
    }
  }
  else{
    if (value > 3000){
      closeGripper();
      up();
      isUp = true;
      delay(2000);
    }
  }
}
