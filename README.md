# ESP82XX_WiFi_UART_Bridge
Use ESP82XX to broadcast UART data as UDP packets over WiFi...

The root folder contains the necessary files for the ESP8285 Arduino sketch that runs the UART WiFi Bridge. The serial protocol is based 
upon the MultiWii Serial Protocol (MSP) developed by the MultiWii open source flight controller project. Basically, when the host MCU has
data to update, it pushes it over the UART to the ESP8285 bridge. The ESP8285 then bundles the individual bytes into a UDP packet and 
transmits it. The ESP8285 UART Wifi Bridge is configured as an access point for a cell phone, tablet or another ESP82XX can connect to it.
The two sub-folders contain examples of the UART bridge from the host MCU (an STM32L4 running an AHRS sketch) to the ESP8285 and a second
ESP8285 board configured as a UDP client. The client is used to verify that the UDP packets are received; they are sent over the client 
board's USB serial port to any serial terminal program of the user's choice...
