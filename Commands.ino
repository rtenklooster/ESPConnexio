/*********************************************************************************************\
 * Deze functie checked of de code die ontvangen is een uitvoerbare opdracht is/
 * Als het een correct commando is wordt deze uitgevoerd en 
 * true teruggegeven. Zo niet dan wordt er een 'false' retour gegeven.
 \*********************************************************************************************/

boolean ExecuteCommand(struct NodoEventStruct *EventToExecute)
  {
  unsigned long a;
  int w,x,y,z;
  byte error=0;
  
  struct NodoEventStruct TempEvent,TempEvent2;
  ClearEvent(&TempEvent);
  ClearEvent(&TempEvent2);
  
  char *TempString=(char*)malloc(80);
  char *TempString2=(char*)malloc(15);
  
  switch(EventToExecute->Command)
    {   
    case CMD_VARIABLE_TOGGLE:
      UserVar[EventToExecute->Par1-1]=UserVar[EventToExecute->Par1-1]>0.5?0.0:1.0;
      TempEvent.Type         = ESP_TYPE_EVENT;
      TempEvent.Command      = EVENT_VARIABLE;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2         = float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Direction    = VALUE_DIRECTION_INPUT;
      TempEvent.Port         = VALUE_SOURCE_SYSTEM;
      ProcessEvent(&TempEvent);
      break;        

    case CMD_VARIABLE_INC:
      UserVar[EventToExecute->Par1-1]+=ul2float(EventToExecute->Par2);
      TempEvent.Type         = ESP_TYPE_EVENT;
      TempEvent.Command      = EVENT_VARIABLE;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2         = float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Direction    = VALUE_DIRECTION_INPUT;
      TempEvent.Port         = VALUE_SOURCE_SYSTEM;
      ProcessEvent(&TempEvent);
      break;        

    case CMD_VARIABLE_DEC:
      UserVar[EventToExecute->Par1-1]-=ul2float(EventToExecute->Par2);
      TempEvent.Type         = ESP_TYPE_EVENT;
      TempEvent.Command      = EVENT_VARIABLE;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2         = float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Direction    = VALUE_DIRECTION_INPUT;
      TempEvent.Port         = VALUE_SOURCE_SYSTEM;
      ProcessEvent(&TempEvent);
      break;        

    case CMD_VARIABLE_SET:
      UserVar[EventToExecute->Par1-1]=ul2float(EventToExecute->Par2);
      TempEvent.SourceUnit   = Settings.Unit;
      TempEvent.Type         = ESP_TYPE_EVENT;
      TempEvent.Command      = EVENT_VARIABLE;
      TempEvent.Port         = VALUE_SOURCE_SYSTEM;
      TempEvent.Direction    = VALUE_DIRECTION_INPUT;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2         = EventToExecute->Par2;
      ProcessEvent(&TempEvent);      // verwerk binnengekomen event.
      break;         

    case CMD_VARIABLE_MULTIPLY:
      {
        float temp = UserVar[EventToExecute->Par1-1];
        Serial.println(temp);
        temp = temp * ul2float(EventToExecute->Par2);
        Serial.println(temp);
        UserVar[EventToExecute->Par1-1]=temp;
        break;
      }
      
    #if WIRED
    case CMD_VARIABLE_SET_WIRED_ANALOG:
      UserVar[EventToExecute->Par1-1]=analogRead(PIN_WIRED_IN_1+EventToExecute->Par2-1);
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Type         = ESP_TYPE_EVENT;
      TempEvent.Command=EVENT_VARIABLE;
      TempEvent.Port=VALUE_SOURCE_SYSTEM;
      TempEvent.Direction=VALUE_DIRECTION_INPUT;
      ProcessEvent(&TempEvent);      // verwerk binnengekomen event.
      break;
    #endif         
  
    case CMD_VARIABLE_VARIABLE:
      UserVar[EventToExecute->Par1-1]=UserVar[EventToExecute->Par2-1];
      TempEvent.Type         = ESP_TYPE_EVENT;
      TempEvent.Command=EVENT_VARIABLE;
      TempEvent.Port=VALUE_SOURCE_SYSTEM;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Direction=VALUE_DIRECTION_INPUT;
      ProcessEvent(&TempEvent);      // verwerk binnengekomen event.
      break;        

    case CMD_VARIABLE_ADD_VARIABLE:
      UserVar[EventToExecute->Par1-1]+=UserVar[EventToExecute->Par2-1];
      TempEvent.Type         = ESP_TYPE_EVENT;
      TempEvent.Command=EVENT_VARIABLE;
      TempEvent.Port=VALUE_SOURCE_SYSTEM;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Direction=VALUE_DIRECTION_INPUT;
      ProcessEvent(&TempEvent);      // verwerk binnengekomen event.
      break;        

    case CMD_VARIABLE_PULSE_COUNT:
      // Tellen van pulsen actief: enable IRQ behorende bij PIN_IR_RX_DATA
      // Als er toch een reeks pulsen komt, dan wordt in FetchSignal() het tellen van pulsen gedisabled.
      bitWrite(HW_Config,HW_PULSE,true);
      attachInterrupt(PULSE_IRQ,PulseCounterISR,PULSE_TRANSITION); 
      
      UserVar[EventToExecute->Par1-1]=PulseCount;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Command=EVENT_VARIABLE;
      TempEvent.Type=ESP_TYPE_EVENT;
      TempEvent.Port=VALUE_SOURCE_SYSTEM;
      TempEvent.Direction=VALUE_DIRECTION_INPUT;
      PulseCount=0;
      ProcessEvent(&TempEvent);      // verwerk binnengekomen event.
      break;         

    case CMD_VARIABLE_PULSE_TIME:
      // Tellen van pulsen actief: enable IRQ behorende bij PIN_IR_RX_DATA
      // Als er toch een reeks pulsen komt, dan wordt in FetchSignal() het tellen van pulsen gedisabled.
      bitWrite(HW_Config,HW_PULSE,true);
      attachInterrupt(PULSE_IRQ,PulseCounterISR,PULSE_TRANSITION); 

      UserVar[EventToExecute->Par1-1]=PulseTime;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Type= ESP_TYPE_EVENT;
      TempEvent.Command=EVENT_VARIABLE;
      TempEvent.Port=VALUE_SOURCE_SYSTEM;
      TempEvent.Direction=VALUE_DIRECTION_INPUT;
      ProcessEvent(&TempEvent);      // verwerk binnengekomen event.
      break;         

    case CMD_STOP:
      error=MESSAGE_EXECUTION_STOPPED;
      break;

    case CMD_BREAK_ON_VAR_EQU:
      {
      if((int)UserVar[EventToExecute->Par1-1]==(int)ul2float(EventToExecute->Par2))
        error=MESSAGE_BREAK;
      break;
      }
      
    case CMD_BREAK_ON_VAR_NEQU:
      if((int)UserVar[EventToExecute->Par1-1]!=(int)ul2float(EventToExecute->Par2))
        error=MESSAGE_BREAK;
      break;

    case CMD_BREAK_ON_VAR_MORE:
      if(UserVar[EventToExecute->Par1-1] > ul2float(EventToExecute->Par2))
        error=MESSAGE_BREAK;
      break;

    case CMD_BREAK_ON_VAR_LESS:
      if(UserVar[EventToExecute->Par1-1] < ul2float(EventToExecute->Par2))
        error=MESSAGE_BREAK;
      break;

    case CMD_BREAK_ON_VAR_LESS_VAR:
      if(UserVar[EventToExecute->Par1-1] < UserVar[EventToExecute->Par2-1])
        error=MESSAGE_BREAK;
      break;

    case CMD_BREAK_ON_VAR_MORE_VAR:
      if(UserVar[EventToExecute->Par1-1] > UserVar[EventToExecute->Par2-1])
        error=MESSAGE_BREAK;
      break;

    case CMD_SEND_USEREVENT:
      TempEvent.Port                  = VALUE_ALL;
      TempEvent.Type                  = ESP_TYPE_EVENT;
      TempEvent.Command               = EVENT_USEREVENT;
      TempEvent.Par1                  = EventToExecute->Par1;
      TempEvent.Par2                  = EventToExecute->Par2;
      SendEvent(&TempEvent, false, true,Settings.WaitFree==VALUE_ON);
      break;

    case CMD_VARIABLE_SEND:
      TempEvent.Type=ESP_TYPE_EVENT;
      TempEvent.Command=EVENT_VARIABLE;
      TempEvent.Port=EventToExecute->Par2;
      TempEvent.Direction=VALUE_DIRECTION_OUTPUT;
      TempEvent.Par1=EventToExecute->Par1;
      TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
      SendEvent(&TempEvent, false, true,Settings.WaitFree==VALUE_ON);
      break;         

    case CMD_VARIABLE_SAVE:
      for(z=1;z<=USER_VARIABLES_MAX;z++)
        {
        if(z==EventToExecute->Par1 || EventToExecute->Par1==0)
          {
          x=0;
          w=0;                                                                  // Plaats waar variabele al bestaat in eventlist
          y=0;                                                                  // hoogste beschikbare plaats
          while(Eventlist_Read(++x,&TempEvent,&TempEvent2))                     // Zoek of variabele al bestaat in eventlist
            {
            if(TempEvent.Type==ESP_TYPE_EVENT && TempEvent.Command==EVENT_BOOT && TempEvent.Par1==Settings.Unit)
              if(TempEvent2.Type==ESP_TYPE_COMMAND && TempEvent2.Command==CMD_VARIABLE_SET && TempEvent2.Par1==z)
                w=x;
              
            if(TempEvent.Command==0)
              y=x;
            }            

          x=w>0?w:y;                                                            // Bestaande regel of laatste vrije plaats.
    
          TempEvent.Type      = ESP_TYPE_EVENT;
          TempEvent.Command   = EVENT_BOOT;
          TempEvent.Par1      = Settings.Unit;
          TempEvent.Par2      = 0;
          TempEvent2.Type     = ESP_TYPE_COMMAND;
          TempEvent2.Command  = CMD_VARIABLE_SET;
          TempEvent2.Par1     = z;
          TempEvent2.Par2     = float2ul(UserVar[z-1]);
         
          Eventlist_Write(x, &TempEvent, &TempEvent2);                          // Schrijf weg in eventlist
          }
        }

      break;
         
    case CMD_TIMER_SET:
      if(EventToExecute->Par2==0)
        UserTimer[EventToExecute->Par1-1]=0L;
      else
        UserTimer[EventToExecute->Par1-1]=millis()+EventToExecute->Par2*1000L;
      break;

    case CMD_TIMER_SET_VARIABLE:
       UserTimer[EventToExecute->Par1-1]=millis()+(unsigned long)(UserVar[EventToExecute->Par2-1])*1000L;
      break;

    case CMD_TIMER_RANDOM:
      UserTimer[EventToExecute->Par1-1]=millis()+random(EventToExecute->Par2)*1000;
      break;

    case CMD_DELAY:
      Wait(EventToExecute->Par1, false, 0, false);
      break;        

    case CMD_SEND_EVENT:
      TempEvent=LastReceived;
      TempEvent.Port=EventToExecute->Par1==0?VALUE_ALL:EventToExecute->Par1;
      SendEvent(&TempEvent, TempEvent.Command==EVENT_RAWSIGNAL,true, Settings.WaitFree==VALUE_ON);
      break;        

    case CMD_SOUND: 
      Alarm(EventToExecute->Par1,EventToExecute->Par2);
      break;     
  
    #if WIRED
    case CMD_WIRED_PULLUP:
      Settings.WiredInputPullUp[EventToExecute->Par1-1]=EventToExecute->Par2; // EventToExecute->Par1 is de poort[1..]
      
      if(EventToExecute->Par2==VALUE_ON)
        pinMode(A0+PIN_WIRED_IN_1+EventToExecute->Par1-1,INPUT_PULLUP);
      else
        pinMode(A0+PIN_WIRED_IN_1+EventToExecute->Par1-1,INPUT);
      break;
                 
    case CMD_WIRED_OUT:
      digitalWrite(PIN_WIRED_OUT_1+EventToExecute->Par1-1,(EventToExecute->Par2==VALUE_ON));
      WiredOutputStatus[EventToExecute->Par1-1]=(EventToExecute->Par2==VALUE_ON);
      bitWrite(HW_Config,HW_WIRED_OUT,true);
      TempEvent.Par1=EventToExecute->Par1;
      TempEvent.Port=VALUE_SOURCE_SYSTEM;
      TempEvent.Direction=VALUE_DIRECTION_OUTPUT;
      PrintEvent(&TempEvent,VALUE_ALL);
      break;

    case CMD_WIRED_SMITTTRIGGER:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=WIRED_PORTS)
        Settings.WiredInputSmittTrigger[EventToExecute->Par1-1]=EventToExecute->Par2;
      break;                  

    case CMD_WIRED_THRESHOLD:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=WIRED_PORTS)
        Settings.WiredInputThreshold[EventToExecute->Par1-1]=EventToExecute->Par2;
      break;                  

    #endif //WIRED
                         
    case CMD_SETTINGS_SAVE:
      UndoNewNodo();// Status NewNodo verwijderen indien van toepassing
      Save_Settings();
      break;

    case CMD_OUTPUT:
      switch(EventToExecute->Par1)
        {
        case VALUE_SOURCE_IR:
          Settings.TransmitIR=EventToExecute->Par2;
          break;       
        case VALUE_SOURCE_RF:
          Settings.TransmitRF=EventToExecute->Par2;
          break;       
        }
      
      break;
      
    case CMD_STATUS:
      Status(EventToExecute);
      break;
            
    case CMD_UNIT_SET:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=UNIT_MAX)
        {
        Settings.Unit=EventToExecute->Par1;  
        Save_Settings();
        Reboot();
        break;
        }
      
    case CMD_REBOOT:
      Reboot();
      break;        
      
    case CMD_RESET:
      ResetFactory();
      break;

    case CMD_EVENTLIST_ERASE:
      Led(BLUE);
      if(EventToExecute->Par1==0)
        {
        x=1;                                          
        UndoNewNodo();// Status NewNodo verwijderen indien van toepassing
        while(Eventlist_Write(x++,&TempEvent,&TempEvent));
        }
      else
        {
        Eventlist_Write(EventToExecute->Par1,&TempEvent,&TempEvent);
        }
      break;        
        
    case CMD_EVENTLIST_SHOW:
      // Er kunnen zich hier twee situaties voordoen: het verzoek is afkomstig van een Terminal (Serial/Telnet) of 
      // via IR/RF/I2C. Beide kennen een andere afhandeling immers de Terminal variant kan direct naar de MMI.
      // Bij de anderen moet er nog transport plaats vinden via IR, RF, I2C. De Terminal variant is NIET relevant
      // voor een Small omdat deze geen MMI heeft.

      if(EventToExecute->Port==VALUE_SOURCE_SERIAL || EventToExecute->Port==VALUE_SOURCE_TELNET)
        {      
        if(EventToExecute->Par1<=EventlistMax)
          {
          Serial.println(Text_22);
          if(EventToExecute->Par1==0)
            {
            x=1;
            while(EventlistEntry2str(x++,0,TempString,false))
              if(TempString[0]!=0)
                Serial.println(TempString);
            }
          else
            {
            EventlistEntry2str(EventToExecute->Par1,0,TempString,false);
              if(TempString[0]!=0)
                Serial.println(TempString);
            }
          Serial.println(Text_22);
          }
        else
          error=MESSAGE_INVALID_PARAMETER;
        }
      else // Transmissie via I2C/RF/IR: dan de inhoud van de Eventlist versturen.
        {
      
        if(EventToExecute->Par1==0)
          {
          x=1;
          y=EventlistMax;
          }
        else
          {
          x=EventToExecute->Par1;
          y=EventToExecute->Par1;
          }
                
        // Haal de event en action op uit eeprom en verzend deze met extra transmissie vlaggen zodat de data:
        // 1. alleen wordt verstuurd naar de nodo die de data heeft opgevraagd.
        // 2. alleen wordt verzonden naar de poort waar het verzoek vandaan kwam
        // 3. aan de ontvangende zijde in de queue wordt geplaatst
        // 4. de vlag VIEW_SPECIAL mee krijgt zodat de events/commando's niet worden uitgevoerd aan de ontvangende zijde.
        // 5. Met LOCK alle andere Nodo's tijdelijk in de hold worden gezet.
        // In geval van verzending naar queue zal deze tijd niet van toepassing zijn omdat er dan geen verwerkingstijd nodig is.
        // Tussen de events die de queue in gaan een kortere delay tussen verzendingen.

        z=EventToExecute->Port;
        w=EventToExecute->SourceUnit;
        
        while(x<=y && Eventlist_Read(x,&TempEvent,&TempEvent2))
          {
          ClearEvent(EventToExecute);
          EventToExecute->Par1=x;
          EventToExecute->Command=SYSTEM_COMMAND_QUEUE_EVENTLIST_SHOW;
          EventToExecute->Flags=TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT | TRANSMISSION_BUSY;
          EventToExecute->Type=ESP_TYPE_SYSTEM;
          EventToExecute->Port=z;
          EventToExecute->SourceUnit=Settings.Unit;
          EventToExecute->DestinationUnit=w;

          if(TempEvent.Command!=0)
            {
            SendEvent(EventToExecute,false,false,false);

            TempEvent.Flags=TRANSMISSION_VIEW_SPECIAL | TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT | TRANSMISSION_BUSY;
            TempEvent.Port=z;
            TempEvent.DestinationUnit=w;
            SendEvent(&TempEvent,false,false,false);
    
    
            if(x==y)                                                            // Als laatste regel uit de eventlist, dan de ether weer vrijgeven. 
              TempEvent2.Flags=TRANSMISSION_VIEW_SPECIAL | TRANSMISSION_QUEUE ; 
            else
              TempEvent2.Flags=TRANSMISSION_VIEW_SPECIAL | TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT | TRANSMISSION_BUSY;

            TempEvent2.Port=z;
            TempEvent2.DestinationUnit=w;
            SendEvent(&TempEvent2,false,false,false);
            }
          x++;
          }
        }        
      break;

    case CMD_VARIABLE_GET: // VariableReceive <Variabelenummer_Bestemming>, <unit>, <Variabelenummer_Bron_Andere_ESP>
      y=0; // retries
      error=MESSAGE_SENDTO_ERROR;
      do
        {
        // <VariabeleNummerBestemming> zit in Par1
        // <Unit> zit in bit 0..7 van Par2
        // <VariabeleNummerBron> zit in bit bit 15..8 van Par2
        //
        // Verzend naar de andere Nodo een verzoek om de variabele te verzenden.
        ClearEvent(&TempEvent);
        TempEvent.DestinationUnit=EventToExecute->Par2&0xff;
        TempEvent.Type=ESP_TYPE_COMMAND;
        TempEvent.Command=CMD_VARIABLE_SEND;
        TempEvent.Port=VALUE_ALL;
        TempEvent.Par1=(EventToExecute->Par2>>8)&0xff;                          // VariabeleBron
        TempEvent.Par2=NodoOnline(EventToExecute->Par2&0xff,0);                 // Poort waaronder de Slave Nodo bekend is.
        TempEvent.Flags=TRANSMISSION_QUEUE;

        if(TempEvent.Par2==0)                                                   // Als unitnummer niet bekend is
          TempEvent.Par2=VALUE_ALL;

        if(TempEvent.Par2==VALUE_SOURCE_SYSTEM)
          {
          error=MESSAGE_INVALID_PARAMETER;
          break;
          }

        SendEvent(&TempEvent,false,y==0,Settings.WaitFree==VALUE_ON);
        
        // Wacht tot event voorbij komt. De Wait(); funktie wacht op type, command en unit.
        ClearEvent(&TempEvent);
        TempEvent.SourceUnit          = EventToExecute->Par2&0xff;
        TempEvent.Command             = EVENT_VARIABLE;
        TempEvent.Type                = ESP_TYPE_EVENT;

        if(Wait(3,false,&TempEvent,false))
          {
          TempEvent.Par1            = EventToExecute->Par1;  
          TempEvent.Type            = ESP_TYPE_COMMAND;
            TempEvent.Command         = CMD_VARIABLE_SET;
          TempEvent.Direction       = VALUE_DIRECTION_INPUT;
          if(QueuePosition)QueuePosition--;                                   // binnengekomen event is eveneens op de queue geplaatst. deze mag weg.
          ProcessEvent(&TempEvent);                                           // verwerk binnengekomen event.
          error=0;
          }
        }while(error && ++y<3);
      break;        

    case CMD_WIFI_SCAN:
      {
        Serial.println("scan start");
        // WiFi.scanNetworks will return the number of networks found
        int n = WiFi.scanNetworks();
        Serial.println("scan done");
        if (n == 0)
          Serial.println("no networks found");
        else
          {
            Serial.print(n);
            Serial.println(" networks found");
            for (int i = 0; i < n; ++i)
              {
                // Print SSID and RSSI for each network found
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(WiFi.SSID(i));
                Serial.print(" (");
                Serial.print(WiFi.RSSI(i));
                Serial.print(")");
                Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
                delay(10);
              }
          }
        Serial.println("");
      }
      break;        

    case CMD_WIFI_CONNECT:
      {
        if (Settings.WifiSSID[0] != 0)
          {
            WiFi.mode(WIFI_STA);
            WiFi.begin(Settings.WifiSSID, Settings.WifiKey);
            while (WiFi.status() != WL_CONNECTED)
              {
                delay(500);
                Serial.print(".");
              }
            Serial.println("");
          }
        else
          Serial.println("No SSID!");
      }
      break;        

    case CMD_WIFI_DISCONNECT:
      {
        WiFi.disconnect();
      }
      break;        

    case CMD_DOMOTICZ_GET:
      {
        if (WiFi.status() == WL_CONNECTED)
          {
            float temp=0;
            if (Domoticz_getData(EventToExecute->Par1,&temp))
              {
                UserVar[EventToExecute->Par2-1]=temp;
                Serial.println(UserVar[EventToExecute->Par2-1]);
              }
            else
              {
                Serial.println("Error getting Domoticz data");
               error=MESSAGE_BREAK;
             }
          }
        else
          Serial.println("Not connected");
      }
      break;        

    case CMD_DOMOTICZ_SET:
      {
        if (WiFi.status() == WL_CONNECTED)
          {
            float temp=UserVar[EventToExecute->Par2-1];
            Domoticz_sendData(EventToExecute->Par1,temp);
          }
        else
          Serial.println("Not connected");
      }
      break;

    }

  free(TempString2);
  free(TempString);

  return error;
  }

