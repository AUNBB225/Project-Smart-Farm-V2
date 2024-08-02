#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SETข้อมูลสำหรับ Firebase
#define API_KEY "AIzaSyAlzFuhs6xdfMegTzZBFawvVlj9uPibrmo"
#define DATABASE_URL "https://iotfirebase-96f81-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "npksensor@gmail.com"
#define USER_PASSWORD "22062549"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String uid;

ESP8266WebServer server(80);


#define RE 2 //pin RE al 8 del MEGA
#define DE 0 //pin DE al 7 del MEGA

const byte nitro[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

int values[7];
SoftwareSerial mod(14, 12); //pin RO al 10 del MEGA y DI al 11 del mega
void handleRoot() {
  server.send(200, "text/plain", "hello ALL esp8266!");
}
//------------------------------
bool shouldSaveConfig = false;
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Initialize WiFi
void initWiFi() {
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  Serial.println("Connecting...");
  if (!wifiManager.autoConnect("NPK Sensor")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("Connected.");

  server.on("/", handleRoot);
  server.begin();
}

void setup()
{
   // Debug console
  Serial.begin(9600);
// เริ่มต้น WiFiManager
  Serial.println("เวอร์ชั่่น 7.0");
  initWiFi();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;
  Firebase.begin(&config, &auth);

  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.print(uid);
  //------------------------------------

  mod.begin(9600);
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);

display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  delay(500);
  display.clearDisplay();
  display.setCursor(25, 15);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Starting...");
  display.setCursor(25, 35);
  display.setTextSize(1);
  display.print("IOT Project");
  display.display();
  delay(3000);

}

bool status1 = true; // ตั้งค่าเริ่มต้นของ val1 เป็น true หรือ false ตามที่คุณต้องการ

void loop()
{
  status1 = status(status1);
  status(status1);
   int val1, val2, val3;
  val1 = nitrogen();
  delay(250);
  val2 = phosphorous();
  delay(250);
  val3 = potassium();
  delay(250);


      if(Firebase.RTDB.setInt(&fbdo, "npksensor/N", val1)){ 
      Serial.println();
       Serial.println(val1);
      Serial.print(" - บันทึกข้อมูลบน Firebase สำเร็จ โดยอยู่ในดาต้าพาท: " + fbdo.dataPath());
      Serial.println("(" + fbdo.dataType() + ")"); 
      status1 = status(status1);

      status(status1);
    }
    else{
      Serial.println("การส่งข้อมูลล้มเหลว: " + fbdo.errorReason()); 
       Firebase.refreshToken(&config);
       status(status1);
    }
      if(Firebase.RTDB.setInt(&fbdo, "npksensor/P", val2)){ 
      Serial.println();
      Serial.println(val2);
      Serial.print(" - บันทึกข้อมูลบน Firebase สำเร็จ โดยอยู่ในดาต้าพาท: " + fbdo.dataPath());
      Serial.println("(" + fbdo.dataType() + ")"); 
      status(status1);
    }
    else{
      Serial.println("การส่งข้อมูลล้มเหลว: " + fbdo.errorReason()); 
       Firebase.refreshToken(&config);
       status(status1);
    }

      if(Firebase.RTDB.setInt(&fbdo, "npksensor/K", val3)){ 
      Serial.println();
      Serial.println(val3);
      Serial.print(" - บันทึกข้อมูลบน Firebase สำเร็จ โดยอยู่ในดาต้าพาท: " + fbdo.dataPath());
      Serial.println("(" + fbdo.dataType() + ")");
      status1 = status(status1);

      status(status1);
    }
    else{
      Serial.println("การส่งข้อมูลล้มเหลว: " + fbdo.errorReason()); 
       Firebase.refreshToken(&config);
       status(status1);
    }

  Serial.print("Nitrogen: ");
  Serial.print(val1);
  Serial.println(" PPM");
  Serial.print("Phosphorous: ");
  Serial.print(val2);
  Serial.println(" PPM");
  Serial.print("Potassium: ");
  Serial.print(val3);
  Serial.println(" PPM");

 display.clearDisplay();
  
 
  display.setTextSize(2);
  display.setCursor(0, 5);
  display.print("N: ");
  display.print(val1);
  display.setTextSize(1);
  display.print(" PPM");
 
  display.setTextSize(2);
  display.setCursor(0, 25);
  display.print("P: ");
  display.print(val2);
  display.setTextSize(1);
  display.print(" PPM");
 
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print("K: ");
  display.print(val3);
  display.setTextSize(1);
  display.print(" PPM");
 
  display.display();
    if (Firebase.isTokenExpired()) {
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }
}

int nitrogen() {
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(1000);
  if (mod.write(nitro, sizeof(nitro)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (int i = 0; i < 7; i++) {
      values[i] = mod.read();
      Serial.print(values[i], HEX);
    }
    Serial.println();
  }
  return (values[3] << 8) | values[4]; // อ่านค่า 16 บิต
}

int phosphorous() {
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(1000);
  if (mod.write(phos, sizeof(phos)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (int i = 0; i < 7; i++) {
      values[i] = mod.read();
      Serial.print(values[i], HEX);
    }
    Serial.println();
  }
   return (values[3] << 8) | values[4]; // อ่านค่า 16 บิต
}

int potassium() {
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(1000);
  if (mod.write(pota, sizeof(pota)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (int i = 0; i < 7; i++) {
      values[i] = mod.read();
      Serial.print(values[i], HEX);
    }
    Serial.println();
  }
  return (values[3] << 8) | values[4]; // อ่านค่า 16 บิต
}



bool status(bool status1) {
  if(Firebase.RTDB.setBool(&fbdo, "status/NPKsensor", status1)){ 
    Serial.println();
    Serial.println(status1);
    Serial.print(" - บันทึกข้อมูลบน Firebase สำเร็จ โดยอยู่ในดาต้าพาท: " + fbdo.dataPath());
    Serial.println("(" + fbdo.dataType() + ")"); 

    // สลับค่าของ val1
    status1 = !status1;
    return status1;
  }
  else{
    Serial.println("การส่งข้อมูลล้มเหลว: " + fbdo.errorReason()); 
    Firebase.refreshToken(&config);
    return status1;
  }
}

