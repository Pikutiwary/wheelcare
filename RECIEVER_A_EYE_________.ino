#include <NRFLite.h>

const static uint8_t RADIO_ID = 0;  // Receiver ID
const static uint8_t PIN_RADIO_CE = 8;
const static uint8_t PIN_RADIO_CSN = 9;

struct RadioPacket {
    uint8_t FromRadioId;
    uint32_t Data;
    uint32_t FailedTxCount;
};

NRFLite _radio;
RadioPacket _radioData;

void setup() {
    Serial.begin(9600);

    if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN)) {
        Serial.println("NRF24L01 failed to initialize");
        while (1);
    }
}

void loop() {
    if (_radio.hasData()) {
        _radio.readData(&_radioData);
        
        Serial.print("Received from ");
        Serial.print(_radioData.FromRadioId);
        Serial.print(": ");
        Serial.println(_radioData.Data);

        // Use _radioData.Data to control motors or any output
    }
}
