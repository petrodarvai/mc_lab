#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define L1 2U
#define L2 14U
#define L3 12U
#define BUTTON 13U

ESP8266WebServer server(80);

volatile bool pressEvent = false;
volatile bool locked = false;

bool directionForward = true;
bool running = false;
uint32_t lastStepTime = 0 int8_t stepIndex = 0;

const int8_t leds[3] = { L1, L2, L3 };
const uint16_t stepDelay = 300;

void IRAM_ATTR buttonISR() {
  if (!locked) {
    pressEvent = true;
    locked = true;
  }
}

void handleRoot() {
  String html = "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{text-align:center; background:#222; color:#fff; font-family:sans-serif;}";
  html += ".btn{width:150px; height:150px; border-radius:50%; border:none; background:#ff4444; color:#fff; font-size:20px; font-weight:bold; cursor:pointer; box-shadow: 0 8px #991111; touch-action: manipulation;}";
  html += ".btn:active{box-shadow: 0 2px #991111; transform: translateY(4px); background:#ff2222;}</style></head><body>";
  html += "<h2>WEMOS CONTROL</h2>";
  html += "<button class='btn' onclick=\"sendPress()\">PRESS</button>";
  html += "<script>function sendPress(){fetch('/press').catch(e=>{});}</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handlePress() {
  pressEvent = true;
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(74880);

  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  digitalWrite(L1, LOW);
  digitalWrite(L2, LOW);
  digitalWrite(L3, LOW);

  pinMode(BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonISR, FALLING);

  WiFi.mode(WIFI_STA);
  WiFi.begin("Redmi Note 10 5G", "123456789");

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\nIP: " + WiFi.localIP().toString());
  server.on("/", handleRoot);
  server.on("/press", handlePress);
  server.begin();
}


void runAlgorithm() {
  if (millis() - lastStepTime < stepDelay) return;
  lastStepTime = millis();

  digitalWrite(L1, LOW);
  digitalWrite(L2, LOW);
  digitalWrite(L3, LOW);

  int index = directionForward ? stepIndex : (2 - stepIndex);
  digitalWrite(leds[index], HIGH);

  stepIndex = (stepIndex + 1) % 3;
}


void algoHandler(bool flag_run) {
  if (flag_run) runAlgorithm();
}

void loop() {
  server.handleClient();

  if (locked && digitalRead(BUTTON) == HIGH) {
    static uint32_t lastReleaseTime = 0;
    if (millis() - lastReleaseTime > 50) {
      locked = false;
      lastReleaseTime = millis();
    }
  }

  if (pressEvent) {
    pressEvent = false;
    directionForward = !directionForward;
    running = true;
    stepIndex = 0;
    lastStepTime = 0;
    Serial.println("Action!");
  }


  algoHandler(running);
}