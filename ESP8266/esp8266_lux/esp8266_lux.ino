#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>

#include <ESP8266WiFi.h>

#include "DHT.h"
#define DHTPIN D5     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
#define ONE_WIRE_BUS D6

const char* ssid     = "Elec_LAB";
const char* password = "2016eeb346";
const char* host = "maker.ifttt.com";

int BH1750_address = 0x23; // i2c Addresse
byte buff[2];

OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Wire.begin(4, 5);
  BH1750_Init(BH1750_address);
  dht.begin();
  sensors.begin();
  
  delay(200); 
}

void loop() {

  delay(30000);
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int soil_moisture = analogRead(A0);
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  float valf = 0;

  if (BH1750_Read(BH1750_address) == 2) {

    valf = ((buff[0] << 8) | buff[1]) / 1.2;

    //if (valf < 0)Serial.print("> 65535");
    //else Serial.print((int)valf, DEC);
    //Serial.println(" lx");
  }

  sensors.requestTemperatures(); // Send the command to get temperatures
  float temp_ds = sensors.getTempCByIndex(0);
//  Serial.print(sensors.getTempCByIndex(0));
//  Serial.println(" Degrees C");

  // We now create a URI for the request
  String url = "/trigger/node_01/with/key/dfqbGhQc0KNvacQn5DADp_";
  url += "?value1=";
  url += (String)(valf);
  url += "&value2=";
  url += (String)(temp_ds);
  url += ",";
  url += (String)(h);
  url += ",";
  url += (String)(t);
  url += "&value3=";
  url += (String)(soil_moisture);
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");



}

void BH1750_Init(int address) {

  Wire.beginTransmission(address);
  Wire.write(0x10); // 1 [lux] aufloesung
  Wire.endTransmission();
}

byte BH1750_Read(int address) {

  byte i = 0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while (Wire.available()) {
    buff[i] = Wire.read();
    i++;
  }
  Wire.endTransmission();
  return i;
}

