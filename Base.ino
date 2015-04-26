#define ESP_BUILD                        006                                    // ??? Ophogen bij iedere Build / versiebeheer.
#define ESP_VERSION_MINOR                 10                                    // Ophogen bij gewijzigde settings struct of nummering events/commando's. 
#define ESP_VERSION_MAJOR                  3                                    // Ophogen bij DataBlock en NodoEventStruct wijzigingen.
#define UNIT_ESP                           1                                    // Unit nummer van deze Nodo
#define HOME_ESP                           1                                    // Home adres van Nodo's die tot Ã©Ã©n groep behoren (1..7). Heeft je buurman ook een Nodo, kies hier dan een ander Home adres
#define MIN_RAW_PULSES                    32                                    // =8 bits. Minimaal aantal ontvangen bits*2 alvorens cpu tijd wordt besteed aan decodering, etc. Zet zo hoog mogelijk om CPU-tijd te sparen en minder 'onzin' te ontvangen.
#define RAWSIGNAL_TX_REPEATS               8                                    // Aantal keer dat een frame met pulsen herhaald wordt verzonden (RawSignalSend)
#define RAWSIGNAL_TX_DELAY                20                                    // Tijd in mSec. tussen herhalingen frames bij zenden. (RawSignalSend)
#define RAWSIGNAL_TOLERANCE              100                                    // Tolerantie die gehanteerd wordt bij decoderen van RF/IR signaal. T.b.v. uitrekenen HEX-code.
#define RAWSIGNAL_SAMPLE_DEFAULT          25                                    // Sample grootte / Resolutie in uSec waarmee ontvangen Rawsignalen pulsen worden opgeslagen
#define WAIT_FREE_RX                   false                                    // true: wacht default op verzenden van een event tot de IR/RF lijn onbezet is. Wordt overruled door commando [WaitFreeRX]
#define WAIT_FREE_RX_WINDOW             1000                                    // minimale wachttijd wanneer wordt gewacht op een vrije RF of IR band.
#define WAIT_FREE_RX_TIMEOUT            5000                                    // tijd in ms. waarna het wachten wordt afgebroken als er geen ruimte in de vrije ether komt
#define MIN_PULSE_LENGTH                 100                                    // Pulsen korter dan deze tijd uSec. worden als stoorpulsen beschouwd.
#define SIGNAL_TIMEOUT                     5                                    // na deze tijd in mSec. wordt een signaal als beeindigd beschouwd.
#define SIGNAL_REPEAT_TIME               500                                    // Tijd in mSec. waarbinnen hetzelfde event niet nogmaals via RF/IR mag binnenkomen. Onderdrukt ongewenste herhalingen van signaal
#define PULSE_DEBOUNCE_TIME               10                                    // pulsen kleiner dan deze waarde in milliseconden worden niet geteld. Bedoeld om verstoringen a.g.v. ruis of dender te voorkomen
#define PULSE_TRANSITION             FALLING                                    // FALLING of RISING: Geeft aan op welke flank de PulseCounter start start met tellen. Default FALLING
#define I2C_START_ADDRESS                  1                                    // Alle Nodo's op de I2C bus hebben een uniek adres dat start vanaf dit nummer. Er zijn max. 32 Nodo's. Let op overlap met andere devices. RTC zit op adres 104.
#define BAUD                           19200                                    // Baudrate voor seriele communicatie.
#define PASSWORD_MAX_RETRY                 5                                    // aantal keren dat een gebruiker een foutief wachtwoord mag ingeven alvorens tijdslot in werking treedt
#define PASSWORD_TIMEOUT                 300                                    // aantal seconden dat het terminal venster is geblokkeerd na foutive wachtwoord
#define TERMINAL_TIMEOUT                 600                                    // Aantal seconden dat, na de laatst ontvangen regel, de terminalverbinding open mag staan.
#define DELAY_BETWEEN_TRANSMISSIONS       50                                    // Minimale tijd tussen verzenden van twee events. Geeft ontvangende apparaten (en Nodo's) verwerkingstijd.
#define ESP_TX_TO_RX_SWITCH_TIME         500                                    // Tijd die andere Nodo's nodig hebben om na zenden weer gereed voor ontvangst te staan. (Opstarttijd 433RX modules)
#define TRANSMITTER_STABLE_TIME            5                                    // Tijd die de RF zender nodig heeft om na inschakelen van de voedspanning een stabiele draaggolf te hebben.

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Wire.h>

ESP8266WebServer server(80);

// t.b.v. includen Config_xx.c files
#define stringify(x) #x
#define CONFIGFILE2(a, b) stringify(a/Config/b)
#define CONFIGFILE(a, b) CONFIGFILE2(a, b)
//#include CONFIGFILE(SKETCH_PATH,CONFIG_FILE)

// Onderstaande commando codes mogen worden gebruikt door andere devices dan een Nodo.
// Uit compatibility overwegingen zullen deze commando codes niet worden aangepast bij 
// vervolgreleases. 

#define CMD_DUMMY                       0
#define EVENT_BOOT                      1
#define CMD_SOUND                       2
#define EVENT_USEREVENT                 3
#define EVENT_VARIABLE                  4
#define CMD_VARIABLE_SET                5
#define RESERVED_06                     6
#define RESERVED_07                     7
#define RESERVED_08                     8
#define RESERVED_09                     9
#define RESERVED_10                     10
#define RESERVED_11                     11
#define RESERVED_12                     12
#define RESERVED_13                     13
#define RESERVED_14                     14
#define RESERVED_15                     15
#define COMMAND_MAX_FIXED               15

// Onderstaande commando codes kunnen bij vervolgreleases worden aangepast. Deze codes dus niet buiten de Nodo
// code gebruiken.

#define EVENT_ALARM                     16
#define CMD_ALARM_SET                   17
#define VALUE_ALL                       18                                      // VALUE_ALL moet groter zijn dan USER_VARIABLES_MAX !
#define CMD_ANALYSE_SETTINGS            19
#define CMD_BREAK_ON_DAYLIGHT           20
#define CMD_BREAK_ON_TIME_EARLIER       21
#define CMD_BREAK_ON_TIME_LATER         22
#define CMD_BREAK_ON_VAR_EQU            23
#define CMD_BREAK_ON_VAR_LESS           24
#define CMD_BREAK_ON_VAR_LESS_VAR       25
#define CMD_BREAK_ON_VAR_MORE           26
#define CMD_BREAK_ON_VAR_MORE_VAR       27
#define CMD_BREAK_ON_VAR_NEQU           28
#define VALUE_BUILD                     29
#define CMD_CLIENT_IP                   30
#define VALUE_SOURCE_CLOCK              31
#define EVENT_CLOCK_DAYLIGHT            32
#define CMD_CLOCK_DATE                  33
#define CMD_CLOCK_TIME                  34
#define CMD_CLOCK_SYNC                  35
#define VALUE_DLS                       36
#define CMD_DEBUG                       37
#define CMD_DELAY                       38
#define VALUE_SOURCE_PLUGIN             39
#define CMD_DNS_SERVER                  40
#define CMD_ECHO                        41
#define VALUE_RECEIVED_EVENT            42
#define VALUE_EVENTLIST                 43
#define VALUE_SOURCE_EVENTLIST          44
#define VALUE_EVENTLIST_COUNT           45
#define CMD_EVENTLIST_ERASE             46
#define CMD_EVENTLIST_FILE              47
#define CMD_EVENTLIST_SHOW              48
#define CMD_EVENTLIST_WRITE             49
#define VALUE_SOURCE_FILE               50
#define CMD_FILE_ERASE                  51
#define CMD_FILE_EXECUTE                52
#define CMD_FILE_GET_HTTP               53
#define CMD_FILE_LIST                   54
#define CMD_RES_55                      55                                      //??? reserve
#define CMD_FILE_SHOW                   56
#define CMD_FILE_WRITE                  57
#define VALUE_FREEMEM                   58
#define CMD_GATEWAY                     59
#define CMD_WAIT_FREE_RX                60 
#define VALUE_SOURCE_HTTP               61
#define CMD_HTTP_REQUEST                62
#define VALUE_HWCONFIG                  63
#define VALUE_SOURCE_I2C                64
#define CMD_ID                          65
#define CMD_IF                          66
#define VALUE_DIRECTION_INPUT           67
#define CMD_ESP_IP                      68
#define VALUE_SOURCE_IR                 69
#define CMD_LOCK                        70
#define CMD_LOG                         71
#define EVENT_MESSAGE                   72
#define EVENT_NEWNODO                   73
#define VALUE_OFF                       74
#define VALUE_ON                        75
#define CMD_OUTPUT                      76
#define VALUE_DIRECTION_OUTPUT          77
#define VALUE_RECEIVED_PAR1             78
#define VALUE_RECEIVED_PAR2             79
#define CMD_PASSWORD                    80
#define CMD_PORT_INPUT                  81
#define CMD_PORT_OUTPUT                 82
#define EVENT_RAWSIGNAL                 83
#define CMD_RAWSIGNAL_ERASE             84
#define CMD_RAWSIGNAL_LIST              85
#define CMD_RAWSIGNAL_RECEIVE           86
#define CMD_RAWSIGNAL_SAVE              87
#define CMD_RAWSIGNAL_SEND              88
#define CMD_REBOOT                      89
#define CMD_RECEIVE_SETTINGS            90
#define CMD_RESET                       91
#define VALUE_SOURCE_RF                 92
#define CMD_SEND_EVENT                  93
#define CMD_SENDTO                      94
#define CMD_SEND_USEREVENT              95
#define VALUE_SOURCE_SERIAL             96
#define CMD_SETTINGS_SAVE               97
#define CMD_STATUS                      98
#define CMD_VARIABLE_SAVE               99
#define CMD_SUBNET                      100
#define VALUE_SOURCE_SYSTEM             101
#define CMD_TEMP                        102
#define VALUE_SOURCE_TELNET             103
#define EVENT_TIME                      104
#define EVENT_TIMER                     105
#define CMD_TIMER_RANDOM                106
#define CMD_TIMER_SET                   107
#define CMD_TIMER_SET_VARIABLE          108
#define VALUE_UNIT                      109
#define CMD_UNIT_SET                    110
#define CMD_VARIABLE_DEC                111
#define CMD_VARIABLE_INC                112
#define CMD_VARIABLE_PULSE_COUNT        113
#define CMD_VARIABLE_PULSE_TIME         114
#define CMD_VARIABLE_GET                115
#define CMD_VARIABLE_SEND               116
#define CMD_VARIABLE_VARIABLE           117
#define CMD_VARIABLE_SET_WIRED_ANALOG   118
#define CMD_VARIABLE_ADD_VARIABLE       119
#define CMD_VARIABLE_TOGGLE             120
#define EVENT_WILDCARD                  121
#define VALUE_SOURCE_WIRED              122
#define VALUE_WIRED_ANALOG              123
#define EVENT_WIRED_IN                  124
#define CMD_WIRED_OUT                   125
#define CMD_WIRED_PULLUP                126
#define CMD_WIRED_SMITTTRIGGER          127
#define CMD_WIRED_THRESHOLD             128
#define CMD_FILE_WRITE_LINE             129
#define CMD_VARIABLE_MULTIPLY           130
#define CMD_RAWSIGNAL_SHOW              131
#define CMD_RAWSIGNAL_REPEATS           132
#define CMD_RAWSIGNAL_DELAY             133
#define CMD_RAWSIGNAL_PULSES            134
#define CMD_WIFI_DISCONNECT             135
#define CMD_WIFI_SCAN                   136
#define CMD_SERVER_PORT                 137
#define CMD_SERVER_IP                   138
#define CMD_WIFI_CONNECT                139
#define CMD_WIFI_SSID                   140
#define CMD_WIFI_KEY                    141
#define CMD_STOP                        142
#define CMD_DOMOTICZ_GET                143   
#define CMD_DOMOTICZ_SET                144
#define COMMAND_MAX                     144                                     // hoogste commando

#define MESSAGE_OK                      0
#define MESSAGE_UNKNOWN_COMMAND         1
#define MESSAGE_INVALID_PARAMETER       2
#define MESSAGE_UNABLE_OPEN_FILE        3
#define MESSAGE_NESTING_ERROR           4
#define MESSAGE_EVENTLIST_FAILED        5
#define MESSAGE_TCPIP_FAILED            6
#define MESSAGE_EXECUTION_STOPPED       7
#define MESSAGE_ACCESS_DENIED           8
#define MESSAGE_SENDTO_ERROR            9
#define MESSAGE_SDCARD_ERROR            10
#define MESSAGE_BREAK                   11
#define MESSAGE_RAWSIGNAL_SAVED         12 
#define MESSAGE_PLUGIN_UNKNOWN          13
#define MESSAGE_PLUGIN_ERROR            14
#define MESSAGE_VERSION_ERROR           15
#define MESSAGE_BUSY_TIMEOUT            16
#define MESSAGE_MAX                     16                                      // laatste bericht tekst

// Commando's die in de lijst een vaste positie hebben en houden
const char Cmd_0[]="-";
const char Cmd_1[]="Boot";
const char Cmd_2[]="Sound";
const char Cmd_3[]="UserEvent";
const char Cmd_4[]="Variable";
const char Cmd_5[]="VariableSet";
const char Cmd_6[]="";
const char Cmd_7[]="";
const char Cmd_8[]="";
const char Cmd_9[]="";
const char Cmd_10[]="";
const char Cmd_11[]="";
const char Cmd_12[]="";
const char Cmd_13[]="";
const char Cmd_14[]="";
const char Cmd_15[]="";
// Einde vaste positie


const char Cmd_16[]="";
const char Cmd_17[]="";
const char Cmd_18[]="All";
const char Cmd_19[]="AnalyseSettings";
const char Cmd_20[]="";
const char Cmd_21[]="";
const char Cmd_22[]="";
const char Cmd_23[]="BreakOnVarEqu";
const char Cmd_24[]="BreakOnVarLess";
const char Cmd_25[]="BreakOnVarLessVar";
const char Cmd_26[]="BreakOnVarMore";
const char Cmd_27[]="BreakOnVarMoreVar";
const char Cmd_28[]="BreakOnVarNEqu";
const char Cmd_29[]="Build";
const char Cmd_30[]="ClientIP";
const char Cmd_31[]="Clock";
const char Cmd_32[]="ClockDaylight";
const char Cmd_33[]="ClockSetDate";
const char Cmd_34[]="ClockSetTime";
const char Cmd_35[]="ClockSync";
const char Cmd_36[]="";
const char Cmd_37[]="Debug";
const char Cmd_38[]="Delay";
const char Cmd_39[]="Plugin";
const char Cmd_40[]="DnsServer";
const char Cmd_41[]="Echo";
const char Cmd_42[]="Event";
const char Cmd_43[]="EventList";
const char Cmd_44[]="Eventlist";
const char Cmd_45[]="EventlistCount";
const char Cmd_46[]="EventlistErase";
const char Cmd_47[]="";
const char Cmd_48[]="EventlistShow";
const char Cmd_49[]="EventlistWrite";
const char Cmd_50[]="";
const char Cmd_51[]="";
const char Cmd_52[]="";
const char Cmd_53[]="";
const char Cmd_54[]="";
const char Cmd_55[]="";
const char Cmd_56[]="";
const char Cmd_57[]="";
const char Cmd_58[]="FreeMem";
const char Cmd_59[]="Gateway";
const char Cmd_60[]="";
const char Cmd_61[]="";
const char Cmd_62[]="";
const char Cmd_63[]="";
const char Cmd_64[]="I2C";
const char Cmd_65[]="ID";
const char Cmd_66[]="";
const char Cmd_67[]="Input";
const char Cmd_68[]="IP";
const char Cmd_69[]="IR";
const char Cmd_70[]="";
const char Cmd_71[]="";
const char Cmd_72[]="Message";
const char Cmd_73[]="NewESP";
const char Cmd_74[]="Off";
const char Cmd_75[]="On";
const char Cmd_76[]="Output";
const char Cmd_77[]="Output";
const char Cmd_78[]="Par1";
const char Cmd_79[]="Par2";
const char Cmd_80[]="Password";
const char Cmd_81[]="";
const char Cmd_82[]="";
const char Cmd_83[]="";
const char Cmd_84[]="";
const char Cmd_85[]="";
const char Cmd_86[]="";
const char Cmd_87[]="";
const char Cmd_88[]="";
const char Cmd_89[]="Reboot";
const char Cmd_90[]="ReceiveSettings";
const char Cmd_91[]="Reset";
const char Cmd_92[]="RF";
const char Cmd_93[]="EventSend";
const char Cmd_94[]="SendTo";
const char Cmd_95[]="UserEventSend";
const char Cmd_96[]="Serial";
const char Cmd_97[]="SettingsSave";
const char Cmd_98[]="Status";
const char Cmd_99[]="VariableSave";
const char Cmd_100[]="Subnet";
const char Cmd_101[]="System";
const char Cmd_102[]="";
const char Cmd_103[]="";
const char Cmd_104[]="";
const char Cmd_105[]="Timer";
const char Cmd_106[]="TimerRandom";
const char Cmd_107[]="TimerSet";
const char Cmd_108[]="TimerSetVariable";
const char Cmd_109[]="Unit";
const char Cmd_110[]="UnitSet";
const char Cmd_111[]="VariableDec";
const char Cmd_112[]="VariableInc";
const char Cmd_113[]="VariablePulseCount";
const char Cmd_114[]="VariablePulseTime";
const char Cmd_115[]="VariableGet";
const char Cmd_116[]="VariableSend";
const char Cmd_117[]="VariableSetVariable";
const char Cmd_118[]="VariableWiredAnalog";
const char Cmd_119[]="VariableAddVariable";
const char Cmd_120[]="VariableToggle";
const char Cmd_121[]="WildCard";
const char Cmd_122[]="Wired";
const char Cmd_123[]="WiredAnalog";
const char Cmd_124[]="WiredIn";
const char Cmd_125[]="WiredOut";
const char Cmd_126[]="WiredPullup";
const char Cmd_127[]="WiredSmittTrigger";
const char Cmd_128[]="WiredThreshold";
const char Cmd_129[]="";
const char Cmd_130[]="VariableMultiply";
const char Cmd_131[]="";
const char Cmd_132[]="";
const char Cmd_133[]="";
const char Cmd_134[]="";
const char Cmd_135[]="WifiDisconnect";
const char Cmd_136[]="WifiScan";
const char Cmd_137[]="ServerPort";
const char Cmd_138[]="ServerIP";
const char Cmd_139[]="WifiConnect";
const char Cmd_140[]="WifiSSID";
const char Cmd_141[]="WifiKey";
const char Cmd_142[]="Stop";
const char Cmd_143[]="DomoticzGet";
const char Cmd_144[]="DomoticzSet";

// tabel die refereert aan de commando strings
const char *CommandText_tabel[]={
Cmd_0,Cmd_1,Cmd_2,Cmd_3,Cmd_4,Cmd_5,Cmd_6,Cmd_7,Cmd_8,Cmd_9,
Cmd_10,Cmd_11,Cmd_12,Cmd_13,Cmd_14,Cmd_15,Cmd_16,Cmd_17,Cmd_18,Cmd_19,
Cmd_20,Cmd_21,Cmd_22,Cmd_23,Cmd_24,Cmd_25,Cmd_26,Cmd_27,Cmd_28,Cmd_29,
Cmd_30,Cmd_31,Cmd_32,Cmd_33,Cmd_34,Cmd_35,Cmd_36,Cmd_37,Cmd_38,Cmd_39,
Cmd_40,Cmd_41,Cmd_42,Cmd_43,Cmd_44,Cmd_45,Cmd_46,Cmd_47,Cmd_48,Cmd_49,
Cmd_50,Cmd_51,Cmd_52,Cmd_53,Cmd_54,Cmd_55,Cmd_56,Cmd_57,Cmd_58,Cmd_59,
Cmd_60,Cmd_61,Cmd_62,Cmd_63,Cmd_64,Cmd_65,Cmd_66,Cmd_67,Cmd_68,Cmd_69,
Cmd_70,Cmd_71,Cmd_72,Cmd_73,Cmd_74,Cmd_75,Cmd_76,Cmd_77,Cmd_78,Cmd_79,
Cmd_80,Cmd_81,Cmd_82,Cmd_83,Cmd_84,Cmd_85,Cmd_86,Cmd_87,Cmd_88,Cmd_89,
Cmd_90,Cmd_91,Cmd_92,Cmd_93,Cmd_94,Cmd_95,Cmd_96,Cmd_97,Cmd_98,Cmd_99,
Cmd_100,Cmd_101,Cmd_102,Cmd_103,Cmd_104,Cmd_105,Cmd_106,Cmd_107,Cmd_108,Cmd_109,
Cmd_110,Cmd_111,Cmd_112,Cmd_113,Cmd_114,Cmd_115,Cmd_116,Cmd_117,Cmd_118,Cmd_119,
Cmd_120,Cmd_121,Cmd_122,Cmd_123,Cmd_124,Cmd_125,Cmd_126,Cmd_127,Cmd_128,Cmd_129,
Cmd_130,Cmd_131,Cmd_132,Cmd_133,Cmd_134,Cmd_135,Cmd_136,Cmd_137,Cmd_138,Cmd_139,
Cmd_140,Cmd_141,Cmd_142,Cmd_143,Cmd_144};

// Message max. 40 pos       "1234567890123456789012345678901234567890"
const char Msg_0[]  = "Ok.";
const char Msg_1[]  = "Unknown command.";
const char Msg_2[]  = "Invalid parameter in command.";
const char Msg_3[]  = "Unable to open.";
const char Msg_4[]  = "Nesting error.";
const char Msg_5[]  = "Reading/writing eventlist failed.";
const char Msg_6[]  = "Unable to establish TCP/IP connection.";
const char Msg_7[]  = "Execution stopped.";
const char Msg_8[]  = "Access denied.";
const char Msg_9[]  = "Communication error.";
const char Msg_10[] = "SDCard error.";
const char Msg_11[] = "Break.";
const char Msg_12[] = "RawSignal saved.";
const char Msg_13[] = "Unknown device.";
const char Msg_14[] = "Plugin returned an error.";
const char Msg_15[] = "Incompatibel Nodo event.";
const char Msg_16[] = "Timeout on busy ESP.";

// tabel die refereert aan de message strings
const char *MessageText_tabel[]={Msg_0,Msg_1,Msg_2,Msg_3,Msg_4,Msg_5,Msg_6,Msg_7,Msg_8,Msg_9,Msg_10,Msg_11,Msg_12,Msg_13,Msg_14,Msg_15,Msg_16};

// strings met vaste tekst naar PROGMEM om hiermee RAM-geheugen te sparen.
const char Text_01[] = "ESP Connexio, copyright (C) 2015 www.esp8266.nu";
const char Text_02[] = "Forked from the Nodo Project www.nodo-domotica.nl";
const char Text_03[] = "Major parts hold Copyright (C) 2015 by Paul Tonkes";
const char Text_04[] = "Licensed under GNU General Public License.";
const char Text_05[] = "";
const char Text_07[] = "Waiting...";
const char Text_08[] = "";
const char Text_09[] = "(Last 100 KByte)";
const char Text_10[] = "ESP unit %d is busy, please wait...";
const char Text_11[] = "";
const char Text_12[] = "";
const char Text_14[] = "Event=";
const char Text_16[] = "";
const char Text_17[] = "";
const char Text_18[] = "%s %u.%u.%u.%u";
const char Text_22[] = "!******************************************************************************!";
const char Text_23[] = "";
const char Text_30[] = "";

#define PLUGIN_MMI_IN                1
#define PLUGIN_MMI_OUT               2
#define PLUGIN_RAWSIGNAL_IN          3
#define PLUGIN_COMMAND               4 
#define PLUGIN_INIT                  5
#define PLUGIN_ONCE_A_SECOND         6
#define PLUGIN_EVENT_IN              7
#define PLUGIN_EVENT_OUT             8
#define PLUGIN_SERIAL_IN             9
#define PLUGIN_I2C_IN               10
#define PLUGIN_ETHERNET_IN          11
#define PLUGIN_SCAN_EVENT          255

#define RED                            1                                        // Led = Rood
#define GREEN                          2                                        // Led = Groen
#define BLUE                           3                                        // Led = Blauw
#define UNIT_MAX                      31                                        // Hoogst mogelijke unit nummer van een Nodo
#define SERIAL_TERMINATOR_1         0x0A                                        // Met dit teken wordt een regel afgesloten. 0x0A is een linefeed <LF>
#define SERIAL_TERMINATOR_2         0x00                                        // Met dit teken wordt een regel afgesloten. 0x0D is een Carriage Return <CR>, 0x00 = niet in gebruik.
#define SCAN_HIGH_TIME                50                                        // tijdsinterval in ms. voor achtergrondtaken snelle verwerking
#define SCAN_LOW_TIME               1000                                        // tijdsinterval in ms. voor achtergrondtaken langzame verwerking
#define PLUGIN_MAX                    32                                        // Maximaal aantal devices 
#define MACRO_EXECUTION_DEPTH         10                                        // maximale nesting van macro's.
#define XON                         0x11                                        // Seriale communicatie XON/XOFF handshaking
#define XOFF                        0x13                                        // Seriale communicatie XON/XOFF handshaking

// Hardware in gebruik: Bits worden geset in de variabele HW_Config, uit te lezen met [Status HWConfig]
#define HW_RF_RX       10
#define HW_IR_RX       11
#define HW_I2C         12
#define HW_WIRED_IN    13
#define HW_WIRED_OUT   14
#define HW_RF_TX       15
#define HW_IR_TX       16
#define HW_PULSE       18
#define HW_PLUGIN      19
#define HW_WIRED       20

#define INPUT_LINE_SIZE            128                                          // Buffer waar de karakters van de seriele/IP poort in worden opgeslagen.
#define INPUT_COMMAND_SIZE          80                                          // Maximaal aantal tekens waar een commando uit kan bestaan.
#define FOCUS_TIME                 500 
#define USER_VARIABLES_MAX          15                                          // aantal beschikbare gebruikersvariabelen voor de user.

#define EVENT_QUEUE_MAX              8                                          // maximaal aantal plaatsen in de queue
#define TIMER_MAX                    8                                          // aantal beschikbare timers voor de user, gerekend vanaf 1
#define PULSE_IRQ                    0                                          // IRQ-1 verbonden aan de IR_RX_DATA pen 3 van de ATMega328 (Uno/Nano/Duemillanove)
#define EEPROM_SIZE               4096                                          // Groote van het EEPROM geheugen.
#define WIRED_PORTS                  1                                          // aantal WiredIn/WiredOut poorten
#define PIN_LED_RGB_R               13                                          // RGB-Led, aansluiting rood
#define PIN_LED_RGB_B               13                                          // RGB-Led, aansluiting blauw, maar voor de Nodo Small is dit de eveneens de rode led.
#define PIN_WIRED_IN_1               0                                          // Wired-In 1 t/m 4 aangesloten op A0 t/m A3
#define PIN_WIRED_IN_2               1                                          // Wired-In 1 t/m 4 aangesloten op A0 t/m A3
#define PIN_WIRED_IN_3               2                                          // Wired-In 1 t/m 4 aangesloten op A0 t/m A3
#define PIN_WIRED_IN_4               3                                          // Wired-In 1 t/m 4 aangesloten op A0 t/m A3
#define PIN_SPEAKER                  6                                          // Luidspreker aansluiting
#define PIN_IR_TX_DATA              11                                          // Zender IR-Led. (gebufferd via transistor i.v.m. hogere stroom die nodig is voor IR-led)
#define PIN_IR_RX_DATA               3                                          // Op deze input komt het IR signaal binnen van de TSOP. Bij HIGH bij geen signaal.
#define PIN_RF_TX_VCC                4                                          // +5 volt / Vcc spanning naar de zender.
#define PIN_RF_TX_DATA               2                                          // data naar de zender
#define PIN_RF_RX_DATA               0                                          // Op deze input komt het 433Mhz-RF signaal binnen. LOW bij geen signaal.
#define PIN_RF_RX_VCC               12                                          // Spanning naar de ontvanger via deze pin.
#define PIN_WIRED_OUT_1              2                                          // 4 digitale outputs D07 t/m D10 worden gebruikt voor WiredOut 1 tot en met 4
#define PIN_WIRED_OUT_2              8                                          // 4 digitale outputs D07 t/m D10 worden gebruikt voor WiredOut 1 tot en met 4
#define PIN_WIRED_OUT_3              9                                          // (pwm) 4 digitale outputs D07 t/m D10 worden gebruikt voor WiredOut 1 tot en met 4
#define PIN_WIRED_OUT_4             10                                          // (pwm) 4 digitale outputs D07 t/m D10 worden gebruikt voor WiredOut 1 tot en met 4
#define PIN_I2C_SDA                 A4                                          // I2C communicatie lijn voor de o.a. de realtime clock.
#define PIN_I2C_SLC                 A5                                          // I2C communicatie lijn voor de o.a. de realtime clock.

// In het transport deel van een Nodo event kunnen zich de volgende vlaggen bevinden:
#define TRANSMISSION_BUSY                                1
#define TRANSMISSION_RUBBERDUCK                        128
#define TRANSMISSION_QUEUE                               2                      // Event maakt deel uit van een reeks die in de queue geplaatst moet worden
#define TRANSMISSION_QUEUE_NEXT                          4                      // Event maakt deel uit van een reeks die in de queue geplaatst moet worden
#define TRANSMISSION_SENDTO                              8                      // Aan deze vlag kunnen we herkennen dat een event deel uit maakt van een SendTo reeks. 
#define TRANSMISSION_CONFIRM                            16                      // Verzoek aan master om bevestiging te sturen na ontvangst.
#define TRANSMISSION_VIEW                               32                      // Uitsluitend het event weergeven, niet uitvoeren
#define TRANSMISSION_VIEW_SPECIAL                       64                      // Uitsluitend het event weergeven, niet uitvoeren

// Er zijn een aantal type Nodo events die op verschillende wijze worden behandeld:
#define ESP_TYPE_EVENT                                  1
#define ESP_TYPE_COMMAND                                2
#define ESP_TYPE_SYSTEM                                 3               
#define ESP_TYPE_PLUGIN_EVENT                           4
#define ESP_TYPE_PLUGIN_COMMAND                         5
#define ESP_TYPE_RAWSIGNAL                              6

// De Nodo kent naast gebruikers commando's en events eveneens Nodo interne events
#define SYSTEM_COMMAND_CONFIRMED                         1
#define SYSTEM_COMMAND_RTS                               2
#define SYSTEM_COMMAND_CTS                               3
#define SYSTEM_COMMAND_QUEUE_SENDTO                      4                      // Dit is aankondiging reeks, dus niet het user comando "SendTo".
#define SYSTEM_COMMAND_QUEUE_EVENTLIST_SHOW              5                      // Dit is aankondiging reeks, dus niet het user comando "EventlistShow".
#define SYSTEM_COMMAND_QUEUE_EVENTLIST_WRITE             6                      // Dit is aankondiging reeks, dus niet het user comando "EventlistShow".
#define SYSTEM_COMMAND_QUEUE_POLL                        7                      // Verzoek om een poll naar een andere Nodo die moet worden beantwoord.

//****************************************************************************************************************************************

struct SettingsStruct
  {
  byte    Unit; // Max 5 bits in gebruik = 1..31
  boolean NewNodo;
  byte    WaitFreeNodo;
  byte    TransmitIR;
  byte    TransmitRF;
  byte    WaitFree;
  byte    RawSignalReceive;
  byte    RawSignalSample;
  unsigned long Lock;
  char    WifiSSID[26];
  char    WifiKey[26];
  byte    Server_IP[4];
  unsigned int ServerPort;
  
  #if WIRED
  int     WiredInputThreshold[WIRED_PORTS], WiredInputSmittTrigger[WIRED_PORTS];
  byte    WiredInputPullUp[WIRED_PORTS];
  #endif
  
  int     Version;                                                              // Onjuiste versie in EEPROM zorgt voor een [Reset]
  }Settings;

 // Niet alle gegevens uit een event zijn relevant. Om ruimte in EEPROM te besparen worden uitsluitend
 // de alleen noodzakelijke gegevens in EEPROM opgeslagen. Hiervoor een struct vullen die later als
 // blok weggeschreven kan worden.

 struct EventlistStruct
   {
   byte EventType;
   byte EventCommand;
   byte EventPar1;
   unsigned long EventPar2;
   
   byte ActionType;
   byte ActionCommand;
   byte ActionPar1;
   unsigned long ActionPar2;
   };
  
struct NodoEventStruct
  {
  // Event deel
  byte Type;
  byte Command;
  byte Par1;
    byte Dummy;
  unsigned long Par2;

  // Transmissie deel
  byte SourceUnit;
  byte DestinationUnit;
  byte Flags;
  byte Port;
  byte Direction;
  byte Version;
  byte Checksum;
  };

 // Van alle devices die worden mee gecompileerd, worden in een tabel de adressen opgeslagen zodat
// hier naar toe gesprongen kan worden
void PluginInit(void);
boolean (*Plugin_ptr[PLUGIN_MAX])(byte, struct NodoEventStruct*, char*);
void (*FastLoopCall_ptr)(void);
byte Plugin_id[PLUGIN_MAX];
boolean ExecuteCommand(NodoEventStruct *EventToExecute);                        //protoype definieren.

volatile unsigned long PulseCount=0L;                                           // Pulsenteller van de IR puls. Iedere hoog naar laag transitie wordt deze teller met Ã©Ã©n verhoogd
volatile unsigned long PulseTime=0L;                                            // Tijdsduur tussen twee pulsen teller in milliseconden: millis()-vorige meting.
unsigned long HoldTransmission=0L;                                              // wachten op dit tijdstip in millis() alvorens event te verzenden.
unsigned long BusyNodo=0;                                                       // In deze variabele wordt per bitpositie aangegeven of een Nodo unitnummer busy is.
boolean Transmission_ESPOnly=false;                                            // Als deze vlag staat, dan worden er uitsluitend Nodo-eigen signalen ontvangen.  
byte QueuePosition=0;
unsigned long UserTimer[TIMER_MAX];                                             // Timers voor de gebruiker.
byte DaylightPrevious;                                                          // t.b.v. voorkomen herhaald genereren van events binnen de lopende minuut waar dit event zich voordoet.
byte ExecutionDepth=0;                                                          // teller die bijhoudt hoe vaak er binnen een macro weer een macro wordt uitgevoerd. Voorkomt tevens vastlopers a.g.v. loops die door een gebruiker zijn gemaakt met macro's.
int ExecutionLine=0;                                                            // Regel in de eventlist die in uitvoer is.??? wordt deze nog gebruikt.
void(*Reboot)(void)=0;                                                          // reset functie op adres 0.
uint8_t RFbit,RFport,IRbit,IRport;                                              // t.b.v. verwerking IR/FR signalen.
float UserVar[USER_VARIABLES_MAX];                                              // Gebruikers variabelen
unsigned long HW_Config=0;                                                      // Hardware configuratie zoals gedetecteerd door de Nodo. 
struct NodoEventStruct LastReceived;                                            // Laatst ontvangen event
byte RequestForConfirm=false;                                                   // Als true dan heeft deze Nodo een verzoek ontvangen om een systemevent 'Confirm' te verzenden. Waarde wordt in Par1 meegezonden.
int EventlistMax=0;                                                             // beschikbaar aantal regels in de eventlist. Wordt tijdens setup berekend.

#if WIRED
boolean WiredInputStatus[WIRED_PORTS];                                          // Status van de WiredIn worden hierin opgeslagen.
boolean WiredOutputStatus[WIRED_PORTS];                                         // Wired variabelen.
#endif

byte  Transmission_SendToUnit=0;                                                // Unitnummer waar de events naar toe gestuurd worden. 0=alle.
byte  Transmission_SendToAll=0;                                                 // Waarde die aangeeft of het SendTo permanent staat ingeschakeld. 0=uit, in andere gevallen het unitnummer.
boolean Transmission_SendToFast=false;                                          // Vlag die aangeeft of de SendTo via de snelle modus (zonder handshaking) plaats moet vinden.

// ESP8266
char InputBuffer_Serial[INPUT_LINE_SIZE];                                       // Buffer voor input Seriele data

#define RAW_BUFFER_SIZE            256                                          // Maximaal aantal te ontvangen 128 bits is voldoende voor capture meeste signalen.
struct RawSignalStruct                                                          // Variabelen geplaatst in struct zodat deze later eenvoudig kunnen worden weggeschreven naar SDCard
  {
  boolean Available; //ESP mod
  byte Source;                                                                  // Bron waar het signaal op is binnengekomen.
  int  Number;                                                                  // aantal bits, maal twee omdat iedere bit een mark en een space heeft.
  byte Repeats;                                                                 // Aantal maal dat de pulsreeks verzonden moet worden bij een zendactie.
  byte Delay;                                                                   // Pauze in ms. na verzenden van Ã©Ã©n enkele pulsenreeks
  byte Multiply;                                                                // Pulses[] * Multiply is de echte tijd van een puls in microseconden
  boolean RepeatChecksum;                                                       // Als deze vlag staat moet er eentweede signaal als checksum binnenkomen om een geldig event te zijn. 
  unsigned long Time;                                                           // Tijdstempel wanneer signaal is binnengekomen (millis())
  byte Pulses[RAW_BUFFER_SIZE+2];                                               // Tabel met de gemeten pulsen in microseconden gedeeld door RawSignal.Multiply. Dit scheelt helft aan RAM geheugen.
                                                                                // Om legacy redenen zit de eerste puls in element 1. Element 0 wordt dus niet gebruikt.
  }RawSignal={0,0,0,0,0,0,0L};

void handle_root() {
  Serial.println("Webrequest!");
  String webrequest = server.arg("cmd");
  webrequest.replace("%20", " ");
  Serial.println(webrequest);
  char command[80];
  command[0]=0;
  webrequest.toCharArray(command,79);
  RaiseMessage(ExecuteLine(command,VALUE_SOURCE_SERIAL),0);
  server.send(200, "text/plain", "Hello from ESP Connexio!");
  delay(100);
}

void setup() 
  {
  Serial.begin(BAUD);                                                           // Initialiseer de seriele poort
  Serial.println("\n");
  
  EEPROM.begin(4096);
    
  int x;
  struct NodoEventStruct TempEvent;

  x=(EEPROM_SIZE-sizeof(struct SettingsStruct))/sizeof(struct EventlistStruct); // bereken aantal beschikbare eventlistregels in het eeprom geheugen
  EventlistMax=x>255?255:x;

  for(x=0;x<TIMER_MAX;x++)                                                      // Alle timers op nul zetten.
    UserTimer[x]=0L;

  // Initialiseer in/output poorten.
  pinMode(PIN_IR_RX_DATA, INPUT);
  pinMode(PIN_RF_RX_DATA, INPUT);
  pinMode(PIN_RF_TX_DATA, OUTPUT);
  pinMode(PIN_RF_TX_VCC,  OUTPUT);
  pinMode(PIN_RF_RX_VCC,  OUTPUT);
  pinMode(PIN_IR_TX_DATA, OUTPUT);
  pinMode(PIN_LED_RGB_R,  OUTPUT);
  pinMode(PIN_SPEAKER,    OUTPUT);
  pinMode(PIN_LED_RGB_B,  OUTPUT);
  digitalWrite(PIN_IR_TX_DATA,LOW);                                             // Zet de IR zenders initiÃ«el uit! Anders mogelijk overbelasting !
  digitalWrite(PIN_RF_RX_VCC,HIGH);                                             // Spanning naar de RF ontvanger aan.
  digitalWrite(PIN_IR_RX_DATA,INPUT_PULLUP);                                    // schakel pull-up weerstand in om te voorkomen dat er rommel binnenkomt als pin niet aangesloten.
  digitalWrite(PIN_RF_RX_DATA,INPUT_PULLUP);                                    // schakel pull-up weerstand in om te voorkomen dat er rommel binnenkomt als pin niet aangesloten.

  RFbit=digitalPinToBitMask(PIN_RF_RX_DATA);
  RFport=digitalPinToPort(PIN_RF_RX_DATA);  
  IRbit=digitalPinToBitMask(PIN_IR_RX_DATA);
  IRport=digitalPinToPort(PIN_IR_RX_DATA);

  Led(RED);

  ClearEvent(&LastReceived);

  LoadSettings();                                                               // laad alle settings zoals deze in de EEPROM zijn opgeslagen

  if(Settings.Version!=ESP_VERSION_MINOR)ResetFactory();                       // De Nodo resetten als Versie van de settings zoals geladen vanuit EEPROM niet correct is.

  #if WIRED  
  bitWrite(HW_Config,HW_WIRED,1);
  for(x=0;x<WIRED_PORTS;x++)                                                    // initialiseer de Wired ingangen.
    {
    if(Settings.WiredInputPullUp[x]==VALUE_ON)
      pinMode(PIN_WIRED_IN_1+x,INPUT_PULLUP);
    else
      pinMode(PIN_WIRED_IN_1+x,INPUT);

    pinMode(PIN_WIRED_OUT_1+x,OUTPUT);                                          // definieer Arduino pin's voor Wired-Out
    WiredInputStatus[x]=true;                                                   // Status van de wired poort setten zodat er niet onterect bij start al events worden gegenereerd.
    }
  #endif
  
  #if I2C
  Wire.pins(0,2); // ESP pins to be used for I2C
  Wire.begin();
  #endif

  // Alle devices moeten aan te roepen zijn vanuit de Pluginnummers zoals die in de events worden opgegeven
  // initialiseer de lijst met pointers naar de device funkties.

  PluginInit();

  #if I2C
  bitWrite(HW_Config,HW_I2C,true); // Zet I2C aan zodat het boot event op de I2C-bus wordt verzonden. Hiermee worden bij de andere Nodos de I2C geactiveerd.
  #endif

  PrintWelcome();                                                               // geef de welkomsttekst weer
  
  ClearEvent(&TempEvent);
  TempEvent.Port      = VALUE_ALL;
  TempEvent.Type      = ESP_TYPE_EVENT;
  TempEvent.Direction = VALUE_DIRECTION_OUTPUT;
  TempEvent.Par1      = Settings.Unit;

  if(Settings.NewNodo)
    TempEvent.Command  = EVENT_NEWNODO;
  else
    TempEvent.Command   = EVENT_BOOT;
  SendEvent(&TempEvent,false,true,Settings.WaitFree==VALUE_ON);                 // Zend boot event naar alle Nodo's.  

  #if I2C
  bitWrite(HW_Config,HW_I2C,false);                                             // Zet I2C weer uit. Wordt weer geactiveerd als er een I2C event op de bus verschijnt.
  #endif
  
  TempEvent.Direction = VALUE_DIRECTION_INPUT;
  TempEvent.Port      = VALUE_SOURCE_SYSTEM;
  TempEvent.Type      = ESP_TYPE_EVENT;
  ProcessEvent(&TempEvent);                                                     // Voer het boot event zelf ook uit.
  
  QueueProcess();

  server.on("/", handle_root);
  server.begin();

  #if RF_IRQ
    attachInterrupt(0,RF_ISR,CHANGE);
    RawSignal.Multiply=25;
  #endif
  
  Serial.println(F("\nReady. ESP Connexio is waiting for serial input...\n"));
/*  for (byte x=0; x<127; x++)
    {
      Wire.beginTransmission(x);
      Serial.print((int)x);
      Serial.print("-");
      Serial.println(Wire.endTransmission());
    }
 */
}

void loop() 
  {
  int x,y,z;
  byte SerialInByte;
  int SerialInByteCounter=0;
  byte Slice_1=0;    
  struct NodoEventStruct ReceivedEvent;
  byte PreviousMinutes=0;                                                       // Sla laatst gecheckte minuut op zodat niet vaker dan nodig (eenmaal per minuut de eventlist wordt doorlopen
  
  unsigned long LoopIntervalTimer=0L;                                           // Timer voor periodieke verwerking.
  unsigned long FocusTimer;                                                     // Timer die er voor zorgt dat bij communicatie via een kanaal de focus hier ook enige tijd op blijft
  unsigned long PreviousTimeEvent=0L; 

  InputBuffer_Serial[0]=0;                                                      // serieel buffer string leeg maken

  // hoofdloop: scannen naar signalen
  // dit is een tijdkritische loop die wacht tot binnengekomen event op IR, RF, I2C, SERIAL, CLOCK, DAYLIGHT, TIMER, etc
  // Er is bewust NIET gekozen voor opvangen van signalen via IRQ's omdat er dan communicatie problemen ontstaan.

  while(true)
    {
      yield();
      server.handleClient();

     //Check voor IR, RF events
    if(ScanEvent(&ReceivedEvent))
      { 
      ProcessEvent(&ReceivedEvent);                                             // verwerk binnengekomen event.
      ProcessingStatus(false);
      Slice_1=0;
      }
      
    QueueProcess();
 
    // SERIAL: *************** kijk of er data klaar staat op de seriele poort **********************
    if(Serial.available())
      {
      PluginCall(PLUGIN_SERIAL_IN,0,0);

      FocusTimer=millis()+FOCUS_TIME;

      while(FocusTimer>millis())                                                // blijf even paraat voor luisteren naar deze poort en voorkom dat andere input deze communicatie onderbreekt.
        {
        yield();
        if(Serial.available())
          {                        
          SerialInByte=Serial.read();                

          Serial.write(SerialInByte);                                         // echo ontvangen teken
          
          if(SerialInByte==XON)
            Serial.write(XON);                                                  // om te voorkomen dat beide devices om wat voor reden dan ook op elkaar wachten
          
          if(isprint(SerialInByte))
            if(SerialInByteCounter<(INPUT_LINE_SIZE-1))
              InputBuffer_Serial[SerialInByteCounter++]=SerialInByte;
              
          if(SerialInByte=='\n')
            {
            InputBuffer_Serial[SerialInByteCounter]=0;                          // serieel ontvangen regel is compleet
            RaiseMessage(ExecuteLine(InputBuffer_Serial,VALUE_SOURCE_SERIAL),0);
            Serial.write('>');                                                  // Prompt
            SerialInByteCounter=0;  
            InputBuffer_Serial[0]=0;                                            // serieel ontvangen regel is verwerkt. String leegmaken
            ProcessingStatus(false);
            }
          FocusTimer=millis()+FOCUS_TIME;                                             
          }
        }
      }

    // Een plugin mag tijdelijk een claim doen op de snelle aanroep vanuit de hoofdloop. 
    // Als de waarde FastLoopCall_ptr is gevuld met het adres van een funktie dan wordt de betreffende funktie eenmaal
    // per FAST_LOOP mSec aangeroepen.
    if(FastLoopCall_ptr)
      FastLoopCall_ptr();

    switch(Slice_1++)
      {        
      case 0:
        {
        break;
        }
 
      case 1:
        {
        break;
        }
      
      case 2:
        {
        #if !RF_IRQ
        #if WIRED
        // WIRED: *************** kijk of statussen gewijzigd zijn op WIRED **********************  
        // als de huidige waarde groter dan threshold EN de vorige keer was dat nog niet zo DAN event genereren
        for(x=0;x<WIRED_PORTS;x++)
          {
          // lees analoge waarde. Dit is een 10-bit waarde, unsigned 0..1023
          y=digitalRead(PIN_WIRED_IN_1+x);
          z=false; // verandering
          
          if(WiredInputStatus[x] != y)
            {
            WiredInputStatus[x]=y;
            z=true;
            }
  
          if(z)
            {
            bitWrite(HW_Config,HW_WIRED_IN,true);
            ClearEvent(&ReceivedEvent);
            ReceivedEvent.Type             = ESP_TYPE_EVENT;
            ReceivedEvent.Command          = EVENT_WIRED_IN;
            ReceivedEvent.Par1             = x+1;
            ReceivedEvent.Par2             = WiredInputStatus[x]?VALUE_ON:VALUE_OFF;
            ReceivedEvent.Direction        = VALUE_DIRECTION_INPUT;
            ReceivedEvent.Port             = VALUE_SOURCE_WIRED;
            ProcessEvent(&ReceivedEvent); // verwerk binnengekomen event.
            }
          }
        #endif
        #endif
        break;
        }
        
      default:
        ProcessingStatus(false);
        Slice_1=0;
        break;
      }// switch


    // Niet tijdkritische processen die periodiek uitgevoerd moeten worden
    if(LoopIntervalTimer<millis())// lange interval: Iedere seconde.
      {
      LoopIntervalTimer=millis()+SCAN_LOW_TIME; // reset de timer  

      // PLUGIN: **************** Loop iedere seconde de plugins langs voor onderhoudstaken ***********************
      PluginCall(PLUGIN_ONCE_A_SECOND,&ReceivedEvent,0);

      // TIMER: **************** Genereer event als een timer voor de gebruiker afgelopen is ***********************    
      for(x=0;x<TIMER_MAX;x++)
        {
        if(UserTimer[x]!=0L)// als de timer actief is
          {
          if(UserTimer[x]<millis()) // als de timer is afgelopen.
            {
            UserTimer[x]=0L;// zet de timer op inactief.

            ClearEvent(&ReceivedEvent);
            ReceivedEvent.Type             = ESP_TYPE_EVENT;
            ReceivedEvent.Command          = EVENT_TIMER;
            ReceivedEvent.Par1             = x+1;
            ReceivedEvent.Direction        = VALUE_DIRECTION_INPUT;
            ReceivedEvent.Port             = VALUE_SOURCE_CLOCK;
            ProcessEvent(&ReceivedEvent); // verwerk binnengekomen event.
            }
          }
        }
      }
    }// while 
  }

