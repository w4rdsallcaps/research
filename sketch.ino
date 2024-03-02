#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>

// Your Firebase credentials
const char FIREBASE_HOST[] = "https://researchgroup3-f1f0d-default-rtdb.asia-southeast1.firebasedatabase.app";
const String FIREBASE_AUTH = "pT7IOiB2IrLmtlTMsYyX7AZzggc8Jyv418zdC8vn";
const String FIREBASE_PATH = "/";
const int SSL_PORT = 443;

// Your Wi-Fi credentials
const char* ssid = "Montaus hotspot";
const char* password = "akoangwifininoval26";

// GPS Module pins
#define RXD2 16
#define TXD2 17
HardwareSerial neogps(2);
TinyGPSPlus gps;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//On ESP32: GPIO-21(SDA), GPIO-22(SCL)
#define OLED_RESET -1 //Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C //See datasheet for Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// HTTP client setup
WiFiClient wifiClient;
HttpClient http_client = HttpClient(wifiClient, FIREBASE_HOST, SSL_PORT);

unsigned long previousMillis = 0;
long interval = 10000;
boolean newData = false; // Declaration of newData variable

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("neogps serial initialize");

  http_client.setHttpResponseTimeout(90 * 1000);

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000); // Pause for 2 seconds
  display.clearDisplay(); // Clear the display
}

void loop() {
  http_client.connect(FIREBASE_HOST, SSL_PORT);

  while (true) {
    if (!http_client.connected()) {
      Serial.println();
      http_client.stop(); // Shutdown
      Serial.println("HTTP not connect");
      break;
    }
    else {
      gps_loop();
    }
  }
}

void PostToFirebase(const char* method, const String & path , const String & data, HttpClient* http) {
  String response;
  int statusCode = 0;
  http->connectionKeepAlive();

  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += path + ".json";
  url += "?auth=" + FIREBASE_AUTH;
  Serial.print("POST:");
  Serial.println(url);
  Serial.print("Data:");
  Serial.println(data);

  String contentType = "application/json";
  http->put(url, contentType, data);

  statusCode = http->responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  response = http->responseBody();
  Serial.print("Response: ");
  Serial.println(response);

  if (!http->connected()) {
    Serial.println();
    http->stop(); // Shutdown
    Serial.println("HTTP POST disconnected");
  }
}

void print_speed()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
       
  if (gps.location.isValid() == 1)
  {
    display.setTextSize(1);
    
    display.setCursor(25, 5);
    display.print("Lat: ");
    display.setCursor(50, 5);
    display.print(gps.location.lat(),6);

    display.setCursor(25, 20);
    display.print("Lng: ");
    display.setCursor(50, 20);
    display.print(gps.location.lng(),6);

    display.setCursor(25, 35);
    display.print("Speed: ");
    display.setCursor(65, 35);
    display.print(gps.speed.kmph());
    
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("SAT:");
    display.setCursor(25, 50);
    display.print(gps.satellites.value());

    display.setTextSize(1);
    display.setCursor(70, 50);
    display.print("ALT:");
    display.setCursor(95, 50);
    display.print(gps.altitude.meters(), 0);

    display.display();
    
  }
  else
  {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(3);
    display.print("No Data");
    display.display();
  }  

}

void gps_loop() {
  // No need to redeclare newData here
  // boolean newData = false; // Remove this line
  
  for (unsigned long start = millis(); millis() - start < 2000;) {
    while (neogps.available()) {
      if (gps.encode(neogps.read())) {
        // You can directly use the newData variable declared in the loop() function
        newData = true;
        break;
      }
    }
  }

  if (newData) {
    newData = false;

    String latitude = String(gps.location.lat(), 6);
    String longitude = String(gps.location.lng(), 6);
    String speed = String(gps.speed.kmph(), 2); // Convert speed to kilometers per hour

    Serial.print("Latitude= ");
    Serial.print(latitude);
    Serial.print(" Longitude= ");
    Serial.print(longitude);
    Serial.print(" Speed= ");
    Serial.print(speed);
    Serial.println(" km/h");

    String gpsData = "{";
    gpsData += "\"lat\":" + latitude + ",";
    gpsData += "\"lng\":" + longitude + ",";
    gpsData += "\"speed\":" + speed + "";
    gpsData += "}";

    PostToFirebase("PATCH", FIREBASE_PATH, gpsData, &http_client);

    // Display GPS data on OLED screen
    print_speed();
  }
}
