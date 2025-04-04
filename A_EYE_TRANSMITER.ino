#include <NRFLite.h>

const int xPin = A0;
const int yPin = A1;

float zeroX = 1.65;
float zeroY = 1.65;

const float sensitivity = 0.330;
const float Vref = 3.3;
const int ADC_Resolution = 1024;
const float tiltThreshold = 0.2;

const static uint8_t RADIO_ID = 1;
const static uint8_t DESTINATION_RADIO_ID = 0;
const static uint8_t PIN_RADIO_CE = 8;
const static uint8_t PIN_RADIO_CSN = 9;

struct RadioPacket {
    uint8_t FromRadioId;
    uint32_t Data;
    uint32_t FailedTxCount;
};

NRFLite _radio;
RadioPacket _radioData;

unsigned long forwardTiltStartTime = 0;
bool forwardTimerRunning = false;
const unsigned long requiredTiltDuration = 5000; // 5 seconds

void setup() {
    Serial.begin(9600);

    if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN)) {
        Serial.println("NRF24L01 failed to initialize");
        while (1);
    }

    // Calibrate GY-61
    zeroX = (analogRead(xPin) * Vref) / ADC_Resolution;
    zeroY = (analogRead(yPin) * Vref) / ADC_Resolution;

    Serial.println("Calibration Done!");
    delay(2000);
}

void loop() {
    int rawX = analogRead(xPin);
    int rawY = analogRead(yPin);

    float voltageX = (rawX * Vref) / ADC_Resolution;
    float voltageY = (rawY * Vref) / ADC_Resolution;

    float accelX = (voltageX - zeroX) / sensitivity;
    float accelY = (voltageY - zeroY) / sensitivity;

    int message = 0; // Default: No movement

    // Handle forward tilt with timer
    if (accelY > tiltThreshold) {
        if (!forwardTimerRunning) {
            forwardTiltStartTime = millis();
            forwardTimerRunning = true;
        }

        if (millis() - forwardTiltStartTime >= requiredTiltDuration) {
            message = 1; // Forward only after 5 seconds
        }
    } else {
        forwardTimerRunning = false; // Reset timer if tilt is lost
    }

    // // Backward
    // if (accelY < -tiltThreshold) {
    //     message = 2;
    //     forwardTimerRunning = false; // Cancel forward if backward detected
    // }
    // // Left
    // else if (accelX > tiltThreshold) {
    //     message = 3;
    //     forwardTimerRunning = false;
    // }
    // // Right
    // else if (accelX < -tiltThreshold) {
    //     message = 4;
    //     forwardTimerRunning = false;
    // }

    // Send message
    _radioData.FromRadioId = RADIO_ID;
    _radioData.Data = message;
    _radioData.FailedTxCount = 0;

    if (_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData))) {
        Serial.print("Sent: ");
        Serial.println(message);
    } else {
        Serial.println("Transmission Failed");
    }

    delay(200);
}
