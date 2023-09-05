#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <SD.h>
#include <ETH.h>
#include <WiFiClientSecure.h>
#include <Update.h>
// WiFi
const char *ssid = "Kost D8"; // Enter your WiFi name
const char *password = "606301D8";  // Enter WiFi password
StaticJsonDocument<192>doc;
WiFiClient espClient;
PubSubClient client(espClient);

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic1 = "esp32/area1/node1";
const char *topic2 = "esp32/area1/node2";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

HardwareSerial SerialPort(2); // use UART2
char buffdata[192];
char bufferdata2[192];
char bufferdata3[192];
char bufferdata4[192];
const char* klasifikasi;
void setup()
{
  Serial.begin(115200);
  SerialPort.begin(15200, SERIAL_8N1, 16, 17);
  while(!SerialPort);
  Serial.println("ready");
  WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println("Connecting to WiFi..");
 }
 Serial.println("Connected to the WiFi network");
 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public emqx mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
 }
}
void loop()
{
String mystring = SerialPort.readString();
DeserializationError error = deserializeJson (doc, mystring);
if (error){
    Serial.print(F("DeserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
}
  int nodeNumber  = doc["Node"];
  double windspeed = doc["kecepatan"];
  const char* arah = doc["arah"];
  const char* derajat = doc["derajat"];
  double temperature = doc["suhu"];
  double humidity = doc["humidity"];
  if (nodeNumber == 1){
    serializeJson(doc, bufferdata2);
  }
  if (nodeNumber == 2){
    serializeJson(doc, bufferdata3);
  }
  if (nodeNumber == 3){
    serializeJson(doc, bufferdata4);
  }
if (bufferdata2 != 0 ){
    client.publish(topic1, bufferdata2);
}
if (bufferdata3 != 0 ){
    client.publish(topic2, bufferdata3);
}
delay(2000);
}