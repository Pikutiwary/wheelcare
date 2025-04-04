#include <VL53L0X.h>

VL53L0X sensor1;
VL53L0X sensor2;

const int ledPin1 = 11; 
const int ledPin2 = 12; 

int MinD = 150; // mm 
int MaxD = 250; // mm
int ledDelay = 2000;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Initialize Sensor 1
  sensor1.init();
  sensor1.setAddress(0x30); // Set address for sensor 1

  // Initialize Sensor 2
  sensor2.init();
  sensor2.setAddress(0x31); // Set address for sensor 2

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
}

void loop() {
  // Read distance from sensor 1
  int distance1 = sensor1.readRangeSingleMillimeters();
  Serial.print("Sensor 1: ");
  Serial.print(distance1);
  Serial.println(" mm");

  if (distance1 > MaxD) {
    digitalWrite(ledPin1, HIGH);
    delay(ledDelay);
    digitalWrite(ledPin1, LOW);
    delay(ledDelay);
  }

  // Read distance from sensor 2
  int distance2 = sensor2.readRangeSingleMillimeters();
  Serial.print("Sensor 2: ");
  Serial.print(distance2);
  Serial.println(" mm");

  if (distance2 > MaxD) {
    digitalWrite(ledPin2, HIGH);
    delay(ledDelay);
    digitalWrite(ledPin2, LOW);
    delay(ledDelay);
  }

  delay(500); // Wait 1 second before next loop
}
