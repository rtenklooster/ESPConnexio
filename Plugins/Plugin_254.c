//#######################################################################################################
//#################################### Plugin-254: LuxRead   ############################################
//#######################################################################################################

/*********************************************************************************************\
* Auteur             : Richard ten Klooster 
* Support            : http://weblog.tenklooster.net
* Datum              : 03-2015
* Versie             : 0.1
* Nodo productnummer : 
* Compatibiliteit    : Vanaf Nodo build nummer 744
* Syntax             : "LuxRead <Par1>"
\*********************************************************************************************/


#define PLUGIN_ID_254     254
#define PLUGIN_NAME_254   "luxRead"
#define BH1750_ADDRESS    0x23

boolean Plugin_254(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;

  switch(function)
  {
  case PLUGIN_INIT:
    {
      Serial.println("Lux init");
      Wire.beginTransmission(BH1750_ADDRESS);
      Wire.write(0x10);                             // 1 lx resolution
      Wire.endTransmission();
      break;
    }

  case PLUGIN_COMMAND:
    {
      Serial.println("Lux read");
      Wire.requestFrom(BH1750_ADDRESS, 2);
      byte b1 = Wire.read();
      byte b2 = Wire.read();
      unsigned int val=0;
      val=((b1<<8)|b2)/1.2;
      val=val+15;
      UserVar[event->Par1 -1] = (float)val;
      success=true;
      break;
    }

  case PLUGIN_MMI_IN:
    {
    char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

    if(GetArgv(string,TempStr,1))
      {
      if(strcasecmp(TempStr,PLUGIN_NAME_254)==0)
        {
        if(event->Par1 >0 && event->Par1<=USER_VARIABLES_MAX-1)
          {
          event->Command = PLUGIN_ID_254; // Plugin nummer  
          event->Type = ESP_TYPE_PLUGIN_COMMAND;
          success=true;
          }
        }
      }
    free(TempStr);
    break;
    }

  case PLUGIN_MMI_OUT:
    {
    sprintf(string,"%s",PLUGIN_NAME_254);
    sprintf(string,"%s%s",string," ");
    sprintf(string,"%s%s",string,int2str(event->Par1));
    break;
    }
  }      
  return success;
}
