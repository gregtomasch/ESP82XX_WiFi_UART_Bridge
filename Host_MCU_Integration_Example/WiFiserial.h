#ifndef WiFiSerial_h
#define WiFiSerial_h

// Configuration definitions
#define UART_NUMBER                    4                                                              // Total number of possible serial ports on the STM32
#define TX_BUFFER_SIZE                 512                                                            // 512 is a good size for general use, even GNSS devices
#define INBUF_SIZE                     512                                                            // 512 is a good size for general use, even GNSS devices
#define RECEIVE_SERIAL_DATA            10                                                             // Command 10, receive serial data from the ESP82XX
#define SEND_SERIAL_DATA               20                                                             // Command 20, send serial data to the ESP82XX
#define UPDATE_SIZE                    20                                                             // Size (in float data points) of the data update message pushed to the ESP82XX
#define BUTTERFLY_WIFI_UART            3                                                              // Butterfly UART port connected to the ESP82XX to constitute the UART/WiFi bridge


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

extern float                            data_update[UPDATE_SIZE];                                     // 1D array for data points to be sent over the UART bridge. DATA TREATED AS SP FLOATS

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
     static void                       Push_Data_Update();
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
