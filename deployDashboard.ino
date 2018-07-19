#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// WiFi Parameters
const char* ssid     = "";
const char* password = "";

HTTPClient http;  //Object of class HTTPClient
const char* devUrl = "http://<url>/job/<jobName>/lastBuild/api/json?tree=id,fullDisplayName,nextBuild,result,building,estimatedDuration";
const char* stagingUrl = "http://<url>/job/<jobName>/lastBuild/api/json?tree=id,fullDisplayName,nextBuild,result,building,estimatedDuration";
const char* prodUrl = "http://<url>/job/<jobName>/lastBuild/api/json?tree=id,fullDisplayName,nextBuild,result,building,estimatedDuration";

bool building;
bool debug = false;

byte successStatus = 83;
byte aborutStatus = 65;
byte failureStatus = 70;

#define devRedPin 5 // 1
#define devGreenPin 16 // 0
#define devBluePin 5 // 1

#define stRedPin 0 // 3
#define stGreenPin 4 // 2
#define stBluePin 0 // 3

//
#define prodRedPin 12 // 6
#define prodGreenPin 14 // 7
#define prodBluePin 12 // 6

void setup() {
  pinMode(devRedPin, OUTPUT);
  pinMode(devBluePin, OUTPUT);
  pinMode(devGreenPin, OUTPUT);

  pinMode(stRedPin, OUTPUT);
  pinMode(stBluePin, OUTPUT);
  pinMode(stGreenPin, OUTPUT);

  pinMode(prodRedPin, OUTPUT);
  pinMode(prodBluePin, OUTPUT);
  pinMode(prodGreenPin, OUTPUT);
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
}

int sendRequest(const char* project){
  http.begin(project);
  http.addHeader("Authorization", "Basic ");
  int httpCode = http.GET();
  return httpCode;
}

JsonObject& parseResponse(){
  const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& data = jsonBuffer.parseObject(http.getString());
  return data;
}


byte printResult(JsonObject& data){
  int duration = data["estimatedDuration"];
  int id = data["id"];
  building = data["building"];
  const char* result = data["result"];
  const char* displayName = data["fullDisplayName"];
  if(debug){
    Serial.print(displayName); Serial.print(" "); Serial.print(building); Serial.print(" ");  Serial.println(result);
  }
  return result[0];
}

byte reciveProjectInfo(const char* url){
    byte statusName = 0;
    int responseCode = sendRequest(url);
    if (responseCode > 0) {
      JsonObject& data = parseResponse();
      statusName = printResult(data);
    }
    return statusName;
}

void flashStatus(byte statusName, int redPin, int bluePin, int greenPin){
  if(building){
    pinOn(redPin);
    pinOn(greenPin);
  } else {
    if(statusName == successStatus){
      pinOn(greenPin);
      pinOff(redPin);
    } else if (statusName == failureStatus){
      pinOff(greenPin);
      pinOn(redPin);
    }
  }
}

void blinkStatus(byte statusName, int redPin, int bluePin, int greenPin){
  statusName == successStatus ? pinOn(greenPin) : pinOff(greenPin);
  //statusName == aborutStatus ? pinOn(bluePin) : pinOff(bluePin);
  statusName == failureStatus ? pinOn(redPin) : pinOff(redPin);
}

void pinOff(int pin){
  digitalWrite(pin, LOW);
}

void pinOn(int pin){
  digitalWrite(pin, HIGH);
}
int count = 0;
void loop() {
  // Check WiFi Status
  if (WiFi.status() == WL_CONNECTED) {
    byte devStatus = reciveProjectInfo(devUrl);
    if(devStatus > 0){
      flashStatus(devStatus, devRedPin, devBluePin, devGreenPin);
    }
    byte stagingStatus = reciveProjectInfo(stagingUrl);
    if(stagingStatus > 0){
      flashStatus(stagingStatus, stRedPin, stBluePin, stGreenPin);
    }
    byte prodStatus = reciveProjectInfo(prodUrl);
    if(prodStatus > 0){
      flashStatus(prodStatus, prodRedPin, prodBluePin, prodGreenPin);
    }
  }
  delay(5000);
}
