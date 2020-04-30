#include <ESP8266WiFi.h>

WiFiClient client;
bool inProgC = false;
bool inProgS = false;
bool newData = false;
bool newData_S = false;
bool l_info = false;
char recvData[32];
char srecvData[32];
char* address = "addr-aleatorio";

void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin("Formigao 2.4Ghz", "VhgpMhgp!!07@)05");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void loop() {
  if (!client.connected()){
    connectHost();
  }
  if (digitalRead(2) == HIGH) {
    recvCmd();
  }
  if (digitalRead(2) == LOW) {
    readLInfo();
  }
}

void connectHost() {
  if (!client.connect("192.168.0.8", 1107)) {
    delay(5000);
    return;
  }
}

void recvCmd() {
  byte i;
  while(client.available() > 0){
    char c = client.read();
    if (c == '{'){
      i = 0;
      recvData[0] = '\0';
      inProgC = true;
    } else if (c == '}') {
      inProgC = false;
      newData = true;
      break;
    } else if (inProgC) {
      recvData[i] = c;
      i++;
      recvData[i] = '\0';
    }
  }
  if (newData) {
    Serial.print('<');
    Serial.print(recvData);
    Serial.print('>');
    if (strcmp(recvData, "address") == 0) {
      client.print(address);
    } else if (strcmp(recvData, "lightInfo") == 0) {
      readLInfo();
    }
    newData = false;
  }
}

void readLInfo() {
  byte i = 0;
  char c;
  while(Serial.available() > 0 && !newData_S){
    c = Serial.read();
    Serial.print('<');
    Serial.print(c);
    Serial.println('>');
    if (c != '\n'){
      srecvData[i] = c;
      i++;
    } else {
      srecvData[i] = '\0';
      i = 0;
      newData_S = true;
    }
  }
  if (newData_S) {
    client.print(srecvData);
    newData_S = false;
  }
}
