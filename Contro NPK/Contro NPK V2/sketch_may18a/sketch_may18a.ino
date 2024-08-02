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
#define USER_EMAIL "controlnpk@gmail.com"
#define USER_PASSWORD "22062549"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String uid;
//--ประกาศตัวแปลอื่นๆ---
// ประกาศขาใช้งาน
const int pinD1 = D1;
const int pinD6 = D2;
const int pinD7 = D7;
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
  if (!wifiManager.autoConnect("ControNPK")) {
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

    // กำหนดขาเป็น OUTPUT
  pinMode(pinD1, OUTPUT);
  pinMode(pinD6, OUTPUT);
  pinMode(pinD7, OUTPUT);

  // ตั้งค่าเริ่มต้นเป็น LOW
  digitalWrite(pinD1, HIGH);
  digitalWrite(pinD6, HIGH);
  digitalWrite(pinD7, HIGH);

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
  
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.print(uid);
  //------------------------------------

}
bool autonpk = false;
int DatasetN = 0; // ประกาศตัวแปร DatasetN เป็นตัวแปรแบบ Global
int DatasetP = 0;
int DatasetK = 0;

//ค่าจาก sensorNPK
int N = 0;
int P = 0;
int K = 0;

//สถาณะ
bool statusN = false;
bool statusP = false;
bool statusK = false;

//statusPum
bool pumN = false;
bool pumP = false;
bool pumK = false;

void loop() {
  static bool oldAutonpk = false;
  static int oldDatasetN = 0;
  static int oldDatasetP = 0;
  static int oldDatasetK = 0;
  static int oldN = 0;
  static int oldP = 0;
  static int oldK = 0;
  static bool oldPumN = false;
  static bool oldPumP = false;
  static bool oldPumK = false;
  static bool oldStatusN = false;
  static bool oldStatusP = false;
  static bool oldStatusK = false;

  // การรับข้อมูลจากไฟล์เบส autonpk
  if(Firebase.RTDB.getBool(&fbdo, "button/autonpk")){ 
    if(fbdo.dataType()=="boolean"){ 
      autonpk = fbdo.boolData(); 
      //Serial.println(autonpk);
       
    }
  }
  else{
    Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
  }

  if (autonpk == true) {
    // การรับข้อมูลจากไฟล์เบส setN
    if(Firebase.RTDB.getInt(&fbdo, "Setnpk/SetN")){ 
      if(fbdo.dataType()=="int"){ 
        DatasetN = fbdo.intData(); 
        //Serial.println(DatasetN);
      }
    }
    else{
      Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
    }

    // การรับข้อมูลจากไฟล์เบส setP
    if(Firebase.RTDB.getInt(&fbdo, "Setnpk/SetP")){ 
      if(fbdo.dataType()=="int"){ 
        DatasetP = fbdo.intData(); 
        //Serial.println(DatasetP);
      }
    }
    else{
      Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
    }

    // การรับข้อมูลจากไฟล์เบส setK
    if(Firebase.RTDB.getInt(&fbdo, "Setnpk/SetK")){ 
      if(fbdo.dataType()=="int"){ 
        DatasetK = fbdo.intData(); 
        //Serial.println(DatasetK);
      }
    }
    else{
      Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
    }

//------------------------------------------------------------------------------------------------------------------------------

      // การรับข้อมูลจากไฟล์เบส sensorN
    if(Firebase.RTDB.getInt(&fbdo, "npksensor/N")){ 
      if(fbdo.dataType()=="int"){ 
        N = fbdo.intData(); 
        //Serial.println(N);
      }
    }
    else{
      Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
    }

        // การรับข้อมูลจากไฟล์เบส sensorP
       if(Firebase.RTDB.getInt(&fbdo, "npksensor/P")){ 
      if(fbdo.dataType()=="int"){ 
        P = fbdo.intData(); 
       // Serial.println(P);
      }
    }
    else{
      Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
    }

        // การรับข้อมูลจากไฟล์เบส sensorK
    if(Firebase.RTDB.getInt(&fbdo, "npksensor/K")){ 
      if(fbdo.dataType()=="int"){ 
        K = fbdo.intData(); 
        //Serial.println(K);
      }
    }
    else{
      Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
    }

  }

  //---------------------------------------------------

      if(Firebase.RTDB.getBool(&fbdo, "button/pumN")){ 
        if(fbdo.dataType()=="boolean"){ 
          pumN = fbdo.boolData(); 
           
        }
      }
      else{
        Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
      }

      if(Firebase.RTDB.getBool(&fbdo, "button/pumP")){ 
        if(fbdo.dataType()=="boolean"){ 
          pumP = fbdo.boolData(); 
           
        }
      }
      else{
        Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
      }

      if(Firebase.RTDB.getBool(&fbdo, "button/pumK")){ 
        if(fbdo.dataType()=="boolean"){ 
          pumK = fbdo.boolData(); 
           
        }
      }
      else{
        Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason()); //กรณีที่ไม่สามารถรับข้อมูลจากไฟล์เบสได้ให้ขึ้นข้อความที่ถูกตั้งไว้บนไฟล์เบส
      }


  //pumN
    if (pumN == true) {
    digitalWrite(pinD1, LOW);
  } else if (pumN == false) {
    digitalWrite(pinD1, HIGH);
  }
  //pumP
    if (pumP == true) {
    digitalWrite(pinD6, LOW);
  } else if (pumP == false) {
    digitalWrite(pinD6, HIGH);
  }

  //pumK
    if (pumK == true) {
    digitalWrite(pinD7, LOW);
  } else if (pumK == false) {
    digitalWrite(pinD7, HIGH);
  }

  status1 = status(status1); 

  if (autonpk == true) {

    if (N < DatasetN) {
      Serial.print("เปิดการทำงาน N");
      statusN = true; // อัปเดต statusN ตามที่คุณต้องการ
      if (statusN != oldStatusN) {
        sendstatusN(statusN);
        oldStatusN = statusN;
      }
      status1 = status(status1); 

    } else if (N > DatasetN) {
      Serial.print("ปิดการทำงาน N");
      statusN = false; // อัปเดต statusN ตามที่คุณต้องการ
      if (statusN != oldStatusN) {
        sendstatusN(statusN);
        oldStatusN = statusN;
      }
      status1 = status(status1); 
    }
    //-----------
    if (P < DatasetP) {
      Serial.print("เปิดการทำงาน P");
      statusP = true; // อัปเดต statusP ตามที่คุณต้องการ
      if (statusP != oldStatusP) {
        sendstatusP(statusP);
        oldStatusP = statusP;
      }
      status1 = status(status1); 

    } else if (P > DatasetP) {
      Serial.print("ปิดการทำงาน P");
      statusP = false; // อัปเดต statusP ตามที่คุณต้องการ
      if (statusP != oldStatusP) {
        sendstatusP(statusP);
        oldStatusP = statusP;
      }
      status1 = status(status1); 
    }
    //-----------
    if (K < DatasetK) {
      Serial.print("เปิดการทำงาน K");
      statusK = true; // อัปเดต statusN ตามที่คุณต้องการ
      if (statusK != oldStatusK) {
        sendstatusK(statusK);
        oldStatusK = statusK;
      }
      status1 = status(status1); 
    } else if (K > DatasetK) {
      Serial.print("ปิดการทำงาน K");
      statusK = false; // อัปเดต statusN ตามที่คุณต้องการ
      if (statusK != oldStatusK) {
        sendstatusK(statusK);
        oldStatusK = statusK;
      }
      status1 = status(status1); 
    }
    //-----------

  }

  if (Firebase.isTokenExpired()) {
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
    status1 = status(status1); 
  }
}

void sendstatusN(bool statusN) {
  if(Firebase.RTDB.setBool(&fbdo, "button/pumN", statusN)){ 
    status1 = status(status1); 
  }
  else{
    Serial.println("การส่งข้อมูลล้มเหลว: " + fbdo.errorReason()); 
    Firebase.refreshToken(&config);
  }
}

void sendstatusP(bool statusP) {
  if(Firebase.RTDB.setBool(&fbdo, "button/pumP", statusP)){ 
    status1 = status(status1);  
  }
  else{
    Serial.println("การส่งข้อมูลล้มเหลว: " + fbdo.errorReason()); 
    Firebase.refreshToken(&config);
  }
}

void sendstatusK(bool statusK) {
  if(Firebase.RTDB.setBool(&fbdo, "button/pumK", statusK)){ 
    status1 = status(status1); 
  }
  else{
    Serial.println("การส่งข้อมูลล้มเหลว: " + fbdo.errorReason()); 
    Firebase.refreshToken(&config);
  }
}

bool status(bool status1) {
  if(Firebase.RTDB.setBool(&fbdo, "status/ControPUMNPK", status1)){ 
    status1 = !status1;
    return status1;
  }
  else{
    Serial.println("การส่งข้อมูลล้มเหลว: " + fbdo.errorReason()); 
    Firebase.refreshToken(&config);
    return status1;
  }
}
