 
#define BLYNK_TEMPLATE_ID "TMPLssYeqgqE"
#define BLYNK_DEVICE_NAME "iothuerto"
#define BLYNK_AUTH_TOKEN "ntwlwrtC7VYfQ1ugoaLGGCAOn3cB1npw"
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "FirebaseESP8266.h"
WiFiClient client;

// Wi-Fi settings
const char auth[] = "ntwlwrtC7VYfQ1ugoaLGGCAOn3cB1npw";
const char ssid[] = "Alex";//
const char pass[] = "YOSOYTUPADRE";

//Red Tec
//Tec-IoT
//spotless.magnetic.bridge

//Red Casa
//INFINITUMC8D9_2.4
//7713821045

// Firebase
#define API_KEY "AIzaSyC-0f7aQAJPyLOxR4jhw1QL-MGyRAfMl5o"
const char *FIREBASE_HOST="https://iothuerto-default-rtdb.firebaseio.com/"; 
const char *FIREBASE_AUTH="Yjl4M1DZmyzWdsYPowgQT7WzU9ItYE8dMSQJ6138";
FirebaseData firebaseData;

// DHT
#include "DHT.h"
#define DHTPIN D5 //forzoso NO usar D0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//PINS
const int venti = D1; //5, D3
const int bomba = D2; //4, D4
const int echo = 13;
const int trig = 3;
const int led = 12;
const int higro = A0;

bool statusBomba = false;
bool statusVenti = false;
int t_ctl = 100;      // Control temperature
// int h_ctl;      // Control humidity
int s_ctl = 0;      // Control soil moisture
int depth = 18; // Water depth
int contador=0; 
String nodo = "Proyecto-iot";
  
void setup() {
  Serial.begin(9600);
  pinMode(echo,INPUT);
  pinMode(trig,OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(higro, INPUT);
  pinMode(bomba, OUTPUT);
  pinMode(venti, OUTPUT);

  dht.begin();
  
  // Disable components
  digitalWrite(led,LOW);
  digitalWrite(venti, HIGH);
  digitalWrite(bomba, HIGH);

  //Blynk
  Blynk.begin(auth, ssid, pass);
  Serial.println(F("DHTxx test!"));
  Serial.print("Conectando...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
   
  Serial.println("Conexión OK!");
  Serial.print("IP Local: ");
  Serial.println(WiFi.localIP());

  // Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
    // Blynk gauge range
  Blynk.setProperty(V5, "max", 100);
  Blynk.setProperty(V6, "max", 100);
  Blynk.setProperty(V7, "max", 100);
  Blynk.setProperty(V8, "max", 100);

  //Temperatura
  Firebase.getInt(firebaseData, nodo + "/t_ctl");
  if (firebaseData.intData()==0){
    Firebase.setInt(firebaseData, nodo + "/t_ctl", t_ctl+1);
  }
  else{
    t_ctl=firebaseData.intData()-1;
  }

  //Humedad de suelo
  Firebase.getInt(firebaseData, nodo + "/s_ctl");
  if (firebaseData.intData()==0){
    Firebase.setInt(firebaseData, nodo + "/s_ctl", s_ctl+1);
  }
  else{
    s_ctl=firebaseData.intData()-1;
  }
  
  //Profundidad
  Firebase.getInt(firebaseData, nodo + "/depth");
  if (firebaseData.intData()==0){
    Firebase.setInt(firebaseData, nodo + "/depth", depth+1);
  }
  else{
    depth=firebaseData.intData()-1;
  }
  
  // Set relay and depth values to false by default
  Blynk.virtualWrite(V2, 0);
  Blynk.virtualWrite(V3, 0);
  Blynk.virtualWrite(V9, depth);
  Blynk.virtualWrite(V11, t_ctl);
  Blynk.virtualWrite(V12, s_ctl);
} 
// Fan
BLYNK_WRITE(V2) {
  int V2Data = param.asInt();
  Serial.print(V2Data);
  Serial.println(" Venti");
  statusVenti = (bool)V2Data;
}

// Pump
BLYNK_WRITE(V3) {
  int V3Data = param.asInt();
  Serial.print(V3Data);
  Serial.println(" Bomba");
  statusBomba = (bool)V3Data;
}

// Container depth
BLYNK_WRITE(V9) {
  int V9Data = param.asInt();
  depth = V9Data;
  // int V1Data=param.asInt();
  // digitalWrite(led, V1Data);
}

  // Temperature control value
BLYNK_WRITE(V11) {
  int V11Data = param.asInt();
  t_ctl = V11Data;
}

  // Soil moisture control value
BLYNK_WRITE(V12) {
  int V12Data = param.asInt();
  s_ctl = V12Data;
}


void loop() {
  int t;        // Temperature
  int h;        // Humidity
  long dur;       // Time difference from ultrasonic sensor
  long dist;       // Distance from ultrasonic sensor
  float s;        // Soil moisture
  int s_map;      // Soil moisture normalized
  int waterLevelPercentage;

    // Water level
  digitalWrite(trig, LOW);
  delayMicroseconds(4);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  dur = pulseIn(echo, HIGH);
  dist = ((dur * 0.034) / 2)-3;
  waterLevelPercentage = 100-(((float)dist/(depth-3))*100);
  if (waterLevelPercentage<0){
    waterLevelPercentage=0;
  }

  //waterLevelLED
  if(waterLevelPercentage<25){
    digitalWrite(led,HIGH);
  }
  else{
    digitalWrite(led,LOW);
  }
  

    // Humidity
  h = dht.readHumidity();

  // Temperature
  t = dht.readTemperature();

    // Soil moisture
  s = analogRead(higro);
  s_map = map(s, 0, 1023, 100, 0);

    // Read failure
  if (isnan(h) || isnan(t) || isnan(s) || isnan(dur)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Update fan from Blynk
  if (statusVenti) {
    digitalWrite(venti, LOW);
  } else {
    digitalWrite(venti, HIGH);
  }
  Serial.print("Status venti: ");
  Serial.println(statusVenti);

  // Update pump from Blynk}
  if (statusBomba) {
    digitalWrite(bomba, LOW);
  } else {
    digitalWrite(bomba, HIGH);
  }
  Serial.print("Status bomba: ");
  Serial.println(statusBomba);
  
   // Monitor soil moisture
  if (s_map < s_ctl) {
    digitalWrite(bomba, LOW);
    delay(5000);
    digitalWrite(bomba, HIGH);
    delay(2000);
  }

  // Monitor temperature
  if (t > t_ctl) {
    digitalWrite(venti, LOW);
    delay(5000);
    digitalWrite(venti, HIGH);
    delay(2000);
  }

  // Print stuff
  Serial.print(waterLevelPercentage);
  Serial.print("%  ");
  Serial.print(dist);
  Serial.println(" cm");
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C  Soil humidity: "));
  Serial.print(s_map);
  Serial.println("%");
  Serial.print("Control temperature: ");
  Serial.print(t_ctl);
  Serial.print(" °C  Control soil moisture: ");
  Serial.print(s_ctl);
  Serial.println("%");
//  Serial.println(statusBomba);
  

  // Write to Blynk
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
  Blynk.virtualWrite(V7, s_map);
  Blynk.virtualWrite(V8, waterLevelPercentage);

  Firebase.setInt(firebaseData, nodo + "/s_ctl", s_ctl+1);
  Firebase.setInt(firebaseData, nodo + "/t_ctl", t_ctl+1);
  Firebase.setInt(firebaseData, nodo + "/depth", depth+1);

  if(contador == 3){
    // push de datos Firebase
    Firebase.pushInt(firebaseData, nodo + "/Temperature", t);
    Firebase.pushInt(firebaseData, nodo + "/Humidity", h);
    Firebase.pushInt(firebaseData, nodo + "/Soil_Moisture", s_map);
    Firebase.pushInt(firebaseData, nodo + "/Water_Level", waterLevelPercentage);
    contador=0;
  }
  contador = contador + 1;
  
  delay(5000);
}
