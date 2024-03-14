#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>


const char * FIREBASE_HOST = "research3-95ab0-default-rtdb.asia-southeast1.firebasedatabase.app";
const char * FIREBASE_AUTH = "XiIxxZKxK5jV7An71bmTnRf71fORdRkNkiXbjbAm";
const String FIREBASE_PATH = "/"; 
const int SSL_PORT = 443;
//dont touch this pls :3
const char* ssid = "Your wifi name";
const char* password = "passwordbs";
x

#define RXD2 16
#define TXD2 17
HardwareSerial neogps(2);
TinyGPSPlus gps;

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

#define OLED_RESET -1 
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiClient wifiClient;
HttpClient http_client = HttpClient(wifiClient, FIREBASE_HOST, SSL_PORT);

unsigned long previousMillis = 0;
long interval = 10000;
boolean newData = false; 
FirebaseData fbdo;
FirebaseData fbdo2;
FirebaseData fbdo3;
FirebaseConfig config;

void setup() {
  Serial.begin(9600);
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

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  config.api_key = FIREBASE_AUTH;
  config.database_url = FIREBASE_HOST;

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  if(Firebase.ready()) {
    Serial.println("Firebase is ready!");
  }
  delay(2000); 
  display.clearDisplay(); 
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    if(!Firebase.ready()) {
      Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
      delay(1000);
    } else {
      gps_loop(); 
    }

  } else {
    Serial.println("WiFi not connected. Reconnecting...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi reconnected");
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
  newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;) {
    while (neogps.available()) {
      if (gps.encode(neogps.read())) {
        newData = true;
        break;
      }
    }
  }

  if (newData) {
    newData = false;
    print_speed();

    char lat[20], lng[20], spd[20];
    dtostrf(gps.location.lat(), 10, 6, lat);
    dtostrf(gps.location.lng(), 10, 6, lng);
    dtostrf(gps.speed.kmph(), 10, 6, spd);
    const char * latitude = lat;
    const char * longitude = lng;
    const char * speed = spd;

    Serial.print("Latitude= ");
    Serial.println(latitude);
    Serial.print(" Longitude= ");
    Serial.print(longitude);
    Serial.print(" Speed= ");
    Serial.print(speed);
    Serial.println(" km/h");


    Firebase.setString(fbdo, "/lat", latitude);
    Firebase.setString(fbdo2, "/lng", longitude);
    Firebase.setString(fbdo3, "/speed", speed);
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(3);
    display.print("alfritz asa ka");
    display.display();
  }
}
