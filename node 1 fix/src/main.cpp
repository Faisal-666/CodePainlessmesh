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

//inisialisasi variabel
const float phi = 3.14159265;
const float k = 3.15;
const float Rrc = 0.07;
volatile unsigned int count = 0;
double speedwind;
float rps;
float RPM;
int nodeNumber = 1;
const char*arah;
String derajat;
int period = 3000;

//filter simple moving average
const int banyakdata = 3;
int dataArray[banyakdata], total;
float avg;
//windvane pin
int hallpin1 = 36;
int hallpin2 = 32;
int hallpin3 = 39;
int hallpin4 = 33;
int hallpin5 = 34;
int hallpin6 = 25;
int hallpin7 = 35;
int hallpin8 = 26;
//state windvane
int state1 = 0;
int state2 = 0;
int state3 = 0;
int state4 = 0;
int state5 = 0;
int state6 = 0;
int state7 = 0;
int state8 = 0;
float t;
//json doc
char data[256];
StaticJsonDocument<192> doc;
void sendMessage();
void turu();
//task
Task ngirim(TASK_SECOND*5, TASK_FOREVER, &sendMessage);

//fungsi menyalakan dht
void dhttemp(){
dht.begin();
}

//fungsi buat intterupt
void magnet_detect()
{
count++;
}

void measuring(){
count = 0;
attachInterrupt(digitalPinToInterrupt(12), magnet_detect, RISING);
unsigned long millis();
long startTime = millis();
while(millis() < startTime + period) {}
detachInterrupt(12);
}

void calc(){
measuring();
RPM = (float)count*60/(period/1000);
rps = (float)RPM/60;
speedwind = k*(2*phi)*Rrc*rps ;
for(int index=0; index<banyakdata-1; index++){
	dataArray[index] = dataArray[index+1];
}
dataArray[banyakdata-1]=speedwind;
total=0;
for(int index=0; index<banyakdata; index++){
	total+=dataArray[index];
}
avg=total/banyakdata;
}

//fungsi untuk windvane
void windvane(){
state1 = digitalRead(hallpin1);
state2 = digitalRead(hallpin2);
state3 = digitalRead(hallpin3);
state4 = digitalRead(hallpin4);
state5 = digitalRead(hallpin5);
state6 = digitalRead(hallpin6);
state7 = digitalRead(hallpin7);
state8 = digitalRead(hallpin8);
if(state1 == LOW){
  arah = "utara";
  derajat = "360";
  return;
}
if(state2 == LOW){
  arah = "timur";
  derajat = "90";
  return;
}
if(state3 == LOW){
  arah = "selatan";
  derajat = "180";
  return;
}
if(state4 == LOW){
  arah = "barat";
  derajat = "270";
  return;
}
if(state5 == LOW){
  arah = "timur laut";
  derajat = "45";
  return;
}
if(state6 == LOW){
  arah = "tenggara";
  derajat = "135";
  return;
}
if(state7 == LOW){
  arah = "barat daya";
  derajat = "225";
  return;
}
if(state8 == LOW){
  arah = "barat laut";
  derajat = "315";
  return;
}
else{
  arah ="";
  derajat="";
}}


//fungsi membuat json dari semua data
void dataprocessing(){
doc["Node"] = nodeNumber;
calc();
char swtemp[5];
sprintf(swtemp,"%.2f",speedwind);
//char smatemp[5];
//sprintf(smatemp,"%.2f",avg);
doc["kecepatan"] = swtemp;
//doc["moving average"] = smatemp;
windvane();
doc["arah"] = arah;
doc["derajat"] = derajat;
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

void sendMessage(){
dataprocessing();
mesh.sendBroadcast(data);
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
pinMode(12, INPUT_PULLUP);
pinMode(hallpin1, INPUT_PULLUP);
pinMode(hallpin2, INPUT_PULLUP);
pinMode(hallpin3, INPUT_PULLUP);
pinMode(hallpin4, INPUT_PULLUP);
pinMode(hallpin5, INPUT_PULLUP);
pinMode(hallpin6, INPUT_PULLUP);
pinMode(hallpin7, INPUT_PULLUP);
pinMode(hallpin8, INPUT_PULLUP);

mesh.setDebugMsgTypes( ERROR | STARTUP| MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE);  // set before init() so that you can see startup messages
//mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION);
mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
mesh.onReceive(&receivedCallback);
mesh.onNewConnection(&newConnectionCallback);
mesh.onChangedConnections(&changedConnectionCallback);
mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

userScheduler.addTask(ngirim);
ngirim.enable();
}


//loop
void loop() {
mesh.update();
}