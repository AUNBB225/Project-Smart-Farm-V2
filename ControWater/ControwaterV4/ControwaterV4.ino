#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Wire.h>
#include <RTClib.h>

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
RTC_DS3231 rtc;
//--ประกาศตัวแปลอื่นๆ---
#define relayPin D0 // กำหนดขา D0 สำหรับรีเลย์
int moistureSensor;  // ตัวแปรสำหรับเก็บค่าความชื้นในดิน
int moistureSet; //ัวแปลค่าความชื้นที่กำหนดจากผู้ใช้
#define relayPin2 D3
bool status1 = true; // ประกาศตัวแปร status1 ที่นี่
#define ledPin D4 // กำหนดขาที่ต่อกับไฟ LED

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

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
 }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
   // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

}


bool pumzone1  = false; 
bool pumzone2  = false; 
bool autopum = false;
bool Autotime = false;
bool pumzone1ToSend = false;
bool pumzone2ToSend = false;


void loop() {
Autotime = Firebase.RTDB.getBool(&fbdo, "button/AutoTimeWater") && fbdo.dataType() == "boolean" ? fbdo.boolData() : Autotime;

if(Autotime){
    Serial.println("\n--- Start of loop ---");
  
  server.handleClient();

  DateTime now = rtc.now();
  if (!now.isValid()) {
    Serial.println("Failed to get valid time from RTC!");
    delay(60000);
    return;
  }

  int currentDay = now.dayOfTheWeek();
  int currentHour = now.hour();
  int currentMinute = now.minute();

  Serial.printf("Current time: Day %d, %02d:%02d\n", currentDay, currentHour, currentMinute);

  String dayNames[] = {"sunday", "mondy", "tuesday", "wednesday", "thursday", "friday", "saturday"};
  String currentDayName = dayNames[currentDay];

  String path = "/AutoTime/schedule/" + currentDayName;
  Serial.printf("Checking Firebase path: %s\n", path.c_str());
  
  if (Firebase.RTDB.getJSON(&fbdo, path)) {
    Serial.println("Successfully got data from Firebase");
    FirebaseJson &scheduleJson = fbdo.jsonObject();
    FirebaseJsonData jsonData;

    scheduleJson.get(jsonData, "enabled");
    if (jsonData.success) {
      bool enabled = jsonData.boolValue;
      Serial.printf("Schedule enabled: %s\n", enabled ? "true" : "false");

      if (enabled) {
        scheduleJson.get(jsonData, "times");
        if (jsonData.success && jsonData.typeNum == FirebaseJson::JSON_ARRAY) {
          FirebaseJsonArray timesArray;
          jsonData.get<FirebaseJsonArray>(timesArray);
          size_t arraySize = timesArray.size();
          Serial.printf("Number of scheduled times: %d\n", arraySize);

          for (size_t i = 0; i < arraySize; i++) {
            FirebaseJsonData timeData;
            if (timesArray.get(timeData, i)) {
              if (timeData.typeNum == FirebaseJson::JSON_OBJECT) {
                FirebaseJson timeObject;
                timeData.get<FirebaseJson>(timeObject);

                int scheduleHour = -1, scheduleMinute = -1, duration = -1;
                
                timeObject.get(jsonData, "time");
                if (jsonData.success) {
                  sscanf(jsonData.stringValue.c_str(), "%d:%d", &scheduleHour, &scheduleMinute);
                }
                
                timeObject.get(jsonData, "duration");
                if (jsonData.success) {
                  duration = jsonData.intValue;
                }

                Serial.printf("Scheduled time %d: %02d:%02d, Duration: %d seconds\n", 
                              i, scheduleHour, scheduleMinute, duration);

                if (currentHour == scheduleHour && currentMinute == scheduleMinute) {
                  Serial.println("It's time to water the plants!");
                  waterPlants(duration, now);
                }
              } else {
                Serial.printf("Invalid data type for time %d\n", i);
              }
            } else {
              Serial.printf("Failed to get data for time %d\n", i);
            }
          }
        } else {
          Serial.println("Failed to get or parse 'times' array");
        }
      }
    } else {
      Serial.println("Failed to get 'enabled' status");
    }
  } else {
    Serial.printf("Failed to get data from Firebase, %s\n", fbdo.errorReason().c_str());
  }

  Serial.println("--- End of loop ---");
  delay(1000); // ตรวจสอบทุก 1 วินาที
}
  

autopum = Firebase.RTDB.getBool(&fbdo, "button/automoisture") && fbdo.dataType() == "boolean" ? fbdo.boolData() : autopum;
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
  digitalWrite(relayPin, pumzone1 ?  LOW : HIGH);
}

pumzone2 = Firebase.RTDB.getBool(&fbdo, "button/pumzone2") && fbdo.dataType() == "boolean" ? fbdo.boolData() : pumzone2;
if(fbdo.errorReason() != "") {
  Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason());
} else {
  Serial.println("ค่าของ pumzone2 คือ: " + String(pumzone2));
   digitalWrite(relayPin2, pumzone2 ? LOW : HIGH);
}

status1 = !status1; // สลับค่าของ status1
Firebase.RTDB.setBool(&fbdo, "status/ControPUMWATER", status1);

  if (Firebase.isTokenExpired()) {
  Firebase.refreshToken(&config);
  Serial.println("Refresh token");
}

}

void waterPlants(int duration, DateTime& now) {
  Serial.println("Watering plants...");
  
  digitalWrite(relayPin, LOW);
  Firebase.RTDB.setBool(&fbdo, "button/pumzone1", true);
  digitalWrite(relayPin2, LOW);
  Firebase.RTDB.setBool(&fbdo, "button/pumzone2", true);
  delay(duration * 60000);
  digitalWrite(relayPin, HIGH);
  Firebase.RTDB.setBool(&fbdo, "button/pumzone1", false);
  digitalWrite(relayPin2, HIGH);
  Firebase.RTDB.setBool(&fbdo, "button/pumzone2", false);

  Serial.println("Watering finished.");
}