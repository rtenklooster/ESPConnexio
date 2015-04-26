unsigned long RepeatingTimer=0L;
unsigned long EventHashPrevious=0,SignalHash,SignalHashPrevious=0L;

boolean ScanEvent(struct NodoEventStruct *Event)                                // Deze routine maakt deel uit van de hoofdloop en wordt iedere 125uSec. doorlopen
  {
  byte Fetched=0;
  static byte Focus=0;
  static boolean BlockRepeatsStatus=false;
  unsigned long Timer=millis()+SCAN_HIGH_TIME;

  while(Timer>millis() || RepeatingTimer>millis())
    {
/*
      if(Focus==0 || Focus==VALUE_SOURCE_IR)
      {
      if(FetchSignal(PIN_IR_RX_DATA,LOW))                                       // IR: *************** kijk of er data start **********************
        {
        if(AnalyzeRawSignal(Event))
          {
          bitWrite(HW_Config,HW_IR_RX,true);
          Fetched=VALUE_SOURCE_IR;
          Focus=Fetched;
          }
        }
      }
    
    
    if(Focus==0 || Focus==VALUE_SOURCE_RF)
      {
      if(FetchSignal(PIN_RF_RX_DATA,HIGH))                                      // RF: *************** kijk of er data start **********************
        {
        if(AnalyzeRawSignal(Event))
          {
          bitWrite(HW_Config,HW_RF_RX,true);
          Fetched=VALUE_SOURCE_RF;
          Focus=Fetched;
          }
        }
      }
*/
    if(Fetched)
      {
      HoldTransmission=DELAY_BETWEEN_TRANSMISSIONS+millis();
      SignalHash=(Event->Command<<24 | Event->Type<<16 | Event->Par1<<8) ^ Event->Par2;
      Event->Port=Fetched;
      Event->Direction=VALUE_DIRECTION_INPUT;
      Fetched=0;
      
      if(RawSignal.RepeatChecksum)RawSignal.Repeats=true;
            
      // Er zijn een aantal situaties die moeten leiden te een event. Echter er zijn er ook die (nog) niet mogen leiden 
      // tot een event en waar het binnengekomen signaal moet worden onderdrukt.
      
      // 1. Het is een (niet repeterend) Nodo signaal of is de herkomst I2C => Alle gevallen doorlaten
      if(Event->Type==ESP_TYPE_EVENT || Event->Type==ESP_TYPE_COMMAND || Event->Type==ESP_TYPE_SYSTEM || Event->Port==VALUE_SOURCE_I2C)
        Fetched=1;

      // 2. Het (mogelijk repeterend) binnenkomende signaal is niet recent eerder binnengekomen, zoals plugin signalen als KAKU, NewKAKU, ... => Herhalingen onderdrukken  
      else if(!RawSignal.RepeatChecksum && (SignalHash!=SignalHashPrevious || RepeatingTimer<millis())) 
        Fetched=2;

      // 3. Het is een herhalend signaal waarbij een herhaling wordt gebruikt als checksum zoals RAwSignals => Pas na twee gelijke signalen een event.
      else if(RawSignal.RepeatChecksum && SignalHash==SignalHashPrevious && (SignalHash!=EventHashPrevious || RepeatingTimer<millis())) 
        Fetched=3;

      // Serial.print(F("DEBUG: Fetched. SignalHash="));Serial.print(SignalHash,HEX);Serial.print(F(", RawSignal.Repeats="));Serial.print(RawSignal.Repeats);Serial.print(F(", RawSignal.RepeatChecksum="));Serial.print(RawSignal.RepeatChecksum);Serial.print(F(", RepeatingTimer>millis()="));Serial.print(RepeatingTimer>millis());Serial.print(F(", Fetched="));Serial.println(Fetched);

      SignalHashPrevious=SignalHash;

      if (Fetched!=1) RepeatingTimer=millis()+SIGNAL_REPEAT_TIME;

      if(Fetched)
        {
        // Nodo event: Toets of de versienummers corresponderen. Is dit niet het geval, dan zullen er verwerkingsfouten optreden! Dan een waarschuwing tonen en geen verdere verwerking.
        // Er is een uitzondering: De eerste commando/eventnummers zijn stabiel en mogen van oudere versienummers zijn.
        if(Event->Version!=0 && Event->Version!=ESP_VERSION_MINOR && Event->Command>COMMAND_MAX_FIXED)
          {
          Event->Command=CMD_DUMMY;
          Event->Type=ESP_TYPE_EVENT;
          Event->Par1=0;
          Event->Par2=0;
          PrintEvent(Event,VALUE_ALL);
          RaiseMessage(MESSAGE_VERSION_ERROR,Event->Version);
          return false;
          }     
    
        // als het informatie uitwisseling tussen Nodo's betreft...
        if(Event->Type==ESP_TYPE_EVENT || Event->Type==ESP_TYPE_COMMAND || Event->Type==ESP_TYPE_SYSTEM)
          {
          // registreer welke Nodo's op welke poort zitten en actualiseer tabel.
          // Wordt gebruikt voor SendTo en I2C communicatie op de Mega.
          // Hiermee kan later automatisch de juiste poort worden geselecteerd met de SendTo en kan in
          // geval van I2C communicatie uitsluitend naar de I2C verbonden Nodo's worden gecommuniceerd.
          NodoOnline(Event->SourceUnit,Event->Port);

          // Een Nodo kan aangeven dat hij Busy is.
          bitWrite(BusyNodo, Event->SourceUnit,(Event->Flags&TRANSMISSION_BUSY)>0);
          }
                  
        // Als het Nodo event voor deze unit bestemd is, dan klaar. Zo niet, dan terugkeren met een false
        // zodat er geen verdere verwerking plaatsvindt.
        if(Event->DestinationUnit==0 || Event->DestinationUnit==Settings.Unit)
          {
          EventHashPrevious=SignalHash;
          // PrintNodoEvent("DEBUG: ScanEvent(): Fetched", Event);
          return true;
          }
        }
      }
    }// while
  Focus=0;
  return false;
  }

