//#######################################################################################################
//#################################### Plugin-094: MCPWiredOut ##########################################
//#######################################################################################################

/*********************************************************************************************\
 * This protocol provides support for the MCP23017, 16 bit I2C IO-Expander
 * Support            : www.esp8266.nu
 * Date               : Apr 2015
 * Compatibility      : R004
 * Syntax             : "MCPWiredOut <Par1:Port>, <Par2:On/Off>"
 *********************************************************************************************
 * Technical data:
 *
 * The MCP23017 is an IO Expander device, connected through the I2C bus
 * Base I2C address = 0x20. This can be changed through address pins A0,A1,A2, value between 0x20 and 0x27.
 * Each device has 16 digital pins, used as output pins within this plugin
 * If multiple unit are connected, number <port> as following examples:
 *       port 1 is port 1 of the first device at address 0x20
 *       port 17 is port 1 of the second device at address 0x21
 *       port 33 is port 1 of the third device at address 0x22
 *       etc.
 \*********************************************************************************************/

#define PLUGIN_ID_094        94
#define PLUGIN_NAME_094      "MCPWiredOut"

boolean Plugin_094(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;

  switch(function)
    {    
    case PLUGIN_COMMAND:
      {
      byte portvalue=0;
      byte unit = (event->Par1-1) / 16;
      byte port = event->Par1 - (unit * 16);
      uint8_t address = 0x20 + unit;
      byte IOBankConfigReg=0;
      byte IOBankValueReg=0x12;

      if (port > 8)
        {
          port=port-8;
          IOBankConfigReg++;
          IOBankValueReg++;
        }
      // turn this port into output, first read current config
      Wire.beginTransmission(address);
      Wire.write(IOBankConfigReg); // IO config register
      Wire.endTransmission();
      Wire.requestFrom(address, (uint8_t)0x1);
        if(Wire.available())
        {
          portvalue = Wire.read();
          portvalue &= ~(1 << (port-1)); // change pin from (default) input to output

          // write new IO config
          Wire.beginTransmission(address);
          Wire.write(IOBankConfigReg); // IO config register
          Wire.write(portvalue);
          Wire.endTransmission();
        }

      // get the current pin status
      Wire.beginTransmission(address);
      Wire.write(IOBankValueReg); // IO data register
      Wire.endTransmission();
      Wire.requestFrom(address, (uint8_t)0x1);
      if(Wire.available())
      {
        portvalue = Wire.read();
        if (event->Par2==VALUE_ON)
          portvalue |= (1 << (port-1));
        else
          portvalue &= ~(1 << (port-1));
        
        // write back new data
        Wire.beginTransmission(address);
        Wire.write(IOBankValueReg); // IO data register
        Wire.write(portvalue);
        Wire.endTransmission();
        success=true;
      }
      break;
      }
    
    case PLUGIN_MMI_IN:
      {
      char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

      if(GetArgv(string,TempStr,1))
        {
        if(strcasecmp(TempStr,PLUGIN_NAME_094)==0)
          {
          if(GetArgv(string,TempStr,2)) 
            {
            if(GetArgv(string,TempStr,3))
              {
              if(event->Par1>0 && event->Par1<129 && (event->Par2==VALUE_ON || event->Par2==VALUE_OFF))            
                {
                  event->Type = ESP_TYPE_PLUGIN_COMMAND;
                  event->Command = PLUGIN_ID_094;
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
      //strcpy(string,PLUGIN_NAME_094);
      //strcat(string," ");
      //strcat(string,int2str(event->Par1));
      //strcat(string,",");
      //if(event->Par2==VALUE_ON)
      //  strcat(string,"On");  
      //else strcat(string,"Off");

      sprintf(string,"%s",PLUGIN_NAME_094);
      sprintf(string,"%s%s",string," ");
      sprintf(string,"%s%s",string,int2str(event->Par1));
      sprintf(string,"%s%s",string,",");
      if(event->Par2==VALUE_ON)
        //strcat(string,"On");  
        sprintf(string,"%s%s",string,"On");
      else
        //strcat(string,"Off");
        sprintf(string,"%s%s",string,"Off");
      break;
      }
    }
    
  return success;
  }
