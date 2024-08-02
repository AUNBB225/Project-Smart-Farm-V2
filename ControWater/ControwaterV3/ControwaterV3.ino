#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

//INC อื่นๆ

// SETข้อมูลสำหรับ Firebase
#define API_KEY "AIzaSyAlzFuhs6xdfMegTzZBFawvVlj9uPibrmo"
#define DATABASE_URL "https://iotfirebase-96f81-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "controlwater@gmail.com"
#define USER_PASSWORD "22062549"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String uid;
//--ประกาศตัวแปลอื่นๆ---
int relayPin = 16;  // กำหนดขา D0 สำหรับรีเลย์
int moistureSensor;  // ตัวแปรสำหรับเก็บค่าความชื้นในดิน
int moistureSet; //ัวแปลค่าความชื้นที่กำหนดจากผู้ใช้
int relayPin2 = 5;
bool status1 = true; // ประกาศตัวแปร status1 ที่นี่
int ledPin = D4; // กำหนดขาที่ต่อกับไฟ LED

//---------------------

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "hello ALL esp8266!");
}

bool shouldSaveConfig = false; // add this line at the top of your code

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
  if (!wifiManager.autoConnect("ControWater")) {
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

void setup() {
  Serial.begin(9600);
  Serial.println("เวอร์ชั่่น 5.0");
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
  pinMode(relayPin, OUTPUT);  // กำหนดขา D0 เป็นขา Output
  pinMode (relayPin2, OUTPUT);
  digitalWrite(relayPin, HIGH);
  digitalWrite(relayPin2, HIGH);
  pinMode(ledPin, OUTPUT); // กำหนดขาเป็น OUTPUT
  digitalWrite(ledPin, HIGH);
}


bool pumzone1  = false; 
bool pumzone2  = false; 
bool autopum = false;
bool pumzone1ToSend = false;
bool pumzone2ToSend = false;


void loop() {

autopum = Firebase.RTDB.getBool(&fbdo, "button/autopum") && fbdo.dataType() == "boolean" ? fbdo.boolData() : autopum;
if(fbdo.errorReason() != "") {
  Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason());
} else {
  Serial.println("ค่าของ autopum คือ: " + String(autopum));
  digitalWrite(ledPin, HIGH);
  if(autopum) {
      digitalWrite(ledPin, LOW);
      pumzone1 = Firebase.RTDB.getBool(&fbdo, "button/pumzone1") && fbdo.dataType() == "boolean" ? fbdo.boolData() : pumzone1;
      if(fbdo.errorReason() != "") {
        Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason());
      } else {
        Serial.println("ค่าของ pumzone1 คือ: " + String(pumzone1));
      }

      pumzone2 = Firebase.RTDB.getBool(&fbdo, "button/pumzone2") && fbdo.dataType() == "boolean" ? fbdo.boolData() : pumzone2;
      if(fbdo.errorReason() != "") {
        Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason());
      } else {
        Serial.println("ค่าของ pumzone2 คือ: " + String(pumzone2));
      }

      moistureSensor = Firebase.RTDB.getInt(&fbdo, "DTHsensor/moisture") && fbdo.dataType() == "int" ? fbdo.intData() : moistureSensor;
      if(fbdo.errorReason() != "") {
        Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason());
      } else {
        Serial.println("ค่าความชื้นในดินที่อ่านได้คือ: " + String(moistureSensor));
      }

      moistureSet = Firebase.RTDB.getInt(&fbdo, "Setmoisture/Setmoisture") && fbdo.dataType() == "int" ? fbdo.intData() : moistureSet;
      if(fbdo.errorReason() != "") {
        Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason());
      } else {
        Serial.println("ค่าความชื้นที่กำหนดจากผู้ใช้คือ: " + String(moistureSet));
      }


    if(moistureSensor < moistureSet) {
      Serial.println("เปิดปั๊ม");
      pumzone1ToSend = true;
      pumzone2ToSend = true;
      Firebase.RTDB.setBool(&fbdo, "button/pumzone1", pumzone1ToSend);
      Firebase.RTDB.setBool(&fbdo, "button/pumzone2", pumzone2ToSend);
      
    } else {
      Serial.println("ปิดปั๊ม");
      pumzone1ToSend = false;
      pumzone2ToSend = false;
      Firebase.RTDB.setBool(&fbdo, "button/pumzone1", pumzone1ToSend);
      Firebase.RTDB.setBool(&fbdo, "button/pumzone2", pumzone2ToSend);
    }
  }
}

pumzone1 = Firebase.RTDB.getBool(&fbdo, "button/pumzone1") && fbdo.dataType() == "boolean" ? fbdo.boolData() : pumzone1;
if(fbdo.errorReason() != "") {
  Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason());
} else {
  Serial.println("ค่าของ pumzone1 คือ: " + String(pumzone1));
  digitalWrite(relayPin, pumzone1 ?  HIGH : LOW);
}

pumzone2 = Firebase.RTDB.getBool(&fbdo, "button/pumzone2") && fbdo.dataType() == "boolean" ? fbdo.boolData() : pumzone2;
if(fbdo.errorReason() != "") {
  Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason());
} else {
  Serial.println("ค่าของ pumzone2 คือ: " + String(pumzone2));
   digitalWrite(relayPin2, pumzone2 ? HIGH : LOW);
}

status1 = !status1; // สลับค่าของ status1
Firebase.RTDB.setBool(&fbdo, "status/ControPUMWATER", status1);

  if (Firebase.isTokenExpired()) {
  Firebase.refreshToken(&config);
  Serial.println("Refresh token");
}

}

