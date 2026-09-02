#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "arduino_secrets.h"
#define BLYNK_TEMPLATE_ID SECRET_BLYNK_TEMPLATE_ID
#define BLYNK_TEMPLATE_NAME SECRET_BLYNK_TEMPLATE_NAME
#define BLYNK_AUTH_TOKEN SECRET_BLYNK_AUTH_TOKEN
#define soundSensor 35
#define maxDB 3000
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = SECRET_WIFI_SSID;
char pass[] = SECRET_WIFI_PASS;

TinyGPSPlus gps;

HardwareSerial mySerial(2);

Adafruit_MPU6050 mpu;

float angleY = 0;
float initialAngleY = 0;
float gyroBiasY = 0;
unsigned long lastTime = 0;

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

void setup() {
  Serial.begin(115200);
  pinMode(soundSensor, INPUT);

  mySerial.begin(9600, SERIAL_8N1, 17, 16);
  delay(3000);

  Blynk.begin(auth, ssid, pass);
  while (Blynk.connect() == false) {
    Serial.println("Connecting to Blynk...");
    delay(1000);
  }
  Serial.println("Connected to Blynk");

  if (!mpu.begin()) {
    Serial.println("Sensor init failed");
    while (1) yield();
  }

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  calibrateGyro();

  lastTime = millis();

  xTaskCreatePinnedToCore(
    readGyro,
    "Task1",
    10000,
    NULL,
    1,
    &Task1,
    0);

  xTaskCreatePinnedToCore(
    checkSound,
    "Task2",
    10000,
    NULL,
    2,
    &Task2,
    1);

  xTaskCreatePinnedToCore(
    readGPS,
    "Task3",
    10000,
    NULL,
    3,
    &Task3,
    1);
}

void loop() {
}

void readGyro(void * parameter) {
  while (1) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    unsigned long currentTime = millis();
    float dt = (currentTime - lastTime) / 1000.0;

    float gyroY = g.gyro.y - gyroBiasY;
    angleY += gyroY * dt;

    float deg = angleY * (180 / 3.14);

    deg = fmod(deg, 360.0);
    if (deg < 0) {
      deg += 360.0;
    }

    lastTime = currentTime;

    Serial.print("Angle Y: ");
    Serial.println(deg);

    if (deg > 90 && deg <= 270) {
      Serial.println("Your dog has fallen");
      Blynk.virtualWrite(V1, "Your dog has fallen");
    } else {
      Blynk.virtualWrite(V1, "Your dog is upright");
    }

    delay(1000);
  }
}

void checkSound(void * parameter) {
  while (1) {
    int sensorData = analogRead(soundSensor);
    Serial.print("Sound Sensor: ");
    Serial.println(sensorData);

    if (sensorData > maxDB) {
      Serial.println(".................................................................................Too loud");
      Blynk.virtualWrite(V0, "Too loud");
    } else {
      Blynk.virtualWrite(V0, "Acceptable Amount");
    }

    delay(1000);
  }
}

void readGPS(void * parameter) {
  while (1) {
    while (mySerial.available() > 0) {
      char c = mySerial.read();
      Serial.write(c);
      if (gps.encode(c)) {
        displayInfo();
      }
    }

    if (millis() > 5000 && gps.charsProcessed() < 10) {
      Serial.println(F("No GPS detected: check wiring."));
      while (true);
    }

    delay(1000);
  }
}

void displayInfo() {
  Serial.print(F("Location: "));
  if (gps.location.isValid()) {
    Serial.print("Lat: ");
   double lat= gps.location.lat();
   Serial.print(lat);
    Blynk.virtualWrite(V2,lat);
    Serial.print(F(", Lng: "));
   double lng= gps.location.lng();
   Serial.print(lng);
    Blynk.virtualWrite(V3,lng);
    Serial.println();
  } else {
    Serial.print(F("INVALID"));
  }
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    mySerial.write(Serial.read());
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read());
  }
}

void calibrateGyro() {
  const int numReadings = 100;
  float sumY = 0;

  for (int i = 0; i < numReadings; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sumY += g.gyro.y;
    delay(10);
  }

  gyroBiasY = sumY / numReadings;
  Serial.print("Gyro Bias Y: ");
  Serial.println(gyroBiasY);
}
