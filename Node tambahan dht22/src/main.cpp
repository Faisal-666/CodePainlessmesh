//libraries
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoJson.h>
#include "painlessMesh.h"

//buat setup painlessmesh
#define   MESH_PREFIX     "Area 1"
#define   MESH_PASSWORD   "AWIKWOKAWIKWOK"
#define   MESH_PORT        5555

#define   DHTPIN           13
#define   DHTTYPE          DHT22 


//insialisasi variabel mesh
Scheduler userScheduler;
painlessMesh  mesh;

//inisialisasi dht
DHT dht(DHTPIN, DHTTYPE);
int nodeNumber = 3;
//json doc
char data[192];
StaticJsonDocument<192> doc;
void sendMessage();
void dataprocessing();
//task
Task ngirim(TASK_SECOND*5, TASK_FOREVER, &sendMessage);

//fungsi ngirim pesan
void sendMessage(){
dataprocessing();
mesh.sendBroadcast(data);
Serial.println(data);
}

//fungsi menyalakan dht
void dhttemp(){
dht.begin();
}

void dataprocessing(){
doc["node"] = nodeNumber;
dhttemp();
double tempsuhu = dht.readTemperature();
double humidity =  dht.readHumidity();
char temphum[5];
char suhu[6];
sprintf(suhu,"%.2f", tempsuhu);
doc["suhu"] = suhu;
sprintf(temphum,"%.2f", humidity);
doc["humidity"] = temphum;
serializeJson(doc, data);
}

// fungsi buat connection callback, node ajust time, recived callback & change node callback
void receivedCallback( uint32_t from, String &msg ) {
Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
}
void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}
void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

//setup
void setup() {
Serial.begin(115200);
//mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
mesh.setDebugMsgTypes( ERROR | STARTUP| MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE);
mesh.onReceive(&receivedCallback);
mesh.onNewConnection(&newConnectionCallback);
mesh.onChangedConnections(&changedConnectionCallback);
mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
mesh.setContainsRoot(true);

userScheduler.addTask(ngirim);
ngirim.enable();
}

//loop
void loop() {
mesh.update();
}