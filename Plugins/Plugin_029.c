//#######################################################################################################
//#################################### Plugin-029: ExtWiredAnalog #######################################
//#######################################################################################################

/*********************************************************************************************\
 * This plugin provides support for 4 extra analog inputs, using the PCF8591 (NXP/Philips)
 * Support            : www.nodo-domotica.nl
 * Date               : Apr 2015
 * Compatibility      : R004
 * Syntax             : "ExtWiredAnalog <Par1:Port>, <Par2:Variable>"
 *********************************************************************************************
 * Technical description:
 *
 * De PCF8591 is a IO Expander chip that connects through the I2C bus
 * Basic I2C address = 0x48
 * Each chip has 4 analog inputs
 * This commando reads the analog input en stores the result into a variable
 \*********************************************************************************************/
#define PLUGIN_ID_029        29
#define PLUGIN_NAME_029      "ExtWiredAnalog"

boolean Plugin_029(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;

  static byte portValue=0;

  switch(function)
    {
    case PLUGIN_COMMAND:
      {
      byte unit = (event->Par1-1) / 4;
      byte port = event->Par1 - (unit * 4);
      uint8_t address = 0x48 + unit;

      // get the current pin value
      Wire.beginTransmission(address);
      Wire.write(port-1);
      Wire.endTransmission();

      Wire.requestFrom(address, (uint8_t)0x2);
      if(Wire.available())
      {
        Wire.read(); // Read older value first (stored in chip)
        UserVar[event->Par2-1] = (float)Wire.read();// now read actual value and store into Nodo var
        success=true;
      }
      break;
      }
    
    case PLUGIN_MMI_IN:
      {
      char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

      if(GetArgv(string,TempStr,1))
        {
        if(strcasecmp(TempStr,PLUGIN_NAME_029)==0)
          {
          if(GetArgv(string,TempStr,2)) 
            {
            if(GetArgv(string,TempStr,3))
              {
              if(event->Par1>0 && event->Par1<33 && event->Par2>=0 && event->Par2<=USER_VARIABLES_MAX)            
                {
                  event->Type = ESP_TYPE_PLUGIN_COMMAND;
                  event->Command = PLUGIN_ID_029;
                  success=true;
                }
              }
            }
          }
        }
      free(TempStr);
      break;
      }

    case PLUGIN_MMI_OUT:
      {
      //strcpy(string,PLUGIN_NAME_029);
      //strcat(string," ");
      //strcat(string,int2str(event->Par1));
      //strcat(string,",");
      //strcat(string,int2str(event->Par2));
      sprintf(string,"%s",PLUGIN_NAME_029);
      sprintf(string,"%s%s",string," ");
      sprintf(string,"%s%s",string,int2str(event->Par1));
      sprintf(string,"%s%s",string,",");
      sprintf(string,"%s%s",string,int2str(event->Par2));
      break;
      }
    }
    
  return success;
  }
