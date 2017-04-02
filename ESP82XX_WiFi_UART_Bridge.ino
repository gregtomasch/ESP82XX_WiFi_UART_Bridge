/* ESP8285 UDP UART/WiFi bridge example code
 by: Greg Tomasch
 date: January 24, 2017
 license: Beerware - Use this code however you'd like. If you 
 find it useful you can buy me a beer some time.
 
 Demonstrate UDP packet transfer from the an non-WiFi MCU using the ESP8285 as  
 WiFi/UART bridge. The Serial protocol is simple (MultiWii MSP). This exapmle was
 written directly for the Pesky Products ESP8285 add-on boards
 made by Kris Winer. This board is the WiFi access point
 */

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "WiFi_UDP.h"
#include "WiFiserial.h"
#include "config.h"
extern "C"
{
  #include "user_interface.h"
  bool wifi_set_sleep_type(sleep_type_t);
  sleep_type_t wifi_get_sleep_type(void);
}

// Global variables
float    data_update[UPDATE_SIZE];
uint32_t currentTime = 0;
uint32_t previousTime = 0;
uint32_t cycleTime = 0;

void setup()
{

  WiFiSerial::SerialOpen(0, 115200);

  delay(1000);

  WiFi_UDP::WiFi_UDP_init();

  delay(1000);

  // Enable Light sleep mode
  wifi_set_sleep_type(LIGHT_SLEEP_T);
}

void loop()
{
  WiFiSerial::serialCom();
  
  // Wait until the loop cycle time has expired
  while ((micros() - previousTime) < CYCLETIME_TARGET) {}
  currentTime = micros();
  cycleTime = currentTime - previousTime;
  previousTime = currentTime;
}
