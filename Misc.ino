/**********************************************************************************************\
 * Schrijft een event in de Eventlist. Deze Eventlist bevindt zich in het EEPROM geheugen.
 \*********************************************************************************************/
boolean Eventlist_Write(int Line, struct NodoEventStruct *Event, struct NodoEventStruct *Action)// LET OP: Gebruikers input. Eerste adres=1
  {
  struct EventlistStruct EEPROM_Block;
  struct NodoEventStruct dummy;
  int x,address;

  // als opgegeven adres=0, zoek dan de eerste vrije plaats.
  if(Line==0)
    {
    Line++;
    while(Eventlist_Read(Line,&dummy,&dummy) && dummy.Command!=0)Line++;
    }
  Line--;                                                                          // echte adressering begint vanaf nul. voor de user vanaf 1.  

  if(Line>EventlistMax)
    return false;

  address=Line * sizeof(struct EventlistStruct) + sizeof(struct SettingsStruct);     // Eerste deel van het EEPROM geheugen is voor de settings. Reserveer deze bytes. Deze niet te gebruiken voor de Eventlist!
  
  byte *B=(byte*)&EEPROM_Block;                                                       // B wijst naar de eerste byte van de struct

  // Nu wegschrijven.
  address=Line * sizeof(struct EventlistStruct) + sizeof(struct SettingsStruct);     // Eerste deel van het EEPROM geheugen is voor de settings. Reserveer deze bytes. Deze niet te gebruiken voor de Eventlist!
  EEPROM_Block.EventType=Event->Type;
  EEPROM_Block.EventCommand=Event->Command;
  EEPROM_Block.EventPar1=Event->Par1;
  EEPROM_Block.EventPar2=Event->Par2;
 
  EEPROM_Block.ActionType=Action->Type;
  EEPROM_Block.ActionCommand=Action->Command;
  EEPROM_Block.ActionPar1=Action->Par1;
  EEPROM_Block.ActionPar2=Action->Par2;

  for(x=0;x<sizeof(struct EventlistStruct);x++) // schrijf alle bytes van de struct
    {
    if(address<EEPROM_SIZE)
      EEPROM.write(address++, *(B+x));
    else
      return false;
    }
    EEPROM.commit();
  return true;
  }

/**********************************************************************************************\
 * 
 * Revision 01, 09-12-2009, P.K.Tonkes@gmail.com
 \*********************************************************************************************/
boolean Eventlist_Read(int address, struct NodoEventStruct *Event, struct NodoEventStruct *Action)// LET OP: eerste adres=1
  {
  struct EventlistStruct EEPROM_Block;

  if(address>EventlistMax)
    return false;

  address--;// echte adressering begint vanaf nul. voor de user vanaf 1.
  address=address*sizeof(struct EventlistStruct) + sizeof(struct SettingsStruct);     // Eerste deel van het EEPROM geheugen is voor de settings. Reserveer deze bytes. Deze niet te gebruiken voor de Eventlist!
  byte *B=(byte*)&EEPROM_Block; // B wijst naar de eerste byte van de struct

  for(int x=0;x<sizeof(struct EventlistStruct);x++) // lees alle bytes van de struct
    {
    if(address<EEPROM_SIZE)
      *(B+x)=EEPROM.read(address++);
    else
      return false;
    }

  ClearEvent(Event);
  Event->Type=EEPROM_Block.EventType;
  Event->Command=EEPROM_Block.EventCommand;
  Event->Par1=EEPROM_Block.EventPar1;
  Event->Par2=EEPROM_Block.EventPar2;
  
  ClearEvent(Action);
  Action->Type=EEPROM_Block.ActionType;
  Action->Command=EEPROM_Block.ActionCommand;
  Action->Par1=EEPROM_Block.ActionPar1;
  Action->Par2=EEPROM_Block.ActionPar2;

  Action->Port=VALUE_SOURCE_EVENTLIST;
  Action->Direction=VALUE_DIRECTION_INPUT;
  return true;
  }

/*********************************************************************************************\
 * Deze funktie verstuurt een message. Aanroep van deze funktie in de code daar waar de foutmelding 
 * is opgetreden, zodat er geen foutcodes door de code heen getransporteerd hoeven te worden.
 * 
 \*********************************************************************************************/
void RaiseMessage(byte MessageCode, unsigned long Option)
  {
  if(MessageCode)
    {
    struct NodoEventStruct TempEvent;
    ClearEvent(&TempEvent);
    TempEvent.Type      = ESP_TYPE_EVENT;
    TempEvent.Command   = EVENT_MESSAGE;
    TempEvent.Par1      = MessageCode;
    TempEvent.Par2      = Option;
    TempEvent.Direction = VALUE_DIRECTION_INPUT;
    TempEvent.Port      = VALUE_SOURCE_SYSTEM;

    PrintEvent(&TempEvent, VALUE_ALL);
  
    switch(MessageCode)                                                         // sommige meldingen mogen niet worden verzonden als event
      {
      case MESSAGE_BREAK:                                                       // normale break
      case MESSAGE_VERSION_ERROR:                                               // gaat rondzingen van events tussen Nodo's opleveren.
      case MESSAGE_EXECUTION_STOPPED:                                           // Slechts een boodschap voor gebruiker. Onnodig om te versturen.
        return;
      }
  
    TempEvent.Port      = VALUE_ALL;
    SendEvent(&TempEvent,false,true,Settings.WaitFree==VALUE_ON);
    }
  }


/**********************************************************************************************\
 * Geef een geluidssignaal met toonhoogte van 'frequentie' in Herz en tijdsduur 'duration' in milliseconden.
 * LET OP: toonhoogte is slechts een grove indicatie. Deze routine is bedoeld als signaalfunctie
 * en is niet bruikbaar voor toepassingen waar de toonhoogte zuiver/exact moet zijn. Geen PWM.
 * Definieer de constante:
 * #define PIN_SPEAKER <LuidsprekerAansluiting>
 * Neem in setup() de volgende regel op:
 * pinMode(PIN_SPEAKER, OUTPUT);
 * Routine wordt verlaten na beeindiging van de pieptoon.
 * Revision 01, 13-02-2009, P.K.Tonkes@gmail.com
 \*********************************************************************************************/

void Beep(int frequency, int duration)//Herz,millisec 
  {
  long halfperiod=500000L/frequency;
  long loops=(long)duration*frequency/(long)1000;

  for(loops;loops>0;loops--) 
    {
    digitalWrite(PIN_SPEAKER, HIGH);
    delayMicroseconds(halfperiod);
    digitalWrite(PIN_SPEAKER, LOW);
    delayMicroseconds(halfperiod);
    }
  }

/**********************************************************************************************\
 * Geeft een belsignaal.
 * Revision 01, 09-03-2009, P.K.Tonkes@gmail.com
 \*********************************************************************************************/
void Alarm(int Variant,int Option)
  {
  byte x,y;

  switch (Variant)
  { 
  case 1:// four beeps
    for(y=1;y<=(Option>1?Option:1);y++)
      {
      Beep(3000,30);
      delay(100);
      Beep(3000,30);
      delay(100);
      Beep(3000,30);
      delay(100);
      Beep(3000,30);
      delay(1000);
      }    
    break;

  case 2: // whoop up
    for(y=1;y<=(Option>1?Option:1);y++)
      {
      for(x=1;x<=50;x++)
        Beep(250*x/4,20);
      }          
    break;

  case 3: // whoop down
    for(y=1;y<=(Option>1?Option:1);y++)
      {
      for(x=50;x>0;x--)
        Beep(250*x/4,20);
      }          
    break;

  case 4:// Settings.O.Settings.
    for(y=1;y<=(Option>1?Option:1);y++)
      {
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      delay(200);
      Beep(1200,300);
      delay(100);
      Beep(1200,300);
      delay(100);
      Beep(1200,300);
      delay(200);
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      if(Option>1)delay(500);
      }
    break;

  case 5:// ding-dong
    for(x=1;x<=(Option>1?Option:1);x++)
      {
      if(x>1)delay(2000);
      Beep(1500,500);
      Beep(1200,500);
      }    
    break;

  case 6: // phone ring
    for(x=1;x<(15*(Option>1?Option:1));x++)
      {
      Beep(1000,40);
      Beep(750,40);
      }
    break;

  case 7: // boot
    Beep(1500,100);
    Beep(1000,100);
    break;

  default:// beep
    if(Variant==0)
      Variant=5; // tijdsduur

    if(Option==0)
      Option=20; // toonhoogte

    Beep(100*Option,Variant*10);
    break;
    }
  }


/**********************************************************************************************\
 * Stuur de RGB-led.
 *
 * Voor de Nodo geldt:
 *
 * Groen = Nodo in rust en wacht op een event.
 * Rood = Nodo verwerkt event of commando.
 * Blauw = Bijzondere modus Nodo waarin Nodo niet in staat is om events te ontvangen of genereren.
 \*********************************************************************************************/
void Led(byte Color)
  { 
  digitalWrite(PIN_LED_RGB_R,(Color==RED || Color==BLUE));
  }


/*********************************************************************************************\
 * Wachtloop. Als <EventsInQueue>=true dan worden voorbijkomende events in de queue geplaatst
 *
 * wachtloop die wordt afgebroken als:
 * - <Timeout> seconden zijn voorbij. In dit geval geeft deze funktie een <false> terug. (Timeout verlengd tijd als geldige events binnenkomen!)
 * - Het opgegeven event <WaitForEvent> voorbij is gekomen
 * - De ether weer is vrijgegeven voor Nodo communicatie (WaitForFreeTransmission=true)
 * - Er is een event opgevangen waar de TRANSMISSION_QUEUE_NEXT vlag NIET staat.
 \*********************************************************************************************/
boolean Wait(int Timeout, boolean WaitForFreeTransmission, struct NodoEventStruct *WaitForEvent, boolean EndSequence)
  {
  unsigned long TimeoutTimer=millis() + (unsigned long)(Timeout)*1000;

  unsigned long MessageTimer=millis() + 3000;
  boolean WaitMessage=false;
  
  struct NodoEventStruct Event;
  ClearEvent(&Event);

  while(TimeoutTimer>millis())
    {
    if(!WaitMessage && MessageTimer<millis())                                   // Als wachten langer duurt dan 5sec. dan melding weergeven aan gebruiker.
      {
      WaitMessage=true;
      Serial.println(Text_07);
      }
      
    if(ScanEvent(&Event))
      {            
     
      TimeoutTimer=millis() + (unsigned long)(Timeout)*1000;                    // Zolang er event binnenkomen geen timeout.
      MessageTimer=millis() + 3000;
      QueueAdd(&Event);                                                         // Events die voorbij komen in de queue plaatsen.

      if(EndSequence && (Event.Flags & TRANSMISSION_QUEUE_NEXT)==0)
        {
        break;
        }
        
      // als het gewacht wordt totdat de communicatie poorten weer beschikbaar zijn, dan wachtloop verlaten.        
      if(WaitForFreeTransmission && BusyNodo==0)
        {
        break;
        }
      
      // break af als opgegeven event voorbij komt. Let op, alleen events met als bestemming 0 of dit unitnummer worden gedetecteerd!
      // De check vindt alleen plaats Type, Command en Unit, dus niet op Par1 en Par2.
      // Als SourceUnit==0 dan wordt input van alle units geaccepteerd.
      if(WaitForEvent!=0)
        {
        if(WaitForEvent->Command==Event.Command && WaitForEvent->Type==Event.Type)
          {
          if(WaitForEvent->SourceUnit==Event.SourceUnit || WaitForEvent->SourceUnit==0)
            {
            *WaitForEvent=Event;
            // Serial.println(F("DEBUG: Wait() WaitForEvent geslaagd."));
            break;
            }
          }
        }
      }
    Led(RED);
    }   
    
  // Serial.println(F("DEBUG: Wait() verlaten."));

  HoldTransmission=ESP_TX_TO_RX_SWITCH_TIME+millis();                          // Hier ingelast om snel afwisselend events uitwisselen tussen twee i2c Nodo's te laten werken.         

  if(TimeoutTimer<=millis())                                                    // als timeout, dan error terug geven
    {
    // Serial.println(F("DEBUG: Wait() Timeout."));
    return false;
    }

  else
    return true;
  }



/*********************************************************************************************\
 * Haal voor het opgegeven Command de status op door resultaat in de event struct te plaatsen.
 * Geef een false terug als commando niet bestaat.
 \*********************************************************************************************/
boolean GetStatus(struct NodoEventStruct *Event)
  { 
  int x;

  // bewaar de opvraag gegevens en begin met een schoon event
  byte xPar1=Event->Par1;
  byte xCommand=Event->Command;  
  ClearEvent(Event);

  Event->Command=xCommand;
  Event->Type=ESP_TYPE_EVENT;

  switch (xCommand)
    {
    case VALUE_EVENTLIST_COUNT:
      x=0;
      struct NodoEventStruct dummy;
      while(Eventlist_Read(++x,&dummy,&dummy))
        {
        if(dummy.Command)
          Event->Par1++;
        }
      Event->Par2=x-Event->Par1-1;
      break;

  case VALUE_BUILD:
    Event->Par2=ESP_BUILD;      
    break;        

//  case VALUE_HWCONFIG: 
//    Event->Par2=HW_Config;      
//    break;        

  case CMD_OUTPUT:
    Event->Par1=xPar1;
    switch(xPar1)
      {
      case VALUE_SOURCE_IR:
        Event->Par2=Settings.TransmitIR;
        break;
  
      case VALUE_SOURCE_RF:
        Event->Par2=Settings.TransmitRF;
        break;
  
      default:
        Event->Command=0;                                                       // Geen geldige optie. Als 0 wordt teruggegeven in command dan wordt niets weergegeven met de status.
      }
    break;

  case VALUE_SOURCE_PLUGIN:
    x=Plugin_id[xPar1];
    if(x)
      Event->Par1=x;
    else
        Event->Command=0;                                                       // Geen device op deze positie
    break;

  case CMD_VARIABLE_SET:
    Event->Par1=xPar1;
    Event->Par2=float2ul(UserVar[xPar1-1]);
    break;

  case CMD_TIMER_SET:
    Event->Par1=xPar1;
    if(UserTimer[xPar1-1])
      Event->Par2=(UserTimer[xPar1-1]-millis())/1000;
    else
      Event->Par2=0;
    break;

  #if WIRED
  case CMD_WIRED_PULLUP:
    Event->Par1=xPar1;
    Event->Par2=Settings.WiredInputPullUp[xPar1-1];
    break;

//  case VALUE_WIRED_ANALOG:
//    Event->Par1=xPar1;
//    Event->Par2=analogRead(PIN_WIRED_IN_1+xPar1-1);
//    break;

//  case CMD_WIRED_THRESHOLD:
//    Event->Par1=xPar1;
//    Event->Par2=Settings.WiredInputThreshold[xPar1-1];
//    break;

//  case CMD_WIRED_SMITTTRIGGER:
//    Event->Par1=xPar1;
//    Event->Par2=Settings.WiredInputSmittTrigger[xPar1-1];
//    break;
    
  case EVENT_WIRED_IN:
    Event->Par1=xPar1;
    Event->Par2=(WiredInputStatus[xPar1-1])?VALUE_ON:VALUE_OFF;
    break;

  case CMD_WIRED_OUT:
    Event->Par1=xPar1;
    Event->Par2=(WiredOutputStatus[xPar1-1])?VALUE_ON:VALUE_OFF;
    break;

  #endif //WIRED

  case VALUE_FREEMEM:    
    Event->Par2=FreeMem();
    break;

  case VALUE_UNIT:
    x=NodoOnline(xPar1,0);
    if(x!=0)
      {
      Event->Par1=xPar1;
      Event->Par2=x;
      }
    else
      Event->Command=0;                                                         // Als resultaat niet geldig is en niet weergegeven mag worden
    
    break;

  default:
    return false;
  }
  return true;
}



/*********************************************************************************************\
 * Sla alle settings op in het EEPROM geheugen.
 \*********************************************************************************************/
void Save_Settings(void)  
  {
  char ByteToSave,*pointerToByteToSave=pointerToByteToSave=(char*)&Settings;    //pointer verwijst nu naar startadres van de struct. 

  for(int x=0; x<sizeof(struct SettingsStruct) ;x++)
    {
    EEPROM.write(x,*pointerToByteToSave); 
    pointerToByteToSave++;
    }
  EEPROM.commit();
  }

/*********************************************************************************************\
 * Laad de settings uit het EEPROM geheugen.
 \*********************************************************************************************/
boolean LoadSettings()
 {
  byte x;

  char ByteToSave,*pointerToByteToRead=(char*)&Settings;    //pointer verwijst nu naar startadres van de struct.

  for(int x=0; x<sizeof(struct SettingsStruct);x++)
    {
    *pointerToByteToRead=EEPROM.read(x);
    pointerToByteToRead++;// volgende byte uit de struct
    }
  }


/*********************************************************************************************\
 * Alle settings van de Nodo weer op default.
 \*********************************************************************************************/
void ResetFactory(void)
  {
  int x,y;

  // maak de eventlist leeg.
  struct NodoEventStruct dummy;
  ClearEvent(&dummy);
  x=1;
  while(Eventlist_Write(x++,&dummy,&dummy)) yield();

  // Herstel alle settings naar defaults
  Settings.Version                    = ESP_VERSION_MINOR;
  Settings.NewNodo                    = true;
  Settings.Lock                       = 0;
  Settings.WaitFreeNodo               = VALUE_OFF;
  Settings.TransmitIR                 = VALUE_OFF;
  Settings.TransmitRF                 = VALUE_ON;
  Settings.Unit                       = UNIT_ESP;
  Settings.RawSignalReceive           = VALUE_OFF;
  Settings.RawSignalSample            = RAWSIGNAL_SAMPLE_DEFAULT;  
  Settings.WifiSSID[0]                = 0;
  Settings.WifiKey[0]                 = 0;
  Settings.Server_IP[0]               = 0;
  Settings.Server_IP[1]               = 0;
  Settings.Server_IP[2]               = 0;
  Settings.Server_IP[3]               = 0;
  Settings.ServerPort                 = 0;
  
  // zet analoge poort  waarden op default
  #if WIRED
  for(x=0;x<WIRED_PORTS;x++)
    {
    Settings.WiredInputThreshold[x]=512; 
    Settings.WiredInputSmittTrigger[x]=10;
    Settings.WiredInputPullUp[x]=VALUE_ON;
    }
  #endif //WIRED

  Save_Settings();
  Reboot();
  }


/**********************************************************************************************\
 * Geeft de status weer of genereert een event.
 * Par1 = Command
 \**********************************************************************************************/
void Status(struct NodoEventStruct *Request)
  {
  byte CMD_Start,CMD_End;
  byte Par1_Start,Par1_End;
  byte Port=0,x;
  boolean s;
  boolean DisplayLocal=false;
  struct NodoEventStruct Result;

  char *TempString=(char*)malloc(INPUT_LINE_SIZE);

  Port=Request->Port;
  DisplayLocal=true;

  // Als de status informatie verzonden moet worden, dan komt deze terecht in de queue van de master waar het status verzoek
  // vandaan kwam. Hier een korte wachttijd omdat anders i.v.m. de omschakeltijd van de Master van zenden naar ontvangen
  // de eerste waarden die door deze slave worden verzonden niet aankomen.
  
  Result.Command=Request->Par1;
  
  if(Request->Par2==VALUE_ALL)
    Request->Par2==0;

  if(DisplayLocal && (Request->Par1==0 || Request->Par1==VALUE_ALL))
    PrintWelcome();

  if(Request->Par1==VALUE_ALL)
    {
    Request->Par2=0;
    CMD_Start=0;
    CMD_End=COMMAND_MAX;
    }
  else
    {
    Result.Command=Request->Par1;
    if(!GetStatus(&Result))                                                     // kijk of voor de opgegeven parameter de status opvraagbaar is. Zo niet dan klaar.
      {
        free(TempString);
        return;
      }
    CMD_Start=Request->Par1;
    CMD_End=Request->Par1;
    }

  for(x=CMD_Start; x<=CMD_End; x++)
    {
    s=false;
    if(DisplayLocal)
      {
      s=true;
      switch (x)
        {
          case CMD_SERVER_IP:
          sprintf(TempString,"%s %u.%u.%u.%u",cmd2str(CMD_SERVER_IP),Settings.Server_IP[0],Settings.Server_IP[1],Settings.Server_IP[2],Settings.Server_IP[3]);
          Serial.println(TempString);
          break;

          case CMD_SERVER_PORT:
          sprintf(TempString,"%s %d",cmd2str(CMD_SERVER_PORT),Settings.ServerPort);
          Serial.println(TempString);
          break;

          case CMD_WIFI_SSID:
          sprintf(TempString,"%s %s",cmd2str(CMD_WIFI_SSID),Settings.WifiSSID);
          Serial.println(TempString);
          break;

          case CMD_WIFI_KEY:
          sprintf(TempString,"%s %s",cmd2str(CMD_WIFI_KEY),Settings.WifiKey);
          Serial.println(TempString);
          break;

        default:
          s=false; 
          break;
        }
      }

    Result.Command=x;
    Result.Par1=Request->Par1;    
    
    if(!s && GetStatus(&Result))                                                // Als het een geldige uitvraag is.
      {
      if(Request->Par2==0)                                                      // Als in het commando 'Status Par1, Par2' Par2 niet is gevuld met een waarde
        {
        switch(x)
          {
          case VALUE_SOURCE_PLUGIN:
            Par1_Start=0;
            Par1_End=PLUGIN_MAX-1;
            break;

          case CMD_OUTPUT:
            Par1_Start=0;
            Par1_End=COMMAND_MAX;
            break;
  
          case VALUE_WIRED_ANALOG:
          case CMD_WIRED_OUT:
          case CMD_WIRED_PULLUP:
          case CMD_WIRED_SMITTTRIGGER:
          case CMD_WIRED_THRESHOLD:
          case EVENT_WIRED_IN:
            Par1_Start=1;
            Par1_End=WIRED_PORTS;
            break;      
  
          case CMD_VARIABLE_SET:
            Par1_Start=1;
            Par1_End=USER_VARIABLES_MAX;
            break;
    
          case CMD_TIMER_SET:
            Par1_Start=1;
            Par1_End=TIMER_MAX;
            break;
  
          case VALUE_UNIT:
            Par1_Start=1;
            Par1_End=UNIT_MAX;
            break;

          default:
            Par1_Start=0;
            Par1_End=0;
          }
        }
      else
        {
        Par1_Start=Request->Par2;
        Par1_End=Request->Par2;
        }

      for(byte y=Par1_Start;y<=Par1_End;y++)
        {
        Result.Command=x;
        Result.Par1=y;
        GetStatus(&Result); 
        Result.Port=Port;
        
        if(Result.Command!=0)
          {
          if(!DisplayLocal)
            {
            Result.Flags=TRANSMISSION_VIEW | TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT | TRANSMISSION_BUSY;
            SendEvent(&Result,false,false,false);
            }            
  
          else
            {
            Event2str(&Result,TempString);
            Serial.println(TempString);
            }
          }
        }
      }
    }

  if(DisplayLocal && Request->Par1==VALUE_ALL)
    Serial.println(Text_22);

  free(TempString);
  }

/*********************************************************************************************\
 * Deze routine parsed string en geeft het opgegeven argument nummer Argc terug in Argv
 * argumenten worden van elkaar gescheiden door een komma of een spatie.
 * Let op dat de ruimte in de doelstring voldoende is EN dat de bron string netjes is afgesloten 
 * met een 0-byte. Bij succes keert de routine terug met als waarde de positie naar het eerste teken 
 * van het volgende argument.  
 \*********************************************************************************************/
byte GetArgv(char *string, char *argv, int argc)
{
  int string_pos=0,argv_pos=0,argc_pos=0; 
  char c,d;

  while(string_pos<strlen(string))
  {
    c=string[string_pos];
    d=string[string_pos+1];

    if       (c==' ' && d==' '){}
    else if  (c==' ' && d==','){}
    else if  (c==',' && d==' '){}
    else if  (c==' ' && d>=33 && d<=126){}
    else if  (c==',' && d>=33 && d<=126){}
    else 
      {
      if(c!=' ' && c!=',')
        {
        argv[argv_pos++]=c;
        argv[argv_pos]=0;
        }          

      if(d==' ' || d==',' || d==0)
        {
        // Bezig met toevoegen van tekens aan een argument, maar er kwam een scheidingsteken.
        argv[argv_pos]=0;
        argc_pos++;

        if(argc_pos==argc)
          return string_pos+1;
          
        argv[0]=0;
        argv_pos=0;
        string_pos++;
      }
    }
    string_pos++;
  }
  return 0;
}


/*********************************************************************************************\
 * Deze routine parsed string en zoekt naar keyword. Geeft de startpositie terug waar het keyword
 * gevonden is. -1 indien niet gevonden. Niet casesensitive.
 \*********************************************************************************************/
int StringFind(char *string, char *keyword)
{
  int x,y;
  int keyword_len=strlen(keyword);
  int string_len=strlen(string);

  if(keyword_len>string_len) // doe geen moeite als het te vinden eyword langer is dan de string.
    return -1;

  for(x=0; x<=(string_len-keyword_len); x++)
  {
    y=0;
    while(y<keyword_len && (tolower(string[x+y])==tolower(keyword[y])))
      y++;

    if(y==keyword_len)
      return x;
  }
  return -1;
}

extern "C" {
#include "user_interface.h"
}
unsigned long FreeMem(void)
  {
    return system_get_free_heap_size();
  }

void PulseCounterISR()
  {
  static unsigned long PulseTimePrevious=0L;

  // in deze interrupt service routine staat millis() stil. Dit is echter geen bezwaar voor de meting.
  PulseTime=millis()-PulseTimePrevious;
  if(PulseTime>=PULSE_DEBOUNCE_TIME)
    PulseCount++;
  else
    PulseTime=0;

  PulseTimePrevious=millis();
  }     


/**********************************************************************************************\
 * Indien het een vers geresette Nodo is, dan ongedaan maken van deze status.
 \*********************************************************************************************/
void UndoNewNodo(void)
  {
  if(Settings.NewNodo)
    {
    Settings.NewNodo=false;
    Save_Settings();
    } 
  }

/**********************************************************************************************\
 * Reset een vers geïnitialiseerde struct. Nodog om dat niet mag worden aangenomen dat alle
 * waarden Na initialisatie leeg zijn.
 \*********************************************************************************************/
void ClearEvent(struct NodoEventStruct *Event)
{    
  Event->Command            = 0;
  Event->Par1               = 0;
  Event->Par2               = 0L;
  Event->Flags              = 0;
  Event->Port               = 0;
  Event->Type               = 0;
  Event->Direction          = 0;
  Event->DestinationUnit    = 0;
  Event->SourceUnit         = Settings.Unit;
  Event->Version            = ESP_VERSION_MINOR;
  Event->Checksum           = 0;
}

//#######################################################################################################
//##################################### Misc: Conversions     ###########################################
//#######################################################################################################

/*********************************************************************************************\
 * kopiëer de string van een commando naar een string[]
 \*********************************************************************************************/
char* cmd2str(int i)
  {
  static char string[80];

  if(i<=COMMAND_MAX)
    strcpy(string,CommandText_tabel[i]);
  else
    string[0]=0;// als er geen gevonden wordt, dan is de string leeg

  return string;
  }


/*********************************************************************************************\
 * converteer een string met tijd volgens format HH:MM naar een UL int met tijd ----HHMM
 * 0xffffffff indien geen geldige invoer.
 \*********************************************************************************************/
unsigned long str2ultime(char* str)
  {
  byte y=0;
  unsigned long TimeInt=0L;
  byte x=strlen(str);
  
  while(!isdigit(str[x]))x--;
  while(isdigit(str[x]))
    {
    TimeInt|=(str[x]-'0')<<y;
    x--;
    y+=4;
    }
  
  y=8;
  while(!isdigit(str[x]))x--;
  while(isdigit(str[x]))
    {
    TimeInt|=(str[x]-'0')<<y;
    x--;
    y+=4;
    }
    
  x=((TimeInt>>12)&0xf)*10 + ((TimeInt>>8)&0xf); // Uren
  y=((TimeInt>>4 )&0xf)*10 + ((TimeInt   )&0xf); // Minuten
  
  if(x >23 || y>59)
    TimeInt=0xffffffff;

  return TimeInt;
  }

/*********************************************************************************************\
 * converteer een string met datum volgens format DD-MM-YYYY naar een UL int met datum ddmmyyyy
 * 0xffffffff indien geen geldige invoer.
 \*********************************************************************************************/
unsigned long str2uldate(char* str)
  {
  int x,y;
  unsigned long DateInt=0L;
  
  x=strlen(str);
  y=0;
  
  while(!isdigit(str[x]))x--;
  while(isdigit(str[x]))
    {
    DateInt|=((unsigned long)(str[x--]-'0'))<<y;
    y+=4;
    }
 
  if(DateInt<0x100)
    DateInt+=0x2000;

  y=16;
  while(!isdigit(str[x]))x--;
  while(isdigit(str[x]))
    {
    DateInt|=((unsigned long)(str[x--]-'0'))<<y;
    y+=4;
    }

  if((((DateInt>>20)&0xf)*10 + ((DateInt>>16 )&0xf))>12)// Maand: ongeldige invoer
    return 0xffffffff;
  
  y=24;
  while(!isdigit(str[x]))x--;
  while(isdigit(str[x]))
    {
    DateInt|=((unsigned long)(str[x--]-'0'))<<y;
    y+=4;
    }

  if((((DateInt>>28)&0xf)*10 + ((DateInt>>24 )&0xf))>31)// Maand: ongeldige invoer
    return 0xffffffff;
  
  return DateInt;
  }

/*********************************************************************************************\
 * Haal uit een string de commando code. False indien geen geldige commando code.
 \*********************************************************************************************/
int str2cmd(char *command)
  {
  for(int x=0;x<=COMMAND_MAX;x++)
    if(strcasecmp(command,cmd2str(x))==0)
      return x;      

  return false;
  }


/*********************************************************************************************\
 * String mag HEX, DEC
 * Deze routine converteert uit een string een unsigned long waarde.
 \*********************************************************************************************/
unsigned long str2int(char *string)
{
  // TODO   return(strtoul(string,NULL,0));
  unsigned long temp=atof(string);
  return temp;
}

/**********************************************************************************************\
 * Converteert een 4byte array IP adres naar een string.
 \*********************************************************************************************/
char* ip2str(byte* IP)
{
  static char str[20];
  sprintf(str,"%u.%u.%u.%u",IP[0],IP[1],IP[2],IP[3]);
  return str;
}

/**********************************************************************************************\
 * Converteert een string naar een 4byte array IP adres
 * 
 \*********************************************************************************************/
boolean str2ip(char *string, byte* IP)
  {
  byte c;
  byte part=0;
  int value=0;

  for(int x=0;x<=strlen(string);x++)
    {
    c=string[x];
    if(isdigit(c))
      {
      value*=10;
      value+=c-'0';
      }

    else if(c=='.' || c==0) // volgende deel uit IP adres
      {
      if(value<=255)
        IP[part++]=value;
      else 
        return false;
      value=0;
      }
    else if(c==' ') // deze tekens negeren
      ;
    else // ongeldig teken
    return false;
    }
  if(part==4)// correct aantal delen van het IP adres
    return true;
  return false;
}


/**********************************************************************************************\
 * Converteert een unsigned long naar een string met decimale integer.
 \*********************************************************************************************/
char* int2str(unsigned long x)
{
  static char OutputLine[12];
  char* OutputLinePosPtr=&OutputLine[10];
  int y;

  *OutputLinePosPtr=0;

  if(x==0)
    {
    *--OutputLinePosPtr='0';
    }
  else
    {  
    while(x>0)
      {
      *--OutputLinePosPtr='0'+(x%10);
      x/=10;
      }
    }    
  return OutputLinePosPtr;
  }

/**********************************************************************************************\
 * Converteert een unsigned long naar een hexadecimale string.
 \*********************************************************************************************/
char* int2strhex(unsigned long x)
  {
  static char OutputLine[12];
  char* OutputLinePosPtr=&OutputLine[10];
  int y;

  *OutputLinePosPtr=0;

  if(x==0)
    {
    *--OutputLinePosPtr='0';
    }
  else
    {  
    while(x>0)
      {
      y=x&0xf;

      if(y<10)
        *--OutputLinePosPtr='0'+y;
      else
        *--OutputLinePosPtr='A'+(y-10);

      x=x>>4;
      ;
    }
    *--OutputLinePosPtr='x';
    *--OutputLinePosPtr='0';
  }
  return OutputLinePosPtr;
}


/**********************************************************************************************\
 * vult een string met een regel uit de Eventlist.
 * geeft false terug als de regel leeg is
 * Let op dat er voldoende ruimte is in [Line]
 \*********************************************************************************************/
boolean EventlistEntry2str(int entry, byte d, char* Line, boolean Script)
  {
  struct NodoEventStruct Event, Action;
  ClearEvent(&Event);
  ClearEvent(&Action);
 
  char *TempString=(char*)malloc(INPUT_LINE_SIZE);
  boolean Ok;

  if(Ok=Eventlist_Read(entry,&Event,&Action)) // lees regel uit de Eventlist. Ga door als gelukt.
    {
    if(Event.Command) // Als de regel gevuld is
      {
      if(!Script)
        {
        // todo strcpy(Line,int2str(entry));
        sprintf(Line,"%s",int2str(entry));
        // todo strcat(Line,": ");
        sprintf(Line,"%s%s",Line,"; ");
        }                                          
      else
        {
        // todo strcpy(Line,cmd2str(CMD_EVENTLIST_WRITE));
        sprintf(Line,"%s",cmd2str(CMD_EVENTLIST_WRITE));
        // todo strcat(Line,"; ");
        sprintf(Line,"%s%s",Line,"; ");
        }
  
      // geef het event weer
      Event2str(&Event, TempString);
      // todo strcat(Line, TempString);
      sprintf(Line,"%s%s",Line,TempString);
  
      // geef het action weer
      // todo strcat(Line,"; ");
      sprintf(Line,"%s%s",Line,"; ");
      Event2str(&Action, TempString);  
      // todo strcat(Line,TempString);
      sprintf(Line,"%s%s",Line,TempString);
      }
    else
      Line[0]=0;
    }

  free(TempString);
  return Ok;
  }

/*******************************************************************************************************\
 * Houdt bij welke Nodo's binnen bereik zijn en via welke Poort.
 * Als Port ongelijk aan reeds bekende poort, dan wordt de lijst geactualiseerd.
 * Als Port=0 dan wordt alleen de poort teruggegeven als de Nodo bekend is.
 \*******************************************************************************************************/
byte NodoOnline(byte Unit, byte Port)
  {
  static byte NodoOnlinePort[UNIT_MAX+1];
  static boolean FirstTime=true;
  
  int x;
  
  // Maak eerste keer de tabel leeg.
  if(FirstTime)
    {
    FirstTime=false;
    for(x=0;x<=UNIT_MAX;x++)
      NodoOnlinePort[x]=0;
    NodoOnlinePort[Settings.Unit]=VALUE_SOURCE_SYSTEM;//Dit is deze unit.
    }
    
  if(Port && Port!=NodoOnlinePort[Unit])
    {
    // Werk tabel bij. Voorkeurspoort voor communicatie.

    if(Port==VALUE_SOURCE_I2C)
      NodoOnlinePort[Unit]=VALUE_SOURCE_I2C;
    else if(Port==VALUE_SOURCE_IR && NodoOnlinePort[Unit]!=VALUE_SOURCE_I2C)
      NodoOnlinePort[Unit]=VALUE_SOURCE_IR;
    else if(Port==VALUE_SOURCE_RF && NodoOnlinePort[Unit]!=VALUE_SOURCE_IR && NodoOnlinePort[Unit]!=VALUE_SOURCE_I2C)
      NodoOnlinePort[Unit]=VALUE_SOURCE_RF;
    }    
  return NodoOnlinePort[Unit];
  }
  
 /********************************************************************************************\
 * Een float en een unsigned long zijn beide 4bytes groot. Deze zijn niet te casten naar 
 * elkaar. Onderstaande twee funkties converteren de unsigned long
 * en de float.
 \*********************************************************************************************/
unsigned long float2ul(float f)
  {
  unsigned long ul;
  memcpy(&ul, &f,4);
  return ul;
  }

float ul2float(unsigned long ul)
  {
  float f;
  memcpy(&f, &ul,4);
  return f;
  }
  
