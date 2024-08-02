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
  Serial.println("เวอร์ชั่่น 4.0");
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
}


bool pumzone1  = false; 
bool pumzone2  = false; 
bool autopum = false;
void loop() {
status1 = status(status1);
      if(Firebase.RTDB.getBool(&fbdo, "button/pumzone1")){ 
      if(fbdo.dataType()=="boolean"){ 
        pumzone1 = fbdo.boolData(); 
        status1 = status(status1);
         
      }
    }
    else{
      Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
    }

          if(Firebase.RTDB.getBool(&fbdo, "button/pumzone2")){ 
      if(fbdo.dataType()=="boolean"){ 
        pumzone2 = fbdo.boolData(); 
        status1 = status(status1);
         
      }
    }
    else{
      Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
    }

  if(Firebase.RTDB.getBool(&fbdo, "button/autopum")){ 
    if(fbdo.dataType()=="boolean"){ 
    autopum = fbdo.boolData(); 
    status1 = status(status1);
  }
}else{
  Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
  status1 = status(status1);
}

  if (autopum == true){
      if(Firebase.RTDB.getInt(&fbdo, "Setmoisture/Setmoisture")){ 
       if(fbdo.dataType()=="int"){ 
      moistureSet = fbdo.intData(); 
      status1 = status(status1);
      
    }
  }
  else{
    Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
    status1 = status(status1);
  }

        if(Firebase.RTDB.getInt(&fbdo, "DTHsensor/moisture")){ 
       if(fbdo.dataType()=="int"){ 
      moistureSensor = fbdo.intData(); 
      status1 = status(status1);
      
    }
  }
  else{
    Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
    status1 = status(status1);
  }

}


  if(Firebase.RTDB.getBool(&fbdo, "button/autopum")){ 
    if(fbdo.dataType()=="boolean"){ 
    autopum = fbdo.boolData(); 
    status1 = status(status1);
  }
}else{
  Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
  status1 = status(status1);
}


  if (pumzone1 == true) {
  digitalWrite(relayPin, LOW);
  status1 = status(status1);
} else if (pumzone1 == false) {
  digitalWrite(relayPin, HIGH);
  status1 = status(status1);
}

  if (pumzone2 == true) {
  digitalWrite(relayPin2, LOW);
  status1 = status(status1);
} else if (pumzone2 == false) {
  digitalWrite(relayPin2, HIGH);
  status1 = status(status1);
}

    if(autopum == true){
      
      if(moistureSensor < moistureSet){
      Serial.print("เปิดการทำงาน pumzone...");
      status1 = status(status1);
      pumzone1 = true; 
      pumzone2 = true; 
      sendpumzone1(pumzone1);
      sendpumzone2(pumzone2);

      } else if(moistureSensor > moistureSet){
      Serial.print("ปิดการทำงาน pumzone...");
      status1 = status(status1);
      pumzone1 = false;
      pumzone2 = false;
      sendpumzone1(pumzone1);
      sendpumzone2(pumzone2);
      }

    }
status1 = status(status1);
if (Firebase.isTokenExpired()) {
  Firebase.refreshToken(&config);
  Serial.println("Refresh token");
  }

}

void sendpumzone1(bool pumzone1) {
  if(Firebase.RTDB.setBool(&fbdo, "button/pumzone1", pumzone1)){ 

    //Serial.print("ส่งสถานะ"); 
    //Serial.println(statusK); 
    status1 = status(status1);

  }
  else{
    Serial.println("การส่งข้อมูลล้มเหลว: " + fbdo.errorReason()); 
    status1 = status(status1); 
  }
}

void sendpumzone2(bool pumzone2) {
  if(Firebase.RTDB.setBool(&fbdo, "button/pumzone2", pumzone2)){ 

    //Serial.print("ส่งสถานะ"); 
    //Serial.println(statusK); 
    status1 = status(status1); 

  }
  else{
    Serial.println("การส่งข้อมูลล้มเหลว: " + fbdo.errorReason()); 
    status1 = status(status1); 
  }
}

bool status(bool status1) {
  if(Firebase.RTDB.setBool(&fbdo, "status/ControPUMWATER", status1)){ 
    Serial.println();
    Serial.println(status1);

    // สลับค่าของ status1
    status1 = !status1;
    return status1;
  }
  else{
    Serial.println("การส่งข้อมูลล้มเหลว: " + fbdo.errorReason()); 
    Firebase.refreshToken(&config);
    return status1;
  }
}
   
   
