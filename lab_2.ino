#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define LED_PIN_R 0   
#define LED_PIN_Y 14  
#define LED_PIN_G 12  
#define BUTTON    13

ESP8266WebServer server(80);

volatile bool isHolding = false;
bool webToggle = false;

uint32_t lastStepTime = 0;
int8_t stepIndex = 0;
const uint16_t stepDelay = 350;

const int8_t ledsNormal[] = { LED_PIN_R, LED_PIN_Y, LED_PIN_G }; 
const int8_t ledsAlt[]    = { LED_PIN_G, LED_PIN_Y, LED_PIN_R, LED_PIN_Y, LED_PIN_G }; 

void IRAM_ATTR buttonISR() {
  isHolding = (digitalRead(BUTTON) == LOW);
}

void handleRoot() {
  String html = "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{text-align:center; background:#222; color:#fff; font-family:sans-serif;}";
  html += ".btn{width:150px; height:150px; border-radius:50%; border:none; background:#44ff44; color:#000; font-size:20px; font-weight:bold; cursor:pointer;}";
  html += ".btn:active{transform: translateY(4px); background:#22ff22;}</style></head><body>";
  html += "<h2>WEMOS CONTROL</h2>";
  html += "<p>Current sequences:</p>";
  html += "<p>Mode 1: L3-L2-L1-L2-L3</p>";
  html += "<p>Mode 2: L1-L2-L3</p>";
  html += "<button class='btn' onclick=\"fetch('/toggle')\">TOGGLE</button>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleToggle() {
  webToggle = !webToggle;
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN_R, OUTPUT);
  pinMode(LED_PIN_Y, OUTPUT);
  pinMode(LED_PIN_G, OUTPUT);
  
  pinMode(BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonISR, CHANGE);

  WiFi.mode(WIFI_STA);
  WiFi.begin("Redmi Note 10 5G", "123456789");
  
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.begin();
}

void handleWeb() {
  server.handleClient();
}

void handleButton() {
  static bool lastState = false;
  bool currentState = isHolding ^ webToggle; 
  
  if (currentState != lastState) {
    stepIndex = 0;
    lastState = currentState;
    digitalWrite(LED_PIN_R, LOW);
    digitalWrite(LED_PIN_Y, LOW);
    digitalWrite(LED_PIN_G, LOW);
  }
}

void handleLEDs() {
  if (millis() - lastStepTime < stepDelay) return;
  lastStepTime = millis();

  digitalWrite(LED_PIN_R, LOW);
  digitalWrite(LED_PIN_Y, LOW);
  digitalWrite(LED_PIN_G, LOW);

  bool currentState = isHolding ^ webToggle;

  if (currentState) {
    digitalWrite(ledsNormal[stepIndex], HIGH);
    stepIndex = (stepIndex + 1) % 3;
  } else {
    digitalWrite(ledsAlt[stepIndex], HIGH);
    stepIndex = (stepIndex + 1) % 5;
  }
}

void loop() {
  handleWeb();
  handleButton();
  handleLEDs();
}
