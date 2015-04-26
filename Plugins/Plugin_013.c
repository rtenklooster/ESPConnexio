//#######################################################################################################
//##################################### Plugin-13 Flamingo FA20RF Rookmelder ############################
//#######################################################################################################

/*********************************************************************************************\
 * Support            : www.esp8266.nu
 * Datum              : Apr 2015
 * Compatibility      : R006
 * Syntax             : "SmokeAlert 0, <Par2: rookmelder ID>"
 *********************************************************************************************
 * Technical description:
 * The Flamingo FA20RF smoke detector contains a RF receiver and transmitter.
 * By default, each unit comes with u uniqeu ID
 * By pressing the learn button, the unit will copy the ID from another unit
 * This is how they can be joined into one ID for all units
 * One unit will be the master, all others will learn this ID
 \*********************************************************************************************/
#define PLUGIN_ID_013    13
#define PLUGIN_NAME_013      "SmokeAlert"

boolean Plugin_013(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number != 52) return false;

      unsigned long bitstream=0L;
      for(byte x=4;x<=50;x=x+2)
      {
        if (RawSignal.Pulses[x-1]*RawSignal.Multiply > 1000) return false; // every preceding puls must be < 1000!
        if (RawSignal.Pulses[x]*RawSignal.Multiply > 1800) bitstream = (bitstream << 1) | 0x1; 
        else bitstream = bitstream << 1;
      }

      if (bitstream == 0) return false;

      event->Par1=0;
      event->Par2=bitstream;
      event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af, dus unit op nul zetten
      event->Port          = VALUE_SOURCE_RF;
      event->Type          = ESP_TYPE_PLUGIN_EVENT;
      event->Command       = PLUGIN_ID_013; // Nummer van dit device

      return true;
      break;
    }
  case PLUGIN_COMMAND:
    {
      break;
    }

  case PLUGIN_MMI_IN:
    {
    char* str=(char*)malloc(INPUT_COMMAND_SIZE);
  
    if(GetArgv(string,str,1))
      {
      if(strcasecmp(str,PLUGIN_NAME_013)==0)
        {
        if(GetArgv(string,str,2))
          {
          event->Par1=str2int(str);    
          if(GetArgv(string,str,3))
            {
              event->Par2=str2int(str);
              event->Type = ESP_TYPE_PLUGIN_EVENT;
              event->Command = PLUGIN_ID_013; 
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
    //strcpy(string,PLUGIN_NAME);            // Eerste argument=het commando deel
    //strcat(string," ");
    //strcat(string,int2str(event->Par1));
    //strcat(string,",");
    //strcat(string,int2str(event->Par2));
    sprintf(string,"%s",PLUGIN_NAME_013);
    sprintf(string,"%s%s",string," ");
    sprintf(string,"%s%s",string,int2str(event->Par1));
    sprintf(string,"%s%s",string,",");
    sprintf(string,"%s%s",string,int2str(event->Par2));
    break;
    }
  }      
  return success;
}
