//#######################################################################################################
//#################################### Plugin-21: LCD I2C 1602 ##########################################
//#######################################################################################################

/*********************************************************************************************\
* Support            : www.esp8266.nu
* Date               : Apr 2015
* Compatibility      : R003
* Syntax             : "LCDWrite <row>, <column>, <command>, <option>
*
*                       Command:	Message		<option> = ID in plugin label definities
*					Variable	<option> = Variabele nummer
*					Clock		toont datum en tijd		
*					IP		toont IP adres (alleen Mega)
*					PortInput	toont input port voor http verkeer (alleen Mega)
*					Event		toont laatste event (alleen Mega)
*
*                       Special commands:
*					<x>,<0>, Reset	wist regel x
*					<0>,<0>, Reset	wist scherm
*					<0>,<0>, On	backlight aan
*					<0>,<0>, Off	backlight uit
***********************************************************************************************
* Technical description:
* This protocol provides support for I2C LCD Displays
* Two types are supported, 1602 (2 x 16 chars) and 2004 (4 x 20 chars)
* Tested on a I2C/TWI LCD1602 from DFRobot and a Funduino I2C LCD2004
* The LCDI2C1602 is a LCD Display that has two lines of 16 charactes each.
* The module is driven through the I2C (twowire) protocol
* The convertor board uses a Philips PCF8574 at I2C address 0x27
* If other PCF8574 chips are uses on the same bus, adjust the units address to something else than 0x27!
* Only one (1) display can be attached to the I2C bus
\*********************************************************************************************/

#define PLUGIN_ID_021        21
#define PLUGIN_NAME_021      "LCDWrite"

const char LCD_01[] = "ESP Connexio";
const char LCD_02[] = "R:%03d U:%d";
const char LCD_03[] = "";
const char LCD_04[] = PLUGIN_021_LABEL_04;
const char LCD_05[] = PLUGIN_021_LABEL_05;
const char LCD_06[] = PLUGIN_021_LABEL_06;
const char LCD_07[] = PLUGIN_021_LABEL_07;
const char LCD_08[] = PLUGIN_021_LABEL_08;
const char LCD_09[] = PLUGIN_021_LABEL_09;
const char LCD_10[] = PLUGIN_021_LABEL_10;
const char LCD_11[] = PLUGIN_021_LABEL_11;
const char LCD_12[] = PLUGIN_021_LABEL_12;
const char LCD_13[] = PLUGIN_021_LABEL_13;
const char LCD_14[] = PLUGIN_021_LABEL_14;
const char LCD_15[] = PLUGIN_021_LABEL_15;
const char LCD_16[] = PLUGIN_021_LABEL_16;
const char LCD_17[] = PLUGIN_021_LABEL_17;
const char LCD_18[] = PLUGIN_021_LABEL_18;
const char LCD_19[] = PLUGIN_021_LABEL_19;
const char LCD_20[] = PLUGIN_021_LABEL_20;

#define LCDI2C_MSG_MAX        20

const char *LCDText_tabel[]={LCD_01,LCD_02,LCD_03,LCD_04,LCD_05,LCD_06,LCD_07,LCD_08,LCD_09,LCD_10,LCD_11,LCD_12,LCD_13,LCD_14,LCD_15,LCD_16,LCD_17,LCD_18,LCD_19,LCD_20};

#define LCD_I2C_ADDRESS 0x27

#if PLUGIN_021==2
  #define PLUGIN_021_ROWS  2
  #define PLUGIN_021_COLS 16
#else
  #define PLUGIN_021_ROWS  4
  #define PLUGIN_021_COLS 20
#endif

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

void LCD_I2C_init();
void LCD_I2C_printline(byte row, byte col, char* message);
inline size_t LCD_I2C_write(uint8_t value);
void LCD_I2C_display();
void LCD_I2C_clear();
void LCD_I2C_home();
void LCD_I2C_setCursor(uint8_t col, uint8_t row);
inline void LCD_I2C_command(uint8_t value);
void LCD_I2C_send(uint8_t value, uint8_t mode);
void LCD_I2C_write4bits(uint8_t value);
void LCD_I2C_expanderWrite(uint8_t _data);                                        
void LCD_I2C_pulseEnable(uint8_t _data);

uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;
uint8_t _numlines;
uint8_t _backlightval=LCD_BACKLIGHT;

boolean Plugin_021(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;

  switch(function)
   {
   case PLUGIN_INIT:
     {
       LCD_I2C_init();
       break;
     }

  case PLUGIN_COMMAND:
     {
     byte Par2=event->Par2 & 0xff;		// Column
     byte Par3=event->Par2>>8 & 0xff;		// Data to display
     byte Par4=event->Par2>>16 & 0xff;		// In case of var, variable number
     boolean Print = true;

     if (event->Par1 >= 0 && event->Par1 <= PLUGIN_021_ROWS)
       {
       char TempString[80];
       TempString[0]=0;

       switch (Par3)
         {
           case 0:
           case CMD_RESET:
             if (event->Par1 == 0)
               LCD_I2C_clear();
             else
               LCD_I2C_printline(event->Par1-1,0, (char*)"");
             Print=false;
             break;

           case EVENT_BOOT:
             LCD_I2C_init();
             break;

           case VALUE_ON:
             _backlightval=LCD_BACKLIGHT;
             LCD_I2C_expanderWrite(0);
             Print=false;
             break;

           case VALUE_OFF:
             _backlightval=LCD_NOBACKLIGHT;
             LCD_I2C_expanderWrite(0);
             Print=false;
             break;

           case EVENT_MESSAGE:
             if ((Par4 > 0) && (Par4 <= LCDI2C_MSG_MAX))
               {
                 sprintf(TempString,"%s", LCDText_tabel[Par4-1]);
                 LCD_I2C_printline(event->Par1-1, Par2-1, TempString);
               }
             break;

           case EVENT_VARIABLE:
             if (Par4 > 0 && Par4 <16)
               {
                 int d1 = UserVar[Par4-1];            // Get the integer part
                 float f2 = UserVar[Par4-1] - d1;     // Get fractional part
                 int d2 = trunc(f2 * 10);   // Turn into integer
                 if (d2<0) d2=d2*-1;
                 sprintf(TempString,"%d.%01d", d1,d2);
                 LCD_I2C_printline(event->Par1-1, Par2-1, TempString);
               }
             break;

         }  // case

         //if (Print)
         //  LCD_I2C_printline(event->Par1-1, Par2-1, TempString);

         Wire.endTransmission(true);
         success=true;
       } // if

     break;
     }

   case PLUGIN_MMI_IN:
     {
     char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);
     
     if(GetArgv(string,TempStr,1))
       {
       if(strcasecmp(TempStr,PLUGIN_NAME_021)==0)
         {
         if(event->Par1 >= 0 && event->Par1 <= PLUGIN_021_ROWS)
           {

           if(GetArgv(string,TempStr,4))
               event->Par2|=str2cmd(TempStr)<<8;

           if(GetArgv(string,TempStr,5))
               event->Par2|=str2int(TempStr)<<16;

           event->Type = ESP_TYPE_PLUGIN_COMMAND;
           event->Command = PLUGIN_ID_021;
           success=true;
           }
         }
       }
     free(TempStr);
     break;
     }

   case PLUGIN_MMI_OUT:
     {
     //strcpy(string,PLUGIN_NAME_021);
     //strcat(string," ");
     //strcat(string,int2str(event->Par1));
     //strcat(string,",");
     //strcat(string,int2str(event->Par2 & 0xff));
     //strcat(string,",");
     //strcat(string,cmd2str(event->Par2>>8 & 0xff));
     //strcat(string,",");
     //strcat(string,int2str(event->Par2>>16 & 0xff));
     sprintf(string,"%s",PLUGIN_NAME_021);
     sprintf(string,"%s%s",string," ");
     sprintf(string,"%s%s",string,int2str(event->Par1));
     sprintf(string,"%s%s",string,",");
     sprintf(string,"%s%s",string,int2str(event->Par2 & 0xff));
     sprintf(string,"%s%s",string,",");
     sprintf(string,"%s%s",string,cmd2str(event->Par2>>8 & 0xff));
     sprintf(string,"%s%s",string,",");
     sprintf(string,"%s%s",string,int2str(event->Par2>>16 & 0xff));
     break;
     }
 }      
 return success;
}

/*********************************************************************/
void LCD_I2C_init()
/*********************************************************************/
{
     _displayfunction = LCD_2LINE;
     _numlines = PLUGIN_021_ROWS;
     delay(50); 
     // Now we pull both RS and R/W low to begin commands
     LCD_I2C_expanderWrite(_backlightval);	// reset expander and turn backlight off (Bit 8 =1)
     delay(1000);

     //put the LCD into 4 bit mode, this is according to the hitachi HD44780 datasheet, figure 24, pg 46
     LCD_I2C_write4bits(0x03 << 4);        // we start in 8bit mode, try to set 4 bit mode
     delayMicroseconds(4500);              // wait min 4.1ms
     LCD_I2C_write4bits(0x03 << 4);        // second try
     delayMicroseconds(4500);              // wait min 4.1ms
     LCD_I2C_write4bits(0x03 << 4);        // third go!
     delayMicroseconds(150);
     LCD_I2C_write4bits(0x02 << 4);        // finally, set to 4-bit interface
     LCD_I2C_command(LCD_FUNCTIONSET | _displayfunction);              // set # lines, font size, etc.
     _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;   // turn the display on with no cursor or blinking default
     LCD_I2C_display();
     LCD_I2C_clear();                                                  // clear it off
     _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;           // Initialize to default text direction (for roman languages)
     LCD_I2C_command(LCD_ENTRYMODESET | _displaymode);                 // set the entry mode
     LCD_I2C_home();

     LCD_I2C_printline(0,0,(char*)LCD_01);
     char TempString[PLUGIN_021_COLS+1];
     sprintf(TempString,LCD_02, ESP_BUILD, Settings.Unit);
     LCD_I2C_printline(1,0,TempString);
     Wire.endTransmission(true);
}

/*********************************************************************/
void LCD_I2C_printline(byte row, byte col, char* message)
/*********************************************************************/
{
 LCD_I2C_setCursor(col,row);
 byte maxcol = PLUGIN_021_COLS-col;

 //clear line if empty message
 if (message[0]==0)
   for (byte x=0; x<PLUGIN_021_COLS; x++) LCD_I2C_write(' ');
 else
   for (byte x=0; x < maxcol; x++)
     {
       if (message[x] != 0) LCD_I2C_write(message[x]);
       else break;
     }
}

/*********************************************************************/
inline size_t LCD_I2C_write(uint8_t value)
/*********************************************************************/
{
LCD_I2C_send(value, Rs);
return 0;
}

/*********************************************************************/
void LCD_I2C_display() {
/*********************************************************************/
 _displaycontrol |= LCD_DISPLAYON;
 LCD_I2C_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

/*********************************************************************/
void LCD_I2C_clear(){
/*********************************************************************/
 LCD_I2C_command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
 delayMicroseconds(2000);  // this command takes a long time!
}

/*********************************************************************/
void LCD_I2C_home(){
/*********************************************************************/
 LCD_I2C_command(LCD_RETURNHOME);  // set cursor position to zero
 delayMicroseconds(2000);  // this command takes a long time!
}

/*********************************************************************/
void LCD_I2C_setCursor(uint8_t col, uint8_t row){
/*********************************************************************/
 int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
 if ( row > _numlines ) {
row = _numlines-1;    // we count rows starting w/0
 }
 LCD_I2C_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

/*********************************************************************/
inline void LCD_I2C_command(uint8_t value) {
/*********************************************************************/
 LCD_I2C_send(value, 0);
}

/*********************************************************************/
void LCD_I2C_send(uint8_t value, uint8_t mode) {
/*********************************************************************/
 uint8_t highnib=value&0xf0;
 uint8_t lownib=(value<<4)&0xf0;
 LCD_I2C_write4bits((highnib)|mode);
 LCD_I2C_write4bits((lownib)|mode); 
}

/*********************************************************************/
void LCD_I2C_write4bits(uint8_t value) {
/*********************************************************************/
 LCD_I2C_expanderWrite(value);
 LCD_I2C_pulseEnable(value);
}

/*********************************************************************/
void LCD_I2C_expanderWrite(uint8_t _data){                                        
/*********************************************************************/
 Wire.beginTransmission(LCD_I2C_ADDRESS);
 Wire.write((int)(_data) | _backlightval);
 Wire.endTransmission(false);
 yield();   
}

/*********************************************************************/
void LCD_I2C_pulseEnable(uint8_t _data){
/*********************************************************************/
 LCD_I2C_expanderWrite(_data | En);	// En high
 delayMicroseconds(1);	 // enable pulse must be >450ns

 LCD_I2C_expanderWrite(_data & ~En);	// En low
 delayMicroseconds(50);	 // commands need > 37us to settle
} 
