#include "Arduino.h"
#include "WiFi_UDP.h"

static byte              packet_byte[1];
static IPAddress         txip;
static IPAddress         ipClient(192,168,4,10);

// Two UDP instances needed for Tx anRx ports
WiFiUDP                  tx_udp;
WiFiUDP                  rx_udp;

WiFi_UDP::WiFi_UDP()
{
}

void WiFi_UDP::WiFi_UDP_init()
{
  WiFi_UDP::setupWiFi();
  rx_udp.begin(rxport);
}

void WiFi_UDP::Handle_Client()
{
  uint8_t noBytes;
  union
  {
    float    float_data;
    uint8_t  float_data_byte[4];
  };
  union
  {
    int32_t  long_signed;
    uint8_t  long_signed_byte[4];
  };
  union
  {
    uint32_t long_unsigned;
    uint8_t  long_unsigned_byte[4];
  };

  // Construct/send the UPD packet
  txip = ipClient;
  tx_udp.beginPacket(txip, txport);
  for(uint8_t i=0; i < UPDATE_SIZE; i++)
  {
    float_data = data_update[i];
    for(uint8_t j=0; j<4; j++)
    {
      tx_udp.write(float_data_byte[j]);
    }
  }
  tx_udp.endPacket();
}

void WiFi_UDP::setupWiFi()
{
  WiFi.mode(WIFI_AP);
  
  // Do a little work to get a unique-ish name. Append the last two bytes of the MAC (HEX'd) to "ESP8266 Inf_Upt"
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ESP8266 Inf_Upt " + macID;
  char  AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);
  for(uint8_t i=0; i<AP_NameString.length(); i++)
  {
    AP_NameChar[i] = AP_NameString.charAt(i);
  }
  WiFi.softAP(AP_NameChar, Telem_WiFiAPPSK);
}
