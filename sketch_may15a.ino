#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>

// WiFi Credentials
const char* ssid = "";
const char* password = "";
WebServer server(80);

// Hardware Pins
const uint16_t kRecvPin = 15;
const uint16_t kBlasterPin = 5;

// CRITICAL: Huge buffer to support Air Conditioners
const uint16_t kCaptureBufferSize = 1024;
const uint8_t kTimeout = 50; 

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
IRsend irsend(kBlasterPin);
decode_results results;

// Universal Storage variables
uint16_t storedRawData[1024];
uint16_t storedRawLength = 0;

void handleRoot() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{text-align:center; font-family:Arial; margin-top:40px;} ";
  html += "button{padding:20px; font-size:20px; background:#007bff; color:white; border:none; border-radius:8px; cursor:pointer;} ";
  html += ".status{margin:20px; font-size:18px; color:#333;}</style></head><body>";
  
  html += "<h1>Universal IR Cloner</h1>";
  
  if (storedRawLength > 0) {
    html += "<div class='status'>✅ Signal Stored! Ready to transmit.</div>";
    html += "<form action='/transmit' method='POST'><button type='submit'>Transmit Signal 📡</button></form>";
  } else {
    html += "<div class='status'>⚠️ No signal stored. Point a remote at the ESP32 and press a button!</div>";
    html += "<button disabled style='background:#ccc;'>Waiting for IR...</button>";
  }
  
  html += "<br><br><a href='/'>Refresh Page</a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleTransmit() {
  if (storedRawLength > 0) {
    // Blind the receiver so it doesn't hear its own blast
    irrecv.disableIRIn();
    
    // Blast the stored signal 3 times, exactly like your old code
      irsend.sendRaw(storedRawData, storedRawLength, 38);
      delay(100);
    
    
    Serial.println("Web Blast Sent 3x!");
    irrecv.enableIRIn();
  }
  
  // Redirect back to main page
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  irsend.begin();
  irrecv.enableIRIn();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP Address:");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/transmit", HTTP_POST, handleTransmit);
  server.begin();
}

void loop() {
  server.handleClient();

  // Listen for ANY incoming IR signal and overwrite the stored buffer
  if (irrecv.decode(&results)) {
    uint16_t* rawArray = resultToRawArray(&results);
    storedRawLength = getCorrectedRawLength(&results);
    
    // Copy the captured signal into our permanent storage array
    for (int i = 0; i < storedRawLength; i++) {
      storedRawData[i] = rawArray[i];
    }
    
    delete[] rawArray; // Clean up memory
    
    Serial.print("New IR Signal Captured! Length: ");
    Serial.println(storedRawLength);
    
    irrecv.resume();
  }
}