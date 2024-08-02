#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN D6          // กำหนดขาข้อมูลเข้าจากเซ็นเซอร์ DHT11
#define DHTTYPE DHT11      // ประเภทของเซ็นเซอร์ DHT11

DHT dht(DHTPIN, DHTTYPE);  // สร้างอ็อบเจกต์ DHT
int soilMoistureValue = 0;
int levelSoil1 = 0;
bool status1 = true; // ประกาศตัวแปร status1 ที่นี่

// SETข้อมูลสำหรับ Firebase
#define API_KEY "AIzaSyAlzFuhs6xdfMegTzZBFawvVlj9uPibrmo"
#define DATABASE_URL "https://iotfirebase-96f81-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "dth@gmail.com"
#define USER_PASSWORD "22062549"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String uid;

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
  if (!wifiManager.autoConnect("All")) {
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
  Serial.begin(115200);
  Serial.println("เวอร์ชั่น 1.0");
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
  Serial.println(uid);

  pinMode(A0, INPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);

  dht.begin();
}

// ฟังก์ชันอ่านค่าความชื้นในดินและอุณหภูมิ/ความชื้นจาก DHT11
void readSensors() {
  // อ่านค่าความชื้นในดิน
  soilMoistureValue = analogRead(A0);
  float levelSoil = (soilMoistureValue * (100.0 / 1024.0)) - 100;
  levelSoil1 = round(levelSoil * (-1));

  Serial.print("ความชื้นในดิน: ");
  Serial.println(levelSoil1);

  // อ่านค่าความชื้นและอุณหภูมิจากเซ็นเซอร์ DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // ตรวจสอบว่าการอ่านค่า DHT11 สำเร็จหรือไม่
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("ไม่สามารถอ่านค่าจาก DHT11");
    return;
  }

  int intHumidity = round(humidity);
  int intTemperature = round(temperature);


  Firebase.RTDB.setInt(&fbdo, "DTHsensor/humidity", intHumidity);
  Firebase.RTDB.setInt(&fbdo, "DTHsensor/temperature", intTemperature);
  // ในส่วนของส่งค่าอุณหภูมิในอากาศไปที่ Firebase

  Serial.print("Humidity: ");
  Serial.print(intHumidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(intTemperature);
  Serial.println(" °C");
}

void loop() {
  // ในส่วนของส่งค่าความชื้นในดินไปที่ Firebase
  Firebase.RTDB.setInt(&fbdo, "DTHsensor/moisture", levelSoil1);
  status1 = !status1; // สลับค่าของ status1
Firebase.RTDB.setBool(&fbdo, "status/statusDTH", status1);
  readSensors();
  delay(2000); // อ่านค่าเซ็นเซอร์ทุกๆ 2 วินาที
}


