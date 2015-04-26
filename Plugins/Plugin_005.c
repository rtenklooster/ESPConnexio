//#######################################################################################################
//#################################### Plugin-05: TempSensor Dallas DS18B20  ############################
//#######################################################################################################

/*********************************************************************************************\
 * Author(s)          : Original created by Paul Tonkes, Modified for ESP-Connexio
 * Support            : www.esp8266.nu
 * Date               : Apr 2015
 * Compatibility      : R002
 * Syntax             : "DallasRead <Par1:Poortnummer>, <Par2:Variabele>"
 ***********************************************************************************************
 * Technical description:
 * Data pin should be connected to WiredOut and a resistor between datapin and VCC
 * Using ROM-skip feature, so no addressing of specific dallas devices
 * Limited to one device per WiredOut pin
 \*********************************************************************************************/

#define PLUGIN_ID_005        5
#define PLUGIN_NAME_005      "DallasRead"

uint8_t DallasPin;

void DS_write(uint8_t ByteToWrite); 
uint8_t DS_read(void);
uint8_t DS_reset();

boolean Plugin_005(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  static byte Call_Status = 0x00; // Each bit represents one relative port. 0=not called before, 1=already called before. 
  
  switch(function)
    {
      
    case PLUGIN_COMMAND:
      {
      int DSTemp;                           // Temperature in 16-bit Dallas format.
      byte ScratchPad[12];                  // Scratchpad buffer Dallas sensor.   
      byte var=event->Par2;                 // Variable to be set.
      byte RelativePort=event->Par1-1;
      
      DallasPin=PIN_WIRED_OUT_1+event->Par1-1;
      
      ClearEvent(event);
  
      noInterrupts();
      while (!(bitRead(Call_Status, RelativePort)))
        {
        // if this is the very first call to the sensor on this port, reset it to wake it up 
        boolean present=DS_reset();
        bitSet(Call_Status, RelativePort);
        }        
      boolean present=DS_reset();DS_write(0xCC /* rom skip */); DS_write(0x44 /* start conversion */);
      interrupts();
              
      if(present)
        {
        delay(800);     // neccesary delay
    
        noInterrupts();
        DS_reset(); DS_write(0xCC /* rom skip */); DS_write(0xBE /* Read Scratchpad */);
    
        digitalWrite(DallasPin,LOW);
        pinMode(DallasPin,INPUT);
    
        for (byte i = 0; i < 9; i++)            // copy 8 bytes
          ScratchPad[i] = DS_read();
        interrupts();
      
        DSTemp = (ScratchPad[1] << 8) + ScratchPad[0];  
    
        event->Type         = ESP_TYPE_COMMAND;
        event->Port         = VALUE_SOURCE_PLUGIN;
        event->Command      = CMD_VARIABLE_SET;
        event->Par1         = var;
        event->Par2         = float2ul(float(DSTemp)*0.0625);
        success=true;
        }
      break;
      }
      
    case PLUGIN_MMI_IN:
      {
      char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

      if(GetArgv(string,TempStr,1))
        {
        if(strcasecmp(TempStr,PLUGIN_NAME_005)==0)
          {
          if(event->Par1>0 && event->Par1<=WIRED_PORTS && event->Par2>0 && event->Par2<=USER_VARIABLES_MAX)            
            {
            event->Type = ESP_TYPE_PLUGIN_COMMAND;
            event->Command = PLUGIN_ID_005;
            success=true;
            }
          }
        }
      free(TempStr);
      break;
      }

    case PLUGIN_MMI_OUT:
      {
      //strcpy(string,PLUGIN_NAME_005);
      //strcat(string," ");
      //strcat(string,int2str(event->Par1));
      //strcat(string,",");
      //strcat(string,int2str(event->Par2));
      sprintf(string,"%s",PLUGIN_NAME_005);
      sprintf(string,"%s%s",string," ");
      sprintf(string,"%s%s",string,int2str(event->Par1));
      sprintf(string,"%s%s",string,",");
      sprintf(string,"%s%s",string,int2str(event->Par2));
      break;
      }
    }      
  return success;
  }

uint8_t DS_read(void)
  {
  uint8_t bitMask;
  uint8_t r = 0;
  uint8_t BitRead;
  
  for (bitMask = 0x01; bitMask; bitMask <<= 1)
    {
    pinMode(DallasPin,OUTPUT);
    digitalWrite(DallasPin,LOW);
    delayMicroseconds(3);

    pinMode(DallasPin,INPUT);// let pin float, pull up will raise
    delayMicroseconds(10);
    BitRead = digitalRead(DallasPin);
    delayMicroseconds(53);

    if (BitRead)
      r |= bitMask;
    }
  return r;
  }

void DS_write(uint8_t ByteToWrite) 
  {
  uint8_t bitMask;

  pinMode(DallasPin,OUTPUT);
  for (bitMask = 0x01; bitMask; bitMask <<= 1)
    {// BitWrite
    digitalWrite(DallasPin,LOW);
    if(((bitMask & ByteToWrite)?1:0) & 1)
      {
      delayMicroseconds(5);// Dallas spec.= 5..15 uSec.
      digitalWrite(DallasPin,HIGH);
      delayMicroseconds(55);// Dallas spec.= 60uSec.
      }
    else
      {
      delayMicroseconds(55);// Dallas spec.= 60uSec.
      digitalWrite(DallasPin,HIGH);
      delayMicroseconds(5);// Dallas spec.= 5..15 uSec.
      }
    }
  }

uint8_t DS_reset()
  {
  uint8_t r;
  uint8_t retries = 125;
  
  pinMode(DallasPin,INPUT);
  do  {  // wait until the wire is high... just in case
      if (--retries == 0) return 0;
      delayMicroseconds(2);
      } while ( !digitalRead(DallasPin));

  pinMode(DallasPin,OUTPUT); digitalWrite(DallasPin,LOW);
  delayMicroseconds(492); // Dallas spec. = Min. 480uSec. Arduino 500uSec.
  pinMode(DallasPin,INPUT);//Float
  delayMicroseconds(40);
  r = !digitalRead(DallasPin);
  delayMicroseconds(420);
  return r;
  }
