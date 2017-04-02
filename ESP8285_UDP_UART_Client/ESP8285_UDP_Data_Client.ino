/* Basic ESP8285 UDP client Example Code

 by: Greg Tomasch
 date: January 24, 2017
 license: Beerware - Use this code however you'd like. If you 
 find it useful you can buy me a beer some time.
 
 This code lets the user program an ESP82XX board to act as a receiver
 client for the ESP82XX UART/WiFi bridge. It configures as a client, connects
 to the bridges acces point, receives the data the bridge transmits and
 pushes it to the serial port for the user to see twhat the bridge is sending
 from the host MCU. This was written for the Pesky Products ESP8285 development board
 made by Kris Winer
 */


#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

#define SERIAL_DEBUG
#define CYCLETIME_TARGET              1000
#define UPDATE_SIZE                   20
#define WIFI_LED_PIN                  15
#define WIFI_LINK_PINMODE             pinMode(WIFI_LED_PIN, OUTPUT);
#define WIFI_LINK_LED_ON              digitalWrite(WIFI_LED_PIN, HIGH);
#define WIFI_LINK_LED_OFF             digitalWrite(WIFI_LED_PIN, LOW);
#define WIFI_LED_TOGGLE               if(digitalRead(WIFI_LED_PIN)) digitalWrite(WIFI_LED_PIN, LOW); else digitalWrite(WIFI_LED_PIN, HIGH);


// Global Variables
static String            ssid_string;
static char              ssid[32];
static char              ssidtest[17];
const char*              password = "startnow";
static uint8_t           IUboard_found = 0;
static uint8_t           connect_success = 0;
const uint16_t           txport   = 9047;
const uint16_t           rxport   = 9048;
uint32_t                 currentTime  = 0;
uint32_t                 previousTime = 0;
uint16_t                 noBytes;
byte                     packetBuffer[4*UPDATE_SIZE];
uint32_t                 Timestamp;
int32_t                  debug[4];
uint8_t                  unity = 1;
IPAddress                ipServer(192,168,4,1);
IPAddress                ipClient(192,168,4,10);
IPAddress                Subnet(255,255,255,0);

// Two UDP instances needed for Tx and Rx ports
WiFiUDP                  tx_udp;
WiFiUDP                  rx_udp;

// Function declarations
void setupWiFi();

void setup()
{
  Serial.begin(115200);
  
  delay(5000);
  
  WIFI_LINK_PINMODE;
  WIFI_LINK_LED_OFF;
  EEPROM.begin(1024);
  
  delay(10);
  
  setupWiFi();

  // 9048 for the RX UDP port
  rx_udp.begin(rxport);
  #ifdef SERIAL_DEBUG
    Serial.print("UDP Rx Port: ");Serial.println(rx_udp.localPort());
    Serial.println();
  #endif
}

void loop()
{
  union
  {
    float float_data;
    uint8_t float_data_byte[4];
  };
  union
  {
    int32_t long_signed;
    uint8_t long_signed_byte[4];
  };
  union
  {
    uint32_t long_unsigned;
    uint8_t long_unsigned_byte[4];
  };
  
  // Check the local Rx port to see if there is an update packet of the correct size
  noBytes = rx_udp.parsePacket();
  if (noBytes == 4*UPDATE_SIZE)
  {
    // Read the packet, decode and print
    rx_udp.read(packetBuffer, noBytes);
    for(uint8_t i=0; i<UPDATE_SIZE; i++)
    {
      for(uint8_t j=0; j<4; j++)
      {
        float_data_byte[j] = packetBuffer[j + (4*i)];
      }
      if(i == 0)
      {
        Serial.print(float_data, 3);
      } else
      {
        Serial.print(float_data, 2);
      }
      if(i < (UPDATE_SIZE - 1))
      {
        Serial.print(",");
      }
      delayMicroseconds(10);
    }
    Serial.println("");
    
    // LED goes on after succesful data transfer
    WIFI_LED_TOGGLE;
  }
    
  // Wait until the loop cycle time has expired
  while ((micros() - previousTime) < CYCLETIME_TARGET) {}
  currentTime = micros();
  previousTime = currentTime;
}

/**
* @fn: setupWiFi()
*
* @brief: Identifies the semi-uniuqe WAP name and configures the ESP8266 as a WiFi client
* 
* @params:
* @returns:
*/
void setupWiFi()
{
  WiFi.mode(WIFI_STA);
  String wap_check;

  // Check the EEPROM for the last IU board connected
  for (int i = 0; i < 15; ++i)
  {
    wap_check += char(EEPROM.read(i));
  }

  // If the first 16bytes of the EEPROM WAP space is "ESP8266 Inf_Upt", use the last WAP was an allowed board
  // Read the full WAP SSID, try connecting to that board first
  if(wap_check == "ESP8266 Inf_Upt")
  {
    for (int i = 0; i < 32; ++i)
    {
      ssid[i] = char(EEPROM.read(i));
    }
    IUboard_found = 1;
  }

  // Now Connect to the IU board
  while(connect_success == 0)
  {
    // If the Harness SSID is unknown or connect times out, scan available networks
    while(IUboard_found == 0)
    {
      WIFI_LINK_LED_ON;
      delay(100);
      #ifdef SERIAL_DEBUG
        Serial.println("scan start");
      #endif
      // WiFi.scanNetworks will return the number of networks found
      int n = WiFi.scanNetworks();
      WIFI_LINK_LED_OFF;
      #ifdef SERIAL_DEBUG
        Serial.println("scan done");
      #endif
      if (n == 0)
      {
        #ifdef SERIAL_DEBUG
          Serial.println("no networks found");
        #endif
      } else
      {
        #ifdef SERIAL_DEBUG
          Serial.print(n);
          Serial.println(" networks found");
        #endif
        for (int i = 0; i < n; ++i)
        {
          // Print SSID and RSSI for each network found
          #ifdef SERIAL_DEBUG
            Serial.print(i + 1);
            Serial.print(": ");
          #endif
          ssid_string = String(WiFi.SSID(i));
          ssid_string.toCharArray(ssidtest,16);
          if(String(ssidtest) == "ESP8266 Inf_Upt")
          {
            ssid_string.toCharArray(ssid,32);
            IUboard_found = 1;
          }
          #ifdef SERIAL_DEBUG
            Serial.print(ssid_string);
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
          #endif
          delay(10);
        }
      }
    }
    #ifdef SERIAL_DEBUG
      Serial.println();
      Serial.println();
      Serial.print("Connecting to ");
      Serial.println(ssid);
    #endif

    // Try connecting to the allowed SSID
    WiFi.begin(ssid, password);
    WiFi.config(ipClient, ipServer, Subnet);
    uint8_t j = 0;
    while(1)
    {
      if(WiFi.status() == WL_CONNECTED)
      {
        // If connects, break out of the loop successful
        connect_success = 1;
        break;
      }
      if(j > 19)
      {
        // If times out break out of loop unsuccessful and go scan available networks
        #ifdef SERIAL_DEBUG
          Serial.println();
        #endif
        connect_success = 0;
        IUboard_found = 0;
        break;
      }
      WIFI_LED_TOGGLE;
      delay(500);
      #ifdef SERIAL_DEBUG
        Serial.print(".");
      #endif
      j++;
    }
  }
  WIFI_LINK_LED_OFF;
  #ifdef SERIAL_DEBUG
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  #endif

  // If you're here, you've successfully connected; save the SSID to EEPROM
  for (int i = 0; i < 32; ++i)
  {
    EEPROM.write(i, ssid[i]);
  }
  EEPROM.commit();
}

