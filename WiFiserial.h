#ifndef WiFiSerial_h
#define WiFiSerial_h

#include "config.h"
#include "WiFi_UDP.h"

#define UART_NUMBER                    1
#define TX_BUFFER_SIZE                 512
#define INBUF_SIZE                     512
#define RECEIVE_SERIAL_DATA            10
#define SEND_SERIAL_DATA               20


static enum _WiFiSerial_state
                                      {
                                        IDLE,
                                        HEADER_START,
                                        HEADER_M,
                                        HEADER_ARROW,
                                        HEADER_SIZE,
                                        HEADER_CMD,
                                      } c_state[UART_NUMBER];
static volatile uint8_t                 serialHeadTX[UART_NUMBER],serialTailTX[UART_NUMBER];
static uint8_t                          serialBufferTX[TX_BUFFER_SIZE][UART_NUMBER];
static uint8_t                          inBuf[INBUF_SIZE][UART_NUMBER];
static uint8_t                          checksum[UART_NUMBER];
static uint8_t                          indRX[UART_NUMBER];
static uint8_t                          cmdWiFiSerial[UART_NUMBER];
static uint8_t                          CURRENTPORT = 0;
static uint8_t                          offset[UART_NUMBER];
static uint8_t                          dataSize[UART_NUMBER];
static uint8_t                          blah_blah = 0;

//extern _WiFiSerial_state               c_state[UART_NUMBER];
extern float                            data_update[UPDATE_SIZE];
extern float                            vbat;

class WiFiSerial
{
  public:
                                       WiFiSerial();
	   static void                       serialCom();
	   static bool                       SerialTXfree(uint8_t port);
	   static void                       SerialOpen(uint8_t port, uint32_t baud);
	   static void                       SerialEnd(uint8_t port);
	   static uint8_t                    SerialRead(uint8_t port);
	   static uint8_t                    SerialAvailable(uint8_t port);
	   static void                       SerialWrite(uint8_t port,uint8_t c);
     static void                       SerialSendMWPcmd(uint8_t port, uint8_t err,uint8_t cmd);
  private:
     static float                      readfloat();
     static uint32_t                   read32();
     static uint16_t                   read16();
     static uint8_t                    read8();
     static void                       headSerialRequest(uint8_t port, uint8_t err, uint8_t s);
     static void                       headSerialResponse(uint8_t err, uint8_t s);
     static void                       headSerialReply(uint8_t s);
     static void inline                headSerialError(uint8_t s);
     static void                       evaluateCommand();
     static void                       serializefloat(float a);
	   static void                       serialize32(uint32_t a);
	   static void                       serialize16(int16_t a);
	   static void                       serialize8(uint8_t a);
     static void                       serializeNames(PGM_P s);
	   static unsigned char              T_USB_Available(uint8_t port);
	   static unsigned char              T_USB_Write(uint8_t port, uint8_t uc_data);
	   static void                       UartSendData(uint8_t port);
	   static void                       SerialSerialize(uint8_t port,uint8_t a);
     static void                       tailSerialRequest(uint8_t port);
     static void                       tailSerialReply();
};

#endif // WiFiSerial_h
