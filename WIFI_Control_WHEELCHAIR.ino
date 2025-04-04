#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "JioFiber-iph8U";
const char* password = "48942109";

// LED pins
#define LED_FORWARD 14
#define LED_BACKWARD 27
#define LED_LEFT 26
#define LED_RIGHT 33

WebServer server(80);

void controlLEDs(int activePin) {
  digitalWrite(LED_FORWARD, activePin == LED_FORWARD ? HIGH : LOW);
  digitalWrite(LED_BACKWARD, activePin == LED_BACKWARD ? HIGH : LOW);
  digitalWrite(LED_LEFT, activePin == LED_LEFT ? HIGH : LOW);
  digitalWrite(LED_RIGHT, activePin == LED_RIGHT ? HIGH : LOW);
}

void stopAllLEDs() {
  digitalWrite(LED_FORWARD, LOW);
  digitalWrite(LED_BACKWARD, LOW);
  digitalWrite(LED_LEFT, LOW);
  digitalWrite(LED_RIGHT, LOW);
}

String getHTML() {
  return R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>ESP32 Remote</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body {
        font-family: Arial, sans-serif;
        background: #222;
        color: white;
        text-align: center;
        margin: 0;
        padding: 0;
      }
      h2 {
        margin: 20px 0;
      }
      .grid {
        display: grid;
        grid-template-columns: 1fr 1fr 1fr;
        gap: 15px;
        justify-items: center;
        align-items: center;
        max-width: 300px;
        margin: auto;
      }
      button {
        background-color: #007bff;
        border: none;
        color: white;
        padding: 25px;
        font-size: 24px;
        border-radius: 12px;
        width: 100%;
        max-width: 90px;
      }
      button:active {
        background-color: #0056b3;
        transform: scale(0.95);
      }
    </style>
  </head>
  <body>
    <h2>ESP32 Remote</h2>
    <div class="grid">
      <div></div>
      <button onclick="sendCommand('/forward')">F</button>
      <div></div>

      <button onclick="sendCommand('/left')">L</button>
      <button onclick="sendCommand('/stop')">S</button>
      <button onclick="sendCommand('/right')">R</button>

      <div></div>
      <button onclick="sendCommand('/backward')">B</button>
      <div></div>
    </div>

    <script>
      function sendCommand(path) {
        fetch(path);
      }
    </script>
  </body>
  </html>
  )rawliteral";
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_FORWARD, OUTPUT);
  pinMode(LED_BACKWARD, OUTPUT);
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
  stopAllLEDs();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() { server.send(200, "text/html", getHTML()); });

  server.on("/forward", []() { controlLEDs(LED_FORWARD); server.send(200, "text/plain", "Forward"); });
  server.on("/backward", []() { controlLEDs(LED_BACKWARD); server.send(200, "text/plain", "Backward"); });
  server.on("/left", []() { controlLEDs(LED_LEFT); server.send(200, "text/plain", "Left"); });
  server.on("/right", []() { controlLEDs(LED_RIGHT); server.send(200, "text/plain", "Right"); });
  server.on("/stop", []() { stopAllLEDs(); server.send(200, "text/plain", "Stop"); });

  server.begin();
}

void loop() {
  server.handleClient();
}
