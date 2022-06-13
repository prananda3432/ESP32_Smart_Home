#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>
#include <PZEM004Tv30.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "time.h"

#define WIFI_SSID "BlockH2/13"
#define WIFI_PASSWORD "Wifiyangmana"
#define API_KEY "AIzaSyCLJ5XeI7E-5IfPPYh5kE3VV7mhmnEANRk"
#define DATABASE_URL "logical-seat-314215-default-rtdb.asia-southeast1.firebasedatabase.app"
#define USER_EMAIL "prananda3432@gmail.com"
#define USER_PASSWORD "CobaProject123"
#define DHTPin 32
#define DHTTYPE DHT22
#define RXD2 21
#define TXD2 22

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
DHT dht(DHTPin, DHTTYPE);
PZEM004Tv30 pzem(&Serial2, RXD2, TXD2);

String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
String timeStamp, weekDay, currentMonthName, dateStamp;
int monthDay, currentMonth, currentYear;
unsigned long epochTime;

boolean smoke_detected;
String Port1, Port2, Port3, Port4, temp, RH, rtdb_voltage, rtdb_current, rtdb_power, rtdb_energy;
int gas_detected, pzem_voltage;
float t, h, pzem_current, pzem_power, pzem_energy;

const byte smoke_pin = 34;
const byte gas_pin = 35;
const byte buzzer_pin = 13;
const byte Relay_1 = 4;
const byte Relay_2 = 16;
const byte Relay_3 = 17;
const byte Relay_4 = 5;

void setup() {
  Serial.begin(115200);
  
  pinMode(smoke_pin, INPUT);
  pinMode(gas_pin, INPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(Relay_1, OUTPUT);
  pinMode(Relay_2, OUTPUT);
  pinMode(Relay_3, OUTPUT);
  pinMode(Relay_4, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  dht.begin();

  timeClient.begin();
  timeClient.setTimeOffset(28800);
}

void loop() {
  timeClient.update();
  epochTime = timeClient.getEpochTime();
  timeStamp = timeClient.getFormattedTime();
  tm *ptm = gmtime ((time_t *)&epochTime);
  weekDay = weekDays[timeClient.getDay()];
  monthDay = ptm->tm_mday;
  currentMonth = ptm->tm_mon+1;
  currentMonthName = months[currentMonth-1];
  currentYear = ptm->tm_year+1900;
  dateStamp = (weekDay + ", " + monthDay + " " + currentMonthName + " " + currentYear);
  Firebase.RTDB.setString(&fbdo, "/NTP/Time", timeStamp);
  Firebase.RTDB.setString(&fbdo, "/NTP/Date", dateStamp);
  Serial.println(timeStamp);
  Serial.println(dateStamp);
  
  temp = "";
  RH = "";
  rtdb_voltage = "";
  rtdb_current = "";
  rtdb_power = "";
  rtdb_energy = "";
  
  smoke_detected = digitalRead(smoke_pin);
  gas_detected = analogRead(gas_pin);
  t = dht.readTemperature();
  h = dht.readHumidity();
  pzem_voltage = pzem.voltage() - 4;
  pzem_current = pzem.current();
  pzem_power = pzem.power();
  pzem_energy = pzem.energy();

  Serial.println(smoke_detected);
  Serial.println(gas_detected);
  
  if (smoke_detected == false){
    Serial.println("Smoke Detected");
    digitalWrite(buzzer_pin, LOW);
    Firebase.RTDB.setInt(&fbdo, "/MQ-2/Smoke", 1);
  }
  else if(gas_detected >= 600){
    Serial.println("Gas Detected");
    Firebase.RTDB.setInt(&fbdo, "/MQ-2/Gas", 1);
  }
  else {
    Serial.println("No Smoke or Gas Detected"); 
    digitalWrite(buzzer_pin, HIGH);
    Firebase.RTDB.setInt(&fbdo, "/MQ-2/Smoke", 0);
    Firebase.RTDB.setInt(&fbdo, "/MQ-2/Gas", 0);
  }

  temp.concat(t);
  RH.concat(h);
  Firebase.RTDB.setString(&fbdo, "/DHT-22/Temperature", temp);
  Firebase.RTDB.setString(&fbdo, "/DHT-22/Humidity", RH);

  rtdb_voltage.concat(pzem_voltage);
  rtdb_current.concat(pzem_current);
  rtdb_power.concat(pzem_power);
  rtdb_energy.concat(pzem_energy);
  Firebase.RTDB.setString(&fbdo, "/PZEM-004T/Voltage", rtdb_voltage);
  Firebase.RTDB.setString(&fbdo, "/PZEM-004T/Current", rtdb_current);
  Firebase.RTDB.setString(&fbdo, "/PZEM-004T/Power", rtdb_power);
  Firebase.RTDB.setString(&fbdo, "/PZEM-004T/Energy", rtdb_energy);

  if (Firebase.RTDB.getString(&fbdo, "/Relay/Port_1")) {
    Port1 = fbdo.stringData();
    if (Port1 == "ON") {
      digitalWrite(Relay_1, HIGH);
      Serial.println("Relay 1 ON");
    } 
    else {
      digitalWrite(Relay_1, LOW);
      Serial.println("Relay 1 OFF");
    }
  }

  if (Firebase.RTDB.getString(&fbdo, "/Relay/Port_2")) {
    Port2 = fbdo.stringData();
    if (Port2 == "ON") {
      digitalWrite(Relay_2, HIGH);
      Serial.println("Relay 2 ON");
    } 
    else {
      digitalWrite(Relay_2, LOW);
      Serial.println("Relay 2 OFF");
    }
  }

  if (Firebase.RTDB.getString(&fbdo, "/Relay/Port_3")) {
    Port1 = fbdo.stringData();
    if (Port1 == "ON") {
      digitalWrite(Relay_3, HIGH);
      Serial.println("Relay 3 ON");
    } 
    else {
      digitalWrite(Relay_3, LOW);
      Serial.println("Relay 3 OFF");
    }
  }

  if (Firebase.RTDB.getString(&fbdo, "/Relay/Port_4")) {
    Port1 = fbdo.stringData();
    if (Port1 == "ON") {
      digitalWrite(Relay_4, HIGH);
      Serial.println("Relay 4 ON");
    } 
    else {
    digitalWrite(Relay_4, LOW);
    Serial.println("Relay 4 OFF");
    }
  }
}
