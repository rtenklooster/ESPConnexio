/****************************************************************************************************************************\
* Arduino project "ESP-Connexio" © Copyright 2015 www.esp8266.nu
* 
* Project forked from the Nodo Project, © Copyright 2015 by Paul Tonkes www.nodo-domotica.nl
*
* This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License 
* as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*
* You received a copy of the GNU General Public License along with this program in tab '_COPYING'.
******************************************************************************************************************************/
// ESP Connexio R006, 25-04-2015
//
// This is the first alpha release for ESP Connexio. Functionality is limited!
// Compiled using Arduino IDE for ESP 1.6.1 zipfile
// Updated with github hardware folder that was downloaded on 16-04-2015
// IDE software can be downloaded from: https://github.com/esp8266/Arduino
//
// Current project status:
//   "Engine" and serial command parser is working
//   IO is tested on ESP01 with only one input and one output pin
//   WiredIn is digital, not analog!
//   Variables, Timers, WiredIn, WiredOut, Boot events are working
//   PulseCount is working with interrupts
//   Webserver is running and accepting commands, sample : http://<ip>/?cmd=WiredOut%201,On
//   Domoticz get/set virtual device value is working
//   Following plugins are working:
//     Dallas Temperature sensor
//     DHT11 Humidity sensor
//     I2C-LCD display
//     RFID Wiegand26 reader
//     MCP23017 16 bit IO expander (output only)
//     PCF8591 4 port analog to digital convertor
//     Kaku with letterwheel adressing
//     Kaku with automatic addressing
//     Flamingo Smoke Detector
//     Oregon V2 sensors

// New commands:
//   WifiConnect
//   WifiDisconnect
//   WifiSSID <SSID>
//   WifiKey <wpa key>
//   DomoticzGet <idx>,<var>
//   DomoticzSet <idx>,<var>
//   VariableMultiply <var>,<factor>
//   VariableAddVariable <var>,<var>

#define SKETCH_PATH S:\Software\ESP\source\ESPConnexio R006\ESPConnexio
//#define CONFIG_FILE Config_01.c

// ESP-01 only has 2 pins. Select according to specific needs:
//   1 = WiredIn/WiredOut
//   2 = Dallas / DHT
//   3 = I2C
//   4 = RF
//   5 = RFID

#define FUNCTION        3

// ============================= do not change anything below this line ========================================

//#define I2C                             false  // it true, GPIO 0 & 2 are used for I2C
//#define WIRED                           true  // if true, GPIO 0 is WiredIn 1 , GPIO 2 is WiredOut 1
//#define RF_IRQ                          true   // if true, GPIO 0 is interrupt driven input. Connect RF receiver

#if FUNCTION==1
  #define I2C                             false 
  #define WIRED                           true
#endif

#if FUNCTION==2
  #define I2C                             false 
  #define WIRED                           false
  #define PLUGIN_005                      // Dallas
  #define PLUGIN_006 11                   // DHT
#endif

#if FUNCTION==3
  #define I2C                             true 
  #define WIRED                           false
  //#define PLUGIN_020                    // BMP085
  #define PLUGIN_021                      4 // LCD
  #define PLUGIN_021_LABEL_04	"Alarm On"
  #define PLUGIN_021_LABEL_05	"Alarm Off"
  #define PLUGIN_021_LABEL_06	"Alarm Pending..."
  #define PLUGIN_021_LABEL_07	"Intruder"
  #define PLUGIN_021_LABEL_08	"Alert"
  #define PLUGIN_021_LABEL_09	"Temp: "
  #define PLUGIN_021_LABEL_10	"Hum: "
  #define PLUGIN_021_LABEL_11	{0xDF, 0}
  #define PLUGIN_021_LABEL_12	"%"
  #define PLUGIN_021_LABEL_13	""
  #define PLUGIN_021_LABEL_14	""
  #define PLUGIN_021_LABEL_15	""
  #define PLUGIN_021_LABEL_16	""
  #define PLUGIN_021_LABEL_17	""
  #define PLUGIN_021_LABEL_18	""
  #define PLUGIN_021_LABEL_19	""
  #define PLUGIN_021_LABEL_20	""
  #define PLUGIN_029                      // PCF8591
  #define PLUGIN_094                      // MCP23017
  #define PLUGIN_254                      // BH1750
  #define PLUGIN_254_CORE
#endif

#if FUNCTION==4
  #define I2C                             false 
  #define WIRED                           true
  #define RF_IRQ                          true
  //#define PLUGIN_001                      // Kaku (old letterwheel)
  #define PLUGIN_002                      // KaKu (automatic addressing)
  #define PLUGIN_012                      // Oregon V2
  #define PLUGIN_013                      // Flamingo FA20RF
#endif

#if FUNCTION==5
  #define I2C                             false
  #define WIRED                           false
  #define PLUGIN_030                      // RFID Wiegand-26
#endif

