//#######################################################################################################
//######################## PLUGIN-100 test                                     ##########################
//#######################################################################################################

/*********************************************************************************************\
 * Support            : www.esp8266.nu
 * Date               : Apr 2015
 * Compatibility      : R003
 * Syntax             : "? <Par1:?>, <Par2:?"
 *********************************************************************************************
 * Configuration:
 * This plugin can be used for testing
 * 
 * Technical description
 \*********************************************************************************************/

#define PLUGIN_ID_100        100
#define PLUGIN_NAME_100      "Test123"

boolean Plugin_100(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  
  switch(function)
    {
    case PLUGIN_INIT:
      {
        Serial.println("P100 init");
        break;
      }

    case PLUGIN_COMMAND:
      {
        Serial.println("P100 cmd");
        success=true;
        break;
      }
    
    case PLUGIN_MMI_IN:
      {
      char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);
      if(GetArgv(string,TempStr,1))
        {
        if(strcasecmp(TempStr,PLUGIN_NAME_100)==0)
          {
            event->Type = ESP_TYPE_PLUGIN_COMMAND;
            event->Command = PLUGIN_ID_100;
            success=true;
          }
        }
      free(TempStr);
      break;
      }
  
    case PLUGIN_MMI_OUT:
      {
      sprintf(string,"%s",PLUGIN_NAME_100);
      sprintf(string,"%s%s",string," ");
      sprintf(string,"%s%s",string,int2str(event->Par1));
      sprintf(string,"%s%s",string,",");
      sprintf(string,"%s%s",string,int2str(event->Par2));
      break;
      }
    }      
  return success;
  }
