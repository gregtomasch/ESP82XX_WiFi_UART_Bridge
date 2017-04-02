#ifndef WiFi_UDP_h
#define WiFi_UDP_h

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "WiFiserial.h"
#include "config.h"

extern float                             data_update[UPDATE_SIZE];
extern uint32_t                          currentTime;

const char                               Telem_WiFiAPPSK[] = "startnow";
const uint16_t                           txport = 9048;
const uint16_t                           rxport = 9047;

class WiFi_UDP
{
  public:
                                         WiFi_UDP();
    static void                          WiFi_UDP_init();
    static void                          Handle_Client();
  private:
    static void                          setupWiFi();
};

#endif // WiFi_UDP_h
