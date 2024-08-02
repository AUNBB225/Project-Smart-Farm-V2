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
#define USER_EMAIL "controlnpk@gmail.com"
#define USER_PASSWORD "22062549"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String uid;
RTC_DS3231 rtc;
//--ประกาศตัวแปลอื่นๆ---
// ประกาศขาใช้งาน
const int pinD0 = D0;
const int pinD3 = D3;
int ledPin = D4; // กำหนดขาที่ต่อกับไฟ LED

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
  pinMode(pinD0, OUTPUT);
  pinMode(pinD3, OUTPUT);
  
  pinMode(ledPin, OUTPUT);

  // ตั้งค่าเริ่มต้นเป็น LOW
  digitalWrite(pinD0, HIGH);
  digitalWrite(pinD3, HIGH);
  
  digitalWrite(ledPin, HIGH);

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

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


}
bool autotimenpk = false;

//statusPum
bool pumnpk1 = false;
bool pumnpk2 = false;


void loop() {

    autotimenpk = Firebase.RTDB.getBool(&fbdo, "button/AutoTimenpk") && fbdo.dataType() == "boolean" ? fbdo.boolData() : autotimenpk;
    if(autotimenpk){
  Serial.println("\n--- Start of loop ---");
  
  server.handleClient();

  DateTime now = rtc.now();
  if (!now.isValid()) {
    Serial.println("Failed to get valid time from RTC!");
    delay(60000);
    return;
  }

  int currentYear = now.year();
  int currentMonth = now.month();
  int currentDay = now.day();
  int currentHour = now.hour();
  int currentMinute = now.minute();

  Serial.printf("Current time: %04d-%02d-%02d %02d:%02d\n", 
                currentYear, currentMonth, currentDay, currentHour, currentMinute);

  char dateStr[11];
  sprintf(dateStr, "%04d-%02d-%02d", currentYear, currentMonth, currentDay);
  String currentDate = String(dateStr);

  String path = "/AutoTimeNPK/schedule/" + currentDate;
  Serial.printf("Checking Firebase path: %s\n", path.c_str());
  
  // ใช้ Firebase.RTDB.getJSON แทน Firebase.get
  if (Firebase.RTDB.getJSON(&fbdo, path)) {
    Serial.println("Successfully got data from Firebase");
    FirebaseJson &scheduleJson = fbdo.jsonObject();
    FirebaseJsonData jsonData;

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
      Serial.println("No watering times set for today");
    }
  } else {
    Serial.printf("No schedule found for today (%s)\n", currentDate.c_str());
  }

  Serial.println("--- End of loop ---");
  delay(1000); // ตรวจสอบทุก 1 นาที
}


    pumnpk1 = Firebase.RTDB.getBool(&fbdo, "button/pumnpk1") && fbdo.dataType() == "boolean" ? fbdo.boolData() : pumnpk1;
    if(fbdo.errorReason() != "") {
      Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason());
    } else {
      Serial.println("ค่าของ pumnpk1 คือ: " + String(pumnpk1));
      digitalWrite(pinD0, pumnpk1 ? LOW : HIGH);
    }

    pumnpk2 = Firebase.RTDB.getBool(&fbdo, "button/pumnpk2") && fbdo.dataType() == "boolean" ? fbdo.boolData() : pumnpk2;
    if(fbdo.errorReason() != "") {
      Serial.println("การรับข้อมูลล้มเหลว: " + fbdo.errorReason());
    } else {
      Serial.println("ค่าของ pumnpk2 คือ: " + String(pumnpk2));
      digitalWrite(pinD3, pumnpk2 ? LOW : HIGH);
    }


status1 = !status1; // สลับค่าของ status1
Firebase.RTDB.setBool(&fbdo, "status/ControPUMNPK", status1);

  if (Firebase.isTokenExpired()) {
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }

}


void waterPlants(int duration, DateTime& now) {
  Serial.println("Watering plants...");
  digitalWrite(pinD3, LOW);
  digitalWrite(pinD0, LOW);
  Firebase.RTDB.setBool(&fbdo, "button/pumnpk1", true);
  Firebase.RTDB.setBool(&fbdo, "button/pumnpk2", true);
  delay(duration * 1000);
  digitalWrite(pinD3, HIGH);
  digitalWrite(pinD0, HIGH);
  Firebase.RTDB.setBool(&fbdo, "button/pumnpk1", false);
  Firebase.RTDB.setBool(&fbdo, "button/pumnpk2", false);
  Serial.println("Watering finished.");
}
