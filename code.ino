#include <WiFi.h>
#include"DHT.h"
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
//DHT11
#define DHTPIN 25  
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float h,t;
// HC-SR501
#define inputPin 33
// MQ4 
#define MQ4_Pin 26
float valeur;

//WiFi MQTT
const char* ssid = "HUAWEI-s66A";
const char* password = "2020@2021";
const char* mqtt_server = "192.168.100.75";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];


void setup() {
  Serial.begin(9600); 
  WiFi.mode(WIFI_STA);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(LED_BUILTIN, OUTPUT);
  dht.begin();
  pinMode(inputPin, INPUT);
  pinMode(MQ4_Pin, INPUT);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if (String(topic) == "esp32/Led") {
    Serial.print("Changing output to ");
    if(messageTemp == "true"){
      Serial.println("on");
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if(messageTemp == "false"){
      Serial.println("off");
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/Led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    //DHT11
    h = dht.readHumidity();
    t = dht.readTemperature();
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.println(F("°C ")) ;
    static char temperatureTemp[7];//convert analog to str
    dtostrf(t, 6, 2, temperatureTemp);//convert analog to str
    static char humidityTemp[7];//convert analog to str
    dtostrf(h, 6, 2, humidityTemp);//convert analog to str
    client.publish("esp32/Temperature",temperatureTemp);
    client.publish("esp32/Humidity",humidityTemp);
 //HC-SR501
  int val = digitalRead(inputPin);
  if (val == HIGH) {
    client.publish("esp32/Motion","Motion detected!");
    Serial.println("Motion detected!");
   }else {
    client.publish("esp32/Motion","No Motion detected!");
    Serial.println("No Motion detected!");
    }
  //MQ4
  valeur = analogRead(MQ4_Pin);
  static char value[7];//convert analog to str
  dtostrf(valeur, 6, 2, value);//convert analog to str
  Serial.print("Methane value: ");
  Serial.println(valeur);
  client.publish("esp32/AirQuality",value);
  }
  delay(3000);
}
