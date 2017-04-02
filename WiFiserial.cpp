#include "Arduino.h"
#include "WiFiSerial.h"

WiFiSerial::WiFiSerial()
{
}

/**
* @fn: serialCom()
*
* @brief: Main MultiWii Serial Protocol (MSP) function; manages port state and packet logistics
* @params:
* @returns:
*/
void WiFiSerial::serialCom()
{
  uint8_t c,n;  

  for(n=0; n<UART_NUMBER; n++)
  {
    CURRENTPORT = n;
    while (WiFiSerial::SerialAvailable(CURRENTPORT))
    {
      // Indicates the number of occupied bytes in TX buffer
      uint8_t bytesTXBuff = ((uint8_t)(serialHeadTX[CURRENTPORT]-serialTailTX[CURRENTPORT]))%TX_BUFFER_SIZE;

      // Ensure there is enough free TX buffer to go further (50 bytes margin)
      if (bytesTXBuff > TX_BUFFER_SIZE - 50 ) return;
      c = WiFiSerial::SerialRead(CURRENTPORT);
      if (c_state[CURRENTPORT] == IDLE)
      {
        c_state[CURRENTPORT] = (c=='$') ? HEADER_START : IDLE;
      }else if (c_state[CURRENTPORT] == HEADER_START)
      {
        c_state[CURRENTPORT] = (c=='M') ? HEADER_M : IDLE;
      } else if (c_state[CURRENTPORT] == HEADER_M)
      {
        c_state[CURRENTPORT] = (c=='<') ? HEADER_ARROW : IDLE;
      } else if (c_state[CURRENTPORT] == HEADER_ARROW)
      { 
        // Now we are expecting the payload size
        if (c > INBUF_SIZE)
        {
          c_state[CURRENTPORT] = IDLE;
          continue;
        }
        dataSize[CURRENTPORT] = c;
        Serial.println(dataSize[CURRENTPORT]);
        offset[CURRENTPORT] = 0;
        checksum[CURRENTPORT] = 0;
        indRX[CURRENTPORT] = 0;
        checksum[CURRENTPORT] ^= c;

        // The command is to follow
        c_state[CURRENTPORT] = HEADER_SIZE;
      } else if (c_state[CURRENTPORT] == HEADER_SIZE)
      {
        cmdWiFiSerial[CURRENTPORT] = c;
        checksum[CURRENTPORT] ^= c;
        c_state[CURRENTPORT] = HEADER_CMD;
      } else if (c_state[CURRENTPORT] == HEADER_CMD && offset[CURRENTPORT] < dataSize[CURRENTPORT])
      {
        checksum[CURRENTPORT] ^= c;
        inBuf[offset[CURRENTPORT]++][CURRENTPORT] = c;
      } else if (c_state[CURRENTPORT] == HEADER_CMD && offset[CURRENTPORT] >= dataSize[CURRENTPORT])
      {
        // Compare calculated and transferred checksum
        if (checksum[CURRENTPORT] == c) 
        {
          // We got a valid packet, evaluate it
          WiFiSerial::evaluateCommand();
        }
        c_state[CURRENTPORT] = IDLE;
      }
    }
  }
}

/**
* @fn: evaluateCommand()
*
* @brief: Main (MSP) command decoder
* @params:
* @returns:
*/
void WiFiSerial::evaluateCommand()
{
  switch(cmdWiFiSerial[CURRENTPORT])
  {
   case RECEIVE_SERIAL_DATA:                                                                // CMD 10
     for(uint8_t i=0; i < (UPDATE_SIZE); i++)
     {
       data_update[i] = WiFiSerial::readfloat();
     }
     delayMicroseconds(500);
     WiFi_UDP::Handle_Client();
     break;
   case SEND_SERIAL_DATA:                                                                   // CMD 20
     WiFiSerial::headSerialReply((UPDATE_SIZE));
     for(uint8_t i=0; i < (UPDATE_SIZE); i++)
     {
       WiFiSerial::serializefloat(data_update[i]);
     }
   default:
     break;
  }
}

/**
* @fn: SerialSendMWPcmd(uint8_t port, uint8_t err,uint8_t cmd)
*
* @brief: Sends a data request command to the remote host
* @params: port, command
* @returns:
*/
void WiFiSerial::SerialSendMWPcmd(uint8_t port, uint8_t err,uint8_t cmd)
{
  // Buffer previous command value for this port
  uint8_t cmd_prev = cmdWiFiSerial[port];
  
  cmdWiFiSerial[port] = cmd;
  WiFiSerial::headSerialRequest(port, err, 0);
  WiFiSerial::tailSerialRequest(port);

  // Restore previous command value for this port
  cmdWiFiSerial[port] = cmd_prev;
}

/**
* @fn: readfloat()
*
* @brief: Read 4bytes from the input buffer and converts to float
* @params:
* @returns:
*/
float WiFiSerial::readfloat()
{
  union
  {
    uint8_t float_data_byte[4];
    float float_data;
  } f;
  
  for( uint8_t i=0; i<4; i++)
  {
    f.float_data_byte[i] = WiFiSerial::read8();
  }
  return f.float_data;
}

/**
* @fn: read32()
*
* @brief: Read 4bytes from the input buffer
* @params:
* @returns:
*/
uint32_t WiFiSerial::read32()
{
  uint32_t t = WiFiSerial::read16();
  t+= (uint32_t)WiFiSerial::read16()<<16;
  return t;
}

/**
* @fn: read16()
*
* @brief: Read 2bytes from the input buffer
* @params:
* @returns:
*/
uint16_t WiFiSerial::read16()
{
  uint16_t t = WiFiSerial::read8();
  t+= (uint16_t)WiFiSerial::read8()<<8;
  return t;
}

/**
* @fn: read8()
*
* @brief: Read 1byte from the input buffer
* @params:
* @returns:
*/
uint8_t WiFiSerial::read8()
{
  return inBuf[indRX[CURRENTPORT]++][CURRENTPORT]&0xff;
}

/**
* @fn: headSerialRequest(uint8_t port, uint8_t err, uint8_t s)
*
* @brief: Construct serial request header
* @params: Port, error state and message size in bytes
* @returns:
*/
void WiFiSerial::headSerialRequest(uint8_t port, uint8_t err, uint8_t s)
{
  WiFiSerial::serialize8('$');
  WiFiSerial::serialize8('M');
  WiFiSerial::serialize8(err ? '!' : '<');
  
  // start calculating a new checksum
  checksum[port] = 0;
  WiFiSerial::serialize8(s);
  WiFiSerial::serialize8(cmdWiFiSerial[port]);
}

/**
* @fn: headSerialResponse(uint8_t err, uint8_t s)
*
* @brief: Construct serial reply header
* @params: Error state and reply size in bytes
* @returns:
*/
void WiFiSerial::headSerialResponse(uint8_t err, uint8_t s)
{
  WiFiSerial::serialize8('$');
  WiFiSerial::serialize8('M');
  WiFiSerial::serialize8(err ? '!' : '>');
  
  // start calculating a new checksum
  checksum[CURRENTPORT] = 0;
  WiFiSerial::serialize8(s);
  WiFiSerial::serialize8(cmdWiFiSerial[CURRENTPORT]);
}

/**
* @fn: headSerialReply(uint8_t s)
*
* @brief: Construct serial reply header with error state "0"
* @params: Reply size in bytes
* @returns:
*/
void WiFiSerial::headSerialReply(uint8_t s)
{
  WiFiSerial::headSerialResponse(0, s);
}

/**
* @fn: headSerialError(uint8_t s)
*
* @brief: Construct serial reply header with error state "1"
* @params: Reply size in bytes
* @returns:
*/
void inline WiFiSerial::headSerialError(uint8_t s)
{
  WiFiSerial::headSerialResponse(1, s);
}

/**
* @fn: tailSerialRequest()
*
* @brief: Construct and transmit checksum data for end of packet
* @params:
* @returns:
*/
void WiFiSerial::tailSerialRequest(uint8_t port)
{
  WiFiSerial::serialize8(checksum[port]);
  WiFiSerial::UartSendData(port);
}

/**
* @fn: tailSerialReply()
*
* @brief: Construct and transmit checksum data for end of packet
* @params:
* @returns:
*/
void WiFiSerial::tailSerialReply()
{
  WiFiSerial::serialize8(checksum[CURRENTPORT]);
  WiFiSerial::UartSendData(CURRENTPORT);
}

/**
* @fn: serializeNames(PGM_P s)
*
* @brief: "Serialize" ASCII into bytes for serial transmission
* @params:
* @returns:
*/
void WiFiSerial::serializeNames(PGM_P s)
{
  for (PGM_P c = s; pgm_read_byte(c); c++)
  {
    WiFiSerial::serialize8(pgm_read_byte(c));
  }
}

/**
* @fn: serializefloat(float a)
*
* @brief: "Serialize" a float into four bytes for serial transmission
* @params:
* @returns:
*/
void WiFiSerial::serializefloat(float a)
{
  union
  {
    float float_data;
    uint8_t float_data_byte[4];
  } f;
  
  for( uint8_t i=0; i<4; i++)
  {
    WiFiSerial::serialize8(f.float_data_byte[i]);
  }
}

/**
* @fn: serialize32(uint32_t a)
*
* @brief: "Serialize" a 32bit unsigned integer into four bytes for serial transmission
* @params:
* @returns:
*/
void WiFiSerial::serialize32(uint32_t a)
{
  WiFiSerial::serialize8((a    ) & 0xFF);
  WiFiSerial::serialize8((a>> 8) & 0xFF);
  WiFiSerial::serialize8((a>>16) & 0xFF);
  WiFiSerial::serialize8((a>>24) & 0xFF);
}

/**
* @fn: serialize16(int16_t a)
*
* @brief: "Serialize" a 16bit signed integer into four bytes for serial transmission
* @params:
* @returns:
*/
void WiFiSerial::serialize16(int16_t a)
{
  WiFiSerial::serialize8((a   ) & 0xFF);
  WiFiSerial::serialize8((a>>8) & 0xFF);
}

/**
* @fn: serialize16(uint8_t a)
*
* @brief: "Serialize" an 8bit unsigned integer into four bytes for serial transmission
* @params:
* @returns:
*/
void WiFiSerial::serialize8(uint8_t a)
{
  uint8_t t = serialHeadTX[CURRENTPORT];
  if (++t >= TX_BUFFER_SIZE) t = 0;
  serialBufferTX[t][CURRENTPORT] = a;
  checksum[CURRENTPORT] ^= a;
  serialHeadTX[CURRENTPORT] = t;
}

/**
* @fn: T_USB_Available(uint8_t port)
*
* @brief: Returns the number of bytes available to be read from port
* @params:
* @returns:
*/
unsigned char WiFiSerial::T_USB_Available(uint8_t port)
{
  int n ; 
  switch (port)
  {
    case 0: n= Serial.available(); break;
    case 1: n= Serial1.available(); break;
    default: n=0; break;
  }
  if (n > 255) n = 255;
  return n;
}

/**
* @fn: T_USB_Write(uint8_t port, uint8_t uc_data)
*
* @brief: Writes specified data to specified port
* @params: Data to be written, port to be written to
* @returns: Number of btes written
*/
unsigned char WiFiSerial::T_USB_Write(uint8_t port, uint8_t uc_data)
{
  int n ; 
  switch (port)
  {
    case 0: n= Serial.write(uc_data); break;
    case 1: n= Serial1.write(uc_data); break;
    default: n=0; break;
  }
  return n;
}

/**
* @fn: UartSendData(uint8_t port)
*
* @brief: Writes TX buffer to serial port
* @params: Port to be written to
* @returns:
*/
void WiFiSerial::UartSendData(uint8_t port)
{
  while(serialHeadTX[port] != serialTailTX[port])
  {
    if (++serialTailTX[port] >= TX_BUFFER_SIZE) serialTailTX[port] = 0;
    WiFiSerial::T_USB_Write (port,serialBufferTX[serialTailTX[port]][port]);
  }
}

/**
* @fn: SerialTXfree(uint8_t port)
*
* @brief: Determines if serial port is free (TX buffer empty)
* @params: Port to be written to
* @returns: Boolean regarding port availability
*/
bool WiFiSerial::SerialTXfree(uint8_t port)
{
  return (serialHeadTX[port] == serialTailTX[port]);
}

/**
* @fn: SerialOpen(uint8_t port, uint32_t baud)
*
* @brief: Opens serial port for use
* @params: Port to be initialized, baud rate
* @returns:
*/
void WiFiSerial::SerialOpen(uint8_t port, uint32_t baud)
{
  switch (port)
  {
    case 0: Serial.begin(baud);  break;
    case 1: Serial1.begin(baud); break;
  }
}

/**
* @fn: SerialEnd(uint8_t port)
*
* @brief: Closes serial port
* @params: Port to be closed
* @returns:
*/
void WiFiSerial::SerialEnd(uint8_t port)
{
  switch (port)
  {
      case 0: Serial.end(); break;
      case 1: Serial1.end(); break;
  }
}

/**
* @fn: SerialRead(uint8_t port)
*
* @brief: Reads a byte from a serialport
* @params: Port to be read
* @returns:
*/
uint8_t WiFiSerial::SerialRead(uint8_t port)
{
  switch (port)
  {
  case 0: return Serial.read();
  case 1: return Serial1.read();
  }
  return 0;
}

uint8_t WiFiSerial::SerialAvailable(uint8_t port)
{
  return WiFiSerial::T_USB_Available(port);
}

void WiFiSerial::SerialSerialize(uint8_t port,uint8_t a)
{
  uint8_t t = serialHeadTX[port];
  if (++t >= TX_BUFFER_SIZE) t = 0;
  serialBufferTX[t][port] = a;
  serialHeadTX[port] = t;
}

void WiFiSerial::SerialWrite(uint8_t port,uint8_t c)
{
  CURRENTPORT=port;
  SerialSerialize(port,c);WiFiSerial::UartSendData(port);
}
