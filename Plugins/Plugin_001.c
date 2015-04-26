//#######################################################################################################
//#################################### Plugin-01: Kaku   ################################################
//#######################################################################################################

/*********************************************************************************************\
 * Author             : Original source code, copyright by P.K.Tonkes, Modified for ESP8266
 * Support            : www.esp8266.nu
 * Date               : Apr 2015
 * Compatibility      : R005
\*********************************************************************************************/

#define KAKU_CodeLength             12
#define KAKU_T                     350
#define PLUGIN_001_EVENT        "Kaku"
#define PLUGIN_001_COMMAND  "KakuSend"


boolean Plugin_001(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  
  switch(function)
    {
    case PLUGIN_RAWSIGNAL_IN:
      {
      int i,j;
      unsigned long bitstream=0;

      if (RawSignal.Number!=(KAKU_CodeLength*4)+2)return false;                 // conventionele KAKU bestaat altijd uit 12 data bits plus stop. Ongelijk, dan geen KAKU!
    
      for (i=0; i<KAKU_CodeLength; i++)
        {
        j=(KAKU_T*2)/RawSignal.Multiply;   
        
        if      (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]<j && RawSignal.Pulses[4*i+4]>j) {bitstream=(bitstream >> 1);} // 0
        else if (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]>j && RawSignal.Pulses[4*i+4]<j) {bitstream=(bitstream >> 1 | (1 << (KAKU_CodeLength-1))); }// 1
        else if (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]<j && RawSignal.Pulses[4*i+4]<j) {bitstream=(bitstream >> 1); event->Par1=2;} // Short 0, Groep commando op 2e bit.
        else {return false;}                                                    // foutief signaal
        }
     
      if ((bitstream&0x600)==0x600)                                             // twee vaste bits van KAKU gebruiken als checksum
        {                                                                       // Alles is in orde, bouw event op           
        RawSignal.Repeats    = true;                                            // het is een herhalend signaal. Bij ontvangst herhalingen onderdrukken.
        event->Par2          = bitstream & 0xFF;
        event->Par1         |= (bitstream >> 11) & 0x01;
        event->SourceUnit    = 0;                                               // Komt niet van een Nodo unit af.
        event->Type          = ESP_TYPE_PLUGIN_EVENT;
        event->Command       = 1;                                               // nummer van deze plugin
        success=true;
        }
      break;
      }
      
    case PLUGIN_COMMAND:
      {
      RawSignal.Multiply=25;
      RawSignal.Repeats=7;                                                      // KAKU heeft minimaal vijf herhalingen nodig om te schakelen.
      RawSignal.Delay=20;                                                       // Tussen iedere pulsenreeks enige tijd rust.
      RawSignal.Number=KAKU_CodeLength*4+2;                                     // Lengte plus een stopbit
      event->Port=VALUE_ALL;                                                    // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.
     
      unsigned long Bitstream = event->Par2 | (0x600 | ((event->Par1&1 /*Commando*/) << 11)); // Stel een bitstream samen
      
      // loop de 12-bits langs en vertaal naar pulse/space signalen.  
      for (byte i=0; i<KAKU_CodeLength; i++)
        {
        RawSignal.Pulses[4*i+1]=KAKU_T/RawSignal.Multiply;
        RawSignal.Pulses[4*i+2]=(KAKU_T*3)/RawSignal.Multiply;
    
        if (((event->Par1>>1)&1) /* Groep */ && i>=4 && i<8) 
          {
          RawSignal.Pulses[4*i+3]=KAKU_T/RawSignal.Multiply;
          RawSignal.Pulses[4*i+4]=KAKU_T/RawSignal.Multiply;
          } // short 0
        else
          {
          if((Bitstream>>i)&1)// 1
            {
            RawSignal.Pulses[4*i+3]=(KAKU_T*3)/RawSignal.Multiply;
            RawSignal.Pulses[4*i+4]=KAKU_T/RawSignal.Multiply;
            }
          else //0
            {
            RawSignal.Pulses[4*i+3]=KAKU_T/RawSignal.Multiply;
            RawSignal.Pulses[4*i+4]=(KAKU_T*3)/RawSignal.Multiply;          
            }          
          }
        // Stopbit
        RawSignal.Pulses[4*KAKU_CodeLength+1]=KAKU_T/RawSignal.Multiply;
        RawSignal.Pulses[4*KAKU_CodeLength+2]=KAKU_T/RawSignal.Multiply;
        }

      SendEvent(event,true,true,Settings.WaitFree==VALUE_ON);
      success=true;
      break;
      }
    case PLUGIN_MMI_IN:
      {
      char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

      if(GetArgv(string,TempStr,1))
        {
        event->Type  = 0;
        
        if(strcasecmp(TempStr,PLUGIN_001_EVENT)==0)
          event->Type  = ESP_TYPE_PLUGIN_EVENT;

        if(strcasecmp(TempStr,PLUGIN_001_COMMAND)==0)
          event->Type  = ESP_TYPE_PLUGIN_COMMAND;

        if(event->Type)
          {
          byte c;
          byte x=0;                                                             // teller die wijst naar het het te behandelen teken
          byte Home=0;                                                          // KAKU home A..P
          byte Address=0;                                                       // KAKU Address 1..16
          event->Command = 1;                                                   // Plugin nummer  
        
          if(GetArgv(string,TempStr,2))                                         // eerste parameter bevat adres volgens codering A0..P16 
            {
            while((c=tolower(TempStr[x++]))!=0)
              {
              if(c>='0' && c<='9'){Address=Address*10;Address=Address+c-'0';}
              if(c>='a' && c<='p'){Home=c-'a';}                                 // KAKU home A is intern 0
              }
          
            if(Address==0)
              {                                                                 // groep commando is opgegeven: 0=alle adressen
              event->Par1=2;                                                    // 2e bit setten voor groep.
              event->Par2=Home;
              }
            else
              event->Par2= Home | ((Address-1)<<4);        
   
            if(GetArgv(string,TempStr,3))                                       // Het door de gebruiker ingegeven tweede parameter bevat het on/off commando
              {
              event->Par1 |= str2cmd(TempStr)==VALUE_ON; 
              success=true;
              }
            }
          }
        }
      free(TempStr);
      break;
      }

    case PLUGIN_MMI_OUT:
      {
      if(event->Type==ESP_TYPE_PLUGIN_EVENT)
        //strcpy(string,PLUGIN_001_EVENT);                                        // Event
        sprintf(string,"%s",PLUGIN_001_EVENT);
         
      else if(event->Type==ESP_TYPE_PLUGIN_COMMAND)
        //strcpy(string,PLUGIN_001_COMMAND);                                      // Command
        sprintf(string,"%s",PLUGIN_001_COMMAND);

      //strcat(string," ");                
      sprintf(string,"%s%s",string," ");

      char t[3];                                                                // Mini string
      t[0]='A' + (event->Par2 & 0x0f);                                          // Home A..P
      t[1]= 0;                                                                  // en de mini-string afsluiten.
      //strcat(string,t);
      sprintf(string,"%s%s",string,t);
    
      if(event->Par1&2)                                                         // als 2e bit in commando staat, dan groep.
        //strcat(string,int2str(0));                                              // Als Groep, dan adres 0       
        sprintf(string,"%s%s",string,int2str(0));
      else
        //strcat(string,int2str(((event->Par2 & 0xf0)>>4)+1));                    // Anders adres toevoegen             
        sprintf(string,"%s%s",string,int2str(((event->Par2 & 0xf0)>>4)+1));
    
      if(event->Par1&1)                                                         // Het 1e bit is get aan/uit commando
        //strcat(string,",On");
        sprintf(string,"%s%s",string,",On");
      else
        //strcat(string,",Off");  
        sprintf(string,"%s%s",string,",Off");

      break;
      }
    }      
  return success;
  }
