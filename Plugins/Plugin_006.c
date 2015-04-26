//#######################################################################################################
//######################## PLUGIN-06 Temperature and Humidity sensor DHT 11/22 ##########################
//#######################################################################################################

/*********************************************************************************************\
 * Support            : www.esp8266.nu
 * Date               : Apr 2015
 * Compatibility      : R003
 * Syntax             : "DHTRead <Par1:Poortnummer>, <Par2:Basis Variabele>"
 *********************************************************************************************
 * Configuration:
 * This plugin can be used for DHT-11 sensor and DHT-22 that has a higher resolution
 * 
 * Technical description
 * The DHT sensor is a 3 pin sensor with a bidirectional data pin. This pin should be connected to a WiredOut pin. 
 * THe protocol uses 1 wire, but it is not compatible with the well known Dallas onewire protocol.
 * This plugin uses two variables, 1 for temperature and 1 for humidity
 \*********************************************************************************************/

#define PLUGIN_ID_006        6
#define PLUGIN_NAME_006      "DHTRead"

uint8_t DHT_Pin;

/*********************************************************************************************\
 * DHT sub to get an 8 bit value from the receiving bitstream
 \*********************************************************************************************/
byte read_dht_dat(void)
  {
  byte i = 0;
  byte result=0;
  noInterrupts();
  for(i=0; i< 8; i++)
    {
    while(!digitalRead(DHT_Pin));  // wait for 50us
    delayMicroseconds(30);
    if(digitalRead(DHT_Pin)) 
      result |=(1<<(7-i));
    while(digitalRead(DHT_Pin));  // wait '1' finish
    }
  interrupts();
  return result;
  }

boolean Plugin_006(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  
  switch(function)
    {
    case PLUGIN_COMMAND:
      {
      DHT_Pin=PIN_WIRED_OUT_1+event->Par1-1;
      byte dht_dat[5];
      byte dht_in;
      byte i;
      byte Retry=0;

      do
        {  
        pinMode(DHT_Pin,OUTPUT);
        // DHT start condition, pull-down i/o pin for 18ms
        digitalWrite(DHT_Pin,LOW);               // Pull low
        delay(18);
        digitalWrite(DHT_Pin,HIGH);              // Pull high
        delayMicroseconds(40);
        pinMode(DHT_Pin,INPUT);                  // change pin to input
        delayMicroseconds(40);
    
        dht_in = digitalRead(DHT_Pin);
        if(!dht_in)
          {
          delayMicroseconds(80);
          dht_in = digitalRead(DHT_Pin);
          if(dht_in)
            {
            delayMicroseconds(40);                     // now ready for data reception
            for (i=0; i<5; i++)
              dht_dat[i] = read_dht_dat();
              
            // Checksum calculation is a Rollover Checksum by design!
            byte dht_check_sum = dht_dat[0]+dht_dat[1]+dht_dat[2]+dht_dat[3];// check check_sum

            if(dht_dat[4]== dht_check_sum)
              {

              #if PLUGIN_006==11
                UserVar[event->Par2 -1] = float(dht_dat[2]); // Temperature
                UserVar[event->Par2   ] = float(dht_dat[0]); // Humidity
              #endif
              
              #if PLUGIN_006==22
                if (dht_dat[2] & 0x80) // negative temperature
                  UserVar[event->Par2 -1] = -0.1 * word(dht_dat[2] & 0x7F, dht_dat[3]);
                else
                  UserVar[event->Par2 -1] = 0.1 * word(dht_dat[2], dht_dat[3]);
                UserVar[event->Par2] = word(dht_dat[0], dht_dat[1]) * 0.1; // Humidity
              #endif

              success=true;
              }
            }
          }
        if(!success)
          {
          delay(2000);
          }
        }while(!success && ++Retry<3);        
      break;
      }
    
    case PLUGIN_MMI_IN:
      {
      char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);
      if(GetArgv(string,TempStr,1))
        {
        if(strcasecmp(TempStr,PLUGIN_NAME_006)==0)
          {
          if(event->Par1>0 && event->Par1<=WIRED_PORTS && event->Par2>0 && event->Par2<=USER_VARIABLES_MAX)            
            {
            event->Type = ESP_TYPE_PLUGIN_COMMAND;
            event->Command = PLUGIN_ID_006;
            success=true;
            }
          }
        }
      free(TempStr);
      break;
      }
  
    case PLUGIN_MMI_OUT:
      {
      //strcpy(string,PLUGIN_NAME);
      //strcat(string," ");
      //strcat(string,int2str(event->Par1));
      //strcat(string,",");
      //strcat(string,int2str(event->Par2));
      sprintf(string,"%s",PLUGIN_NAME_006);
      sprintf(string,"%s%s",string," ");
      sprintf(string,"%s%s",string,int2str(event->Par1));
      sprintf(string,"%s%s",string,",");
      sprintf(string,"%s%s",string,int2str(event->Par2));
      break;
      }
    }      
  return success;
  }
