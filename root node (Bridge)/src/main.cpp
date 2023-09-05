#include "painlessMesh.h"
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>

#define   MESH_PREFIX     "Area 1"
#define   MESH_PASSWORD   "AWIKWOKAWIKWOK"
#define   MESH_PORT        5555

String datarcv;
unsigned long int currenttime;
StaticJsonDocument<192>doc;
void sendmsg1();
void sendmsg2();
Task send1(TASK_SECOND*3, TASK_FOREVER, &sendmsg1);
Task send2(TASK_SECOND*3, TASK_FOREVER, &sendmsg2);

char bufferdata[256];
char bufferdata2[256];
char bufferdata3[256];
HardwareSerial SerialPort(2); // use UART2
// prototypes
void receivedCallback( uint32_t from, String &msg );

painlessMesh  mesh;
Scheduler userScheduler;


void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  currenttime = millis();
  datarcv = msg.c_str();
  DeserializationError error = deserializeJson(doc, datarcv);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
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
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
  unsigned long int networkTime = millis();
  unsigned long int networkTimetaken = networkTime - currenttime;
  Serial.printf("Start : new connection , from nodeId=%u, Time taken for sycn networks:%u millisecond\n", nodeId, networkTimetaken);
  Serial.printf("Start : new connection, %s\n", mesh.subConnectionJson(true).c_str());
}


void setup() {
  Serial.begin(115200);
  SerialPort.begin(15200, SERIAL_8N1, 16, 17); 
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages
  //mesh.setDebugMsgTypes( ERROR | STARTUP| MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE);
  mesh.init( MESH_PREFIX, MESH_PASSWORD,&userScheduler, MESH_PORT);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.onNewConnection(&newConnectionCallback);
  mesh.onReceive(&receivedCallback);
  userScheduler.addTask(send1);
  userScheduler.addTask(send2);
  send1.enable();
  send2.enable();
}

void loop() {
mesh.update();
}

void sendmsg1(){
SerialPort.print(bufferdata2);
}

void sendmsg2(){
SerialPort.print(bufferdata3);
}