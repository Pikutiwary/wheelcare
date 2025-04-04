#include <SPI.h>
#include <NRFLite.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
// Function prototypes (declarations)
void Stop();
void none();
void forward();
void back();
void left();
void right();
void forward2();
void back2();
void left2();
void right2();
void forwardleft();
void forwardright();
void backleft();
void backright();
void joystickControl();
void bluetoothControl();
void gyroControl();
void wifiControl();
void moveAccordingly();
void triggerSOS();
void updateDisplay();
void autopilot();

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin definitions
int x_pin = A0;
int y_pin = A1;
int sw_pin = A2;  // Joystick push button pin

int leftMotorForward = 2;
int leftMotorBackward = 3;
int rightMotorForward = 4;
int rightMotorBackward = 5;

int modeButtonPin1 = 6;  // Button pin for mode switching
int modeButtonState1 = 0;
int lastModeButtonState1 = 0;

int signalPin1 = 7;   // New signal pin 1
int signalPin2 = 8;   // New signal pin 2
int signalPin3 = 9;   // New signal pin 3
int signalPin4 = 10;  // New signal pin 4

int AEB1 = 24;
int AEB2 = 25;

int buttonState = 0;
int lastButtonState = 0;

int buttonState2 = 0;
int lastButtonState2 = 0;

// Mode booleans
bool joystickMode = true;
bool bluetoothMode = false;
bool gyroMode = false;
bool wifiMode = false;
bool autopilotMode = false;
int count = 0;
bool autopilotFirstTime = true;
unsigned long autopilotStartTime = 0;

boolean x = 0;
int directionOfMovement = 0;

int command;
int Speed = 255;
int Speedsec;
int Turnradius = 0;
int brakeTime = 45;
int brkonoff = 1;

String message;
String latitude, longitude;
bool newData = false;
char phone_no[] = "+918340642828";
TinyGPSPlus gps;
String textMessage;

const static uint8_t RADIO_ID = 0;
const static uint8_t PIN_RADIO_CE = 11;
const static uint8_t PIN_RADIO_CSN = 12;

struct RadioPacket {
  uint8_t FromRadioId;
  uint32_t Data;
  uint32_t FailedTxCount;
};

NRFLite _radio;
RadioPacket _radioData;

// WIFI mode pin definitions
int wifiForwardPin = 38;   // Forward when HIGH
int wifiBackwardPin = 39;  // Backward when HIGH
int wifiLeftPin = 40;      // Left when HIGH
int wifiRightPin = 41;     // Right when HIGH

// Use hardware serial ports for GSM and GPS modules
#define Gsm Serial1
#define Gps Serial2

void setup() {
  Serial.begin(9600);
  Gsm.begin(9600);
  Gps.begin(9600);

  pinMode(leftMotorForward, OUTPUT);
  pinMode(leftMotorBackward, OUTPUT);
  pinMode(rightMotorForward, OUTPUT);
  pinMode(rightMotorBackward, OUTPUT);

  // Signal pins setup
  pinMode(signalPin1, INPUT);
  pinMode(signalPin2, INPUT);
  pinMode(signalPin3, INPUT);
  pinMode(signalPin4, INPUT);

  pinMode(x_pin, INPUT);
  pinMode(y_pin, INPUT);
  pinMode(sw_pin, INPUT);
  pinMode(modeButtonPin1, INPUT);

  digitalWrite(sw_pin, HIGH);          // Enable pull-up resistor
  digitalWrite(modeButtonPin1, HIGH);    // Enable pull-up resistor

  // Setup WIFI mode pins
  pinMode(wifiForwardPin, INPUT);
  pinMode(wifiBackwardPin, INPUT);
  pinMode(wifiLeftPin, INPUT);
  pinMode(wifiRightPin, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("DISPLAY CONNECTION");
    display.println("PROBLEM");
    display.display();
    for (;;)
      ;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("Joystick Mode"));
  display.display();

  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN)) {
    Serial.println("Cannot communicate with radio");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("CANNOT COMMUNICATE");
    display.println("WITH - RADIO");
    display.display();
    while (1)
      ;
  }

  Gsm.print("AT+CMGF=1\r");
  delay(100);
  Gsm.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
  //..............................
  pinMode(AEB1, INPUT);
  pinMode(AEB2, INPUT);
}

void loop() {
  buttonState = digitalRead(sw_pin);
  if (buttonState == LOW) {
    triggerSOS();  // Call triggersos() function when the button is pressed
    updateDisplay();
  }
  //..............................................................................

  // Check emergency braking pins
  if (digitalRead(AEB1) == HIGH) {  // front collision pin
    Stop();
    count++;
    if (joystickMode) {
      int x_data = analogRead(x_pin);
      if (x_data <= 300) {
        back();
        Serial.println("Back AEB");
      } else {
        Stop();
      }
    } else if (gyroMode) {
      if (directionOfMovement == 2) {
        back();
      } else {
        none();
      }
    } else if (bluetoothMode) {
      command = Serial.read();
      if (command == 'B') {
        back2();
      } else {
        Stop();
      }
    } else if (wifiMode) {
      // For WIFI mode, you may decide to implement a similar safety behavior.
      // Here we simply stop in case of emergency.
      Stop();
    } else if (autopilotMode && count > 0) {
      right();
      delay(1500);
      Stop();
      forward();
      delay(3000);
      Stop();
      left();
      delay(1500);
      Stop();
      count--;
    }
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Emergency Brake!"));
    display.println(F("Only Backward Allowed"));
    display.display();
    Serial.println("Emergency Braking: Only Backward Allowed");
    delay(500);  // Prevent rapid toggling
    return;
  }

  if (digitalRead(AEB2) == HIGH) {
    Stop();
    if (joystickMode) {
      int x_data = analogRead(x_pin);
      if (x_data >= 900) {
        forward();
        Serial.println("Front AEB");
      } else {
        Stop();
      }
    } else if (gyroMode) {
      if (directionOfMovement == 1) {
        forward();
      } else {
        none();
      }
    } else {
      command = Serial.read();
      if (command == 'F') {
        forward2();
      } else {
        none();
      }
    }
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Emergency Brake!"));
    display.println(F("Only Forward Allowed"));
    display.display();
    Serial.println("Emergency Braking: Only Forward Allowed");
    delay(500);
    return;
  }

  updateDisplay();

  //...........................................................................
  modeButtonState1 = digitalRead(modeButtonPin1);
  if (digitalRead(signalPin4) == HIGH || digitalRead(signalPin3) == HIGH) {
    Stop();
    forward();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Back Way"));
    display.println(F("Not Clear!"));
    display.display();
    Serial.println("Back way not clear");
    delay(2000);
    Stop();
    updateDisplay();
    return;
  }
  // Signal pin checks
  else if (digitalRead(signalPin1) == HIGH || digitalRead(signalPin2) == HIGH) {
    Stop();
    back();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Front Way"));
    display.println(F("Not Clear!"));
    display.display();
    Serial.println("Front way not clear");
    delay(2000);
    Stop();
    updateDisplay();
    return;
  }
  else {
    // Toggle mode when modeButtonPin1 is pressed
    if (modeButtonState1 == LOW && lastModeButtonState1 == HIGH) {
      Serial.println("Button 1 pressed");
      if (joystickMode) {
        // Switch from Joystick -> Bluetooth
        joystickMode = false;
        bluetoothMode = true;
        gyroMode = false;
        wifiMode = false;
        autopilotMode = false;
      } else if (bluetoothMode) {
        // Switch from Bluetooth -> Gyro
        bluetoothMode = false;
        gyroMode = true;
      } else if (gyroMode) {
        // Switch from Gyro -> WIFI
        gyroMode = false;
        wifiMode = true;
      } else if (wifiMode) {
        // Switch from WIFI -> Autopilot
        wifiMode = false;
        autopilotMode = true;
        autopilotStartTime = millis();
        autopilotFirstTime = true;
      } else if (autopilotMode) {  
        // Switch from Autopilot -> Joystick
        autopilotMode = false;
        joystickMode = true;
      }
      updateDisplay();
      delay(300);  // Debounce delay
    }
    lastModeButtonState1 = modeButtonState1;

    if (joystickMode) {
      Serial.println("Joystick function called!");
      joystickControl();
    } else if (bluetoothMode) {
      Serial.println("Bluetooth function called!");
      bluetoothControl();
    } else if (gyroMode) {
      Serial.println("Gyro function called!");
      while (_radio.hasData()) {
        _radio.readData(&_radioData);
        message = _radioData.Data;
        Serial.println(message);
        directionOfMovement = message.toInt();
        moveAccordingly();
      }
    } else if (wifiMode) {
      Serial.println("WIFI function called!");
      wifiControl();
    } else if (autopilotMode) {
      if (autopilotFirstTime) {
        if (millis() - autopilotStartTime >= 20000) {  // Wait for 20 seconds non-blocking
          Serial.println("Entering AUTO-PILOT MODE...");
          autopilotFirstTime = false;
        } else {
          return;  // Wait until 20 seconds pass before executing autopilot
        }
      }
      autopilot();
    }
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  if (joystickMode) {
    display.println(F("Joystick Mode"));
  } else if (bluetoothMode) {
    display.println(F("Bluetooth Mode"));
  } else if (gyroMode) {
    display.println(F("Gyro Mode"));
  } else if (wifiMode) {
    display.println(F("WIFI Mode"));
  } else if (autopilotMode) {
    display.println(F("AUTO-PILOT Mode !!!"));
  }
  display.display();
}

void autopilot() {
  forward();
}

void joystickControl() {
  int x_data = analogRead(x_pin);
  int y_data = analogRead(y_pin);

  if (x_data > 400 && x_data < 800 && y_data > 400 && y_data < 800) {
    none();
    Serial.println("None");
  } else if (x_data <= 300) {
    back();
    Serial.println("Back");
  } else if (x_data >= 900) {
    forward();
    Serial.println("Front");
  } else if (y_data <= 300) {
    left();
    Serial.println("Left");
  } else if (y_data >= 900) {
    right();
    Serial.println("Right");
  } else {
    none();
    Serial.println("Invalid Data - Stopping Motor");
  }
}

void bluetoothControl() {
  if (Serial.available() > 0) {
    command = Serial.read();
    Stop();
    switch (command) {
      case 'F':
        forward2();
        break;
      case 'B':
        back2();
        break;
      case 'L':
        left2();
        break;
      case 'R':
        right2();
        break;
      case 'G':
        forwardleft();
        break;
      case 'I':
        forwardright();
        break;
      case 'H':
        backleft();
        break;
      case 'J':
        backright();
        break;
      case '0':
        Speed = 100;
        break;
      case '1':
        Speed = 140;
        break;
      case '2':
        Speed = 153;
        break;
      case '3':
        Speed = 165;
        break;
      case '4':
        Speed = 178;
        break;
      case '5':
        Speed = 191;
        break;
      case '6':
        Speed = 204;
        break;
      case '7':
        Speed = 216;
        break;
      case '8':
        Speed = 229;
        break;
      case '9':
        Speed = 242;
        break;
      case 'q':
        Speed = 255;
        break;
    }
  }
}

void wifiControl() {
  // In WIFI mode, check the assigned pins for motion commands
  if (digitalRead(wifiForwardPin) == HIGH) {
    forward();
    Serial.println("WIFI Forward");
  } else if (digitalRead(wifiBackwardPin) == HIGH) {
    back();
    Serial.println("WIFI Backward");
  } else if (digitalRead(wifiLeftPin) == HIGH) {
    left();
    Serial.println("WIFI Left");
  } else if (digitalRead(wifiRightPin) == HIGH) {
    right();
    Serial.println("WIFI Right");
  } else {
    none();
    Serial.println("WIFI None");
  }
}

void moveAccordingly() {
  if (directionOfMovement == 1) {
    forward();
  } else if (directionOfMovement == 2) {
    back();
  } else if (directionOfMovement == 3) {
    left();
  } else if (directionOfMovement == 4) {
    right();
  } else if (directionOfMovement == 0) {
    none();
  }
}

void forward() {
  analogWrite(leftMotorForward, Speed);
  analogWrite(rightMotorForward, Speed);
}

void back() {
  digitalWrite(leftMotorBackward, HIGH);
  digitalWrite(rightMotorBackward, HIGH);
  digitalWrite(leftMotorForward, LOW);
  digitalWrite(rightMotorForward, LOW);
}

void left() {
  digitalWrite(rightMotorForward, HIGH);
  digitalWrite(leftMotorBackward, HIGH);
  digitalWrite(leftMotorForward, LOW);
  digitalWrite(rightMotorBackward, LOW);
}

void right() {
  digitalWrite(leftMotorForward, HIGH);
  digitalWrite(rightMotorBackward, HIGH);
  digitalWrite(leftMotorBackward, LOW);
  digitalWrite(rightMotorForward, LOW);
}

void none() {
  digitalWrite(leftMotorForward, LOW);
  digitalWrite(rightMotorForward, LOW);
  digitalWrite(leftMotorBackward, LOW);
  digitalWrite(rightMotorBackward, LOW);
}

void forward2() {
  analogWrite(leftMotorForward, Speed);
  analogWrite(rightMotorForward, Speed);
}

void back2() {
  analogWrite(leftMotorBackward, Speed);
  analogWrite(rightMotorBackward, Speed);
}

void left2() {
  analogWrite(rightMotorForward, Speed);
  analogWrite(leftMotorBackward, Speed);
}

void right2() {
  analogWrite(rightMotorBackward, Speed);
  analogWrite(leftMotorForward, Speed);
}

void forwardleft() {
  analogWrite(leftMotorForward, Speedsec);
  analogWrite(rightMotorForward, Speed);
}

void forwardright() {
  analogWrite(leftMotorForward, Speed);
  analogWrite(rightMotorForward, Speedsec);
}

void backright() {
  analogWrite(leftMotorBackward, Speed);
  analogWrite(rightMotorBackward, Speedsec);
}

void backleft() {
  analogWrite(leftMotorBackward, Speedsec);
  analogWrite(rightMotorBackward, Speed);
}

void Stop() {
  analogWrite(leftMotorForward, 0);
  analogWrite(leftMotorBackward, 0);
  analogWrite(rightMotorForward, 0);
  analogWrite(rightMotorBackward, 0);
}

void triggerSOS() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("E-M-E-R-G-E-N-C-Y !!"));
  display.println(F("SENDING S_O_S ...."));
  display.println(F("((  H-E-L-P  ))"));
  display.display();
  delay(1000);
  Serial.println("SOS triggered!");

  for (unsigned long start = millis(); millis() - start < 1000;) {
    if (Gps.available()) {
      char c = Gps.read();
      Serial.print(c);
      if (gps.encode(c))
        newData = true;
    }
  }

  if (Gsm.available() > 0) {
    textMessage = Gsm.readString();
    textMessage.toUpperCase();
    delay(10);
  }

  if (gps.location.isValid()) {
    latitude = String(gps.location.lat(), 6);
    longitude = String(gps.location.lng(), 6);
    Serial.println("................");
    Serial.println("Latitude= ");
    Serial.print(latitude);
    Serial.println("Longitude= ");
    Serial.print(longitude);

    Gsm.print("AT+CMGF=1\r");
    delay(400);
    Gsm.print("AT+CMGS=\"");
    Gsm.print(phone_no);
    Gsm.println("\"");
    Gsm.println("Alert I need help.............");
    Gsm.print("http://maps.google.com/maps?q=loc:");
    Gsm.print(latitude);
    Gsm.print(",");
    Gsm.print(longitude);
    delay(200);
    Gsm.println((char)26);
    delay(200);
    Gsm.println();
    Serial.println("SMS Sent");
    Serial.println("Call");
    delay(20000);
    Gsm.println("ATD+918340642828;");
    Gsm.println("ATH");
    delay(1000);
  } else {
    Serial.println("Invalid GPS data. Waiting for valid fix...");
    Gsm.print("AT+CMGF=1\r");
    delay(400);
    Gsm.print("AT+CMGS=\"");
    Gsm.print(phone_no);
    Gsm.println("\"");
    Gsm.println("E-M-E-R-G-E-N-C-Y!! Alert I need help....");
    delay(200);
    Gsm.println((char)26);
    delay(200);
    Gsm.println();
    Serial.println("SMS Sent");
    Serial.println("Call");
    delay(20000);
    Gsm.println("ATD+918340642828;");
    Gsm.println("ATH");
    delay(1000);
  }
}

void brakeOn() {
  buttonState = command;
  if (buttonState != lastButtonState) {
    if (buttonState == 'S') {
      digitalWrite(leftMotorForward, HIGH);
      digitalWrite(leftMotorBackward, HIGH);
      digitalWrite(rightMotorForward, HIGH);
      digitalWrite(rightMotorBackward, HIGH);
      delay(brakeTime);
      Stop();
    }
    lastButtonState = buttonState;
  }
}

void brakeOff() {
  // No action needed for brakeOff
}
