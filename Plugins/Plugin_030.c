//#######################################################################################################
//################################# Plugin-30: Wiegand RFID Tag Reader ##################################
//#######################################################################################################

/*********************************************************************************************\
 * Support            : www.esp8266.nu
 * Date               : Apr 2015
 * Compatibility      : R003
 * Syntax             : RFIDWG <reader unit>,<tag ID>
 ***********************************************************************************************
 * Technical description:
 *
 * A Wiegand-26 Tag Reader reads 125KHz EM4001 compatible tags. Datastream conforms to Wiegand 26 protocol
 * This protocol uses two datalines. Pulses are very short (50uSec), so interrupt driven IO is needed.
 * Each EM4001 tag has a unique, presenting a  26 bits serial number
 * Par1 shows reader number (1 of 2)
 * Par2 shows the 26 bits serial number
 * This device generates an event than can trigger various actions
 \*********************************************************************************************/

#define PLUGIN_ID_030       30
#define PLUGIN_NAME_030     "RFIDWG"

#define PLUGIN_030_WGSIZE 26

void Plugin_030_interrupt1();
void Plugin_030_interrupt2();

volatile byte Plugin_030_bitCount = 0;	           // Count the number of bits received.
volatile unsigned long Plugin_030_keyBuffer = 0;   // A 32-bit-long keyBuffer into which the number is stored.
byte Plugin_030_bitCountPrev = 0;                  // to detect noise
byte Plugin_030_Unit=0;

boolean Plugin_030(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;

  switch(function)
    {    
    case PLUGIN_INIT:
      {
        // Init IO pins
        pinMode(0,INPUT);
        pinMode(2,INPUT);
        attachInterrupt(0,Plugin_030_interrupt1,FALLING); 
        attachInterrupt(2,Plugin_030_interrupt2,FALLING); 
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        if ((Plugin_030_bitCount != PLUGIN_030_WGSIZE) && (Plugin_030_bitCount == Plugin_030_bitCountPrev))
          {
            // must be noise
            Plugin_030_bitCount = 0;
            Plugin_030_keyBuffer = 0;
          }

        if (Plugin_030_bitCount == PLUGIN_030_WGSIZE)
          {
            Plugin_030_bitCount = 0;          // Read in the current key and reset everything so that the interrupts can

            Plugin_030_keyBuffer = Plugin_030_keyBuffer >> 1;          // Strip leading and trailing parity bits from the keyBuffer
            Plugin_030_keyBuffer &= 0xFFFFFF;

            struct NodoEventStruct TempEvent;
            ClearEvent(&TempEvent);
            TempEvent.SourceUnit = Settings.Unit;
            TempEvent.Direction  = VALUE_DIRECTION_INPUT;
            TempEvent.Command    = PLUGIN_ID_030;
            TempEvent.Port       = VALUE_SOURCE_WIRED;
            TempEvent.Par1       = Plugin_030_Unit;
            TempEvent.Par2       = Plugin_030_keyBuffer;
            TempEvent.Type       = ESP_TYPE_PLUGIN_EVENT;
            ProcessEvent(&TempEvent);
            Plugin_030_keyBuffer = 0;          // Clear the buffer for the next iteration.
          }

      Plugin_030_bitCountPrev = Plugin_030_bitCount; // store this value for next check, detect noise
      break;
      }
    
    case PLUGIN_MMI_IN:
    {
    char *str=(char*)malloc(INPUT_COMMAND_SIZE);
  
    if(GetArgv(string,str,1))
      {
      if(strcasecmp(str,PLUGIN_NAME_030)==0)
        {
        if(GetArgv(string,str,2))
          {
          event->Par1=str2int(str);    
          if(GetArgv(string,str,3))
            {
              event->Par2=str2int(str);
              event->Type = ESP_TYPE_PLUGIN_EVENT;
              event->Command = PLUGIN_ID_030;   
              success=true;
            }
          }
        }
      }
    free(str);
    break;
    }
     
    case PLUGIN_MMI_OUT:
      {
        //strcpy(string,PLUGIN_NAME);               // Commando 
        //strcat(string," ");
        //strcat(string,int2str(event->Par1));      // Parameter-1 (8-bit)
        //strcat(string,",");
        //strcat(string,int2strhex(event->Par2));
        sprintf(string,"%s",PLUGIN_NAME_030);
        sprintf(string,"%s%s",string," ");
        sprintf(string,"%s%s",string,int2str(event->Par1));
        sprintf(string,"%s%s",string,",");
        sprintf(string,"%s%s",string,int2strhex(event->Par2));
        break;
      }
    }
    
  return success;
  }

/*********************************************************************/
void Plugin_030_interrupt1()
/*********************************************************************/
{
  // We've received a 1 bit. (bit 0 = high, bit 1 = low)
  Plugin_030_keyBuffer = Plugin_030_keyBuffer << 1;     // Left shift the number (effectively multiplying by 2)
  Plugin_030_keyBuffer += 1;    		 // Add the 1 (not necessary for the zeroes)
  Plugin_030_bitCount++;   			 // Increment the bit count
}

/*********************************************************************/
void Plugin_030_interrupt2()
/*********************************************************************/
{
  // We've received a 0 bit. (bit 0 = low, bit 1 = high)
  Plugin_030_keyBuffer = Plugin_030_keyBuffer << 1;     // Left shift the number (effectively multiplying by 2)
  Plugin_030_bitCount++;       		 // Increment the bit count
}
