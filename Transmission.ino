/**********************************************************************************************\
 * verzendt een event en geeft dit tevens weer op SERIAL
 * Als UseRawSignal=true, dan wordt er geen signaal opgebouwd, maar de actuele content van de
 * RawSignal buffer gebruikt. In dit geval werkt de WaitFree niet.
 \*********************************************************************************************/
boolean SendEvent(struct NodoEventStruct *ES, boolean UseRawSignal, boolean Display, boolean WaitForFree)
  {    
  ES->Direction=VALUE_DIRECTION_OUTPUT;
  byte Port=ES->Port;
    
  if(Settings.WaitFreeNodo==VALUE_ON)
    {
    if(BusyNodo!=0)                                                               // Als een Nodo heeft aangegeven busy te zijn, dan wachten.
      {
      if(!Wait(30,true,0,false))
        {
        for(byte x=1;x<=31;x++)
          if(BusyNodo&(1<<x))
            RaiseMessage(MESSAGE_BUSY_TIMEOUT,x);

        BusyNodo=0;
        }
      }
      
    if(ES->Type!=ESP_TYPE_SYSTEM && ES->Command!=SYSTEM_COMMAND_CONFIRMED)
      {
      ES->Flags|=TRANSMISSION_BUSY | TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT;
      RequestForConfirm=true;
      }
    }  

  // loop de plugins langs voor eventuele afhandeling van dit event.
  if(!UseRawSignal) PluginCall(PLUGIN_EVENT_OUT, ES,0);

  // Stuur afhankelijk van de instellingen het event door naar I2C, RF, IR. Eerst wordt het event geprint,daarna een korte wachttijd om
  // te zorgen dat er een minimale wachttijd tussen de signlen zit. Tot slot wordt het signaal verzonden.
  if(WaitForFree)
    if(Port==VALUE_SOURCE_RF || Port==VALUE_SOURCE_IR ||(Settings.TransmitRF==VALUE_ON && Port==VALUE_ALL))
      WaitFree();

  if(!UseRawSignal)
    ESP_2_RawSignal(ES);

  // Respecteer een minimale tijd tussen verzenden van events. Wachten alvorens event te verzenden.
  while(millis()<HoldTransmission);  
                                             
  // Verstuur signaal als IR
  if(Settings.TransmitIR==VALUE_ON && (Port==VALUE_SOURCE_IR || Port==VALUE_ALL))
    { 
    ES->Port=VALUE_SOURCE_IR;
    if(Display)PrintEvent(ES,VALUE_ALL);
    RawSendIR();
    }
  
  // Verstuur signaal als RF
  if(Settings.TransmitRF==VALUE_ON && (Port==VALUE_SOURCE_RF || Port==VALUE_ALL))
    {
    ES->Port=VALUE_SOURCE_RF;
    if(Display)PrintEvent(ES,VALUE_ALL);
    RawSendRF();
    }

  HoldTransmission=DELAY_BETWEEN_TRANSMISSIONS+millis();        
  }


/*********************************************************************************************\
 * Deze funktie berekend de CRC-8 checksum uit van een NodoEventStruct. 
 * Als de Checksum al correct gevuld was wordt er een true teruggegeven. Was dit niet het geval
 * dan wordt NodoEventStruct.Checksum gevuld met de juiste waarde en een false teruggegeven.
 \*********************************************************************************************/

boolean Checksum(NodoEventStruct *event)
  {
  byte OldChecksum=event->Checksum;
  byte NewChecksum=ESP_VERSION_MAJOR;  // Verwerk versie in checksum om communicatie tussen verschillende versies te voorkomen

  event->Checksum=0; // anders levert de beginsituatie een andere checksum op

  for(int x=0;x<sizeof(struct NodoEventStruct);x++)
    NewChecksum^(*((byte*)event+x)); 

  event->Checksum=NewChecksum;
  return(OldChecksum==NewChecksum);
  }

/**********************************************************************************************\
 * Deze functie wacht totdat de 433 en de IR band vrij zijn of er een timeout heeft plaats gevonden.
 * Er wordt gemeten hoeveel geldige pulsen ( > MIN_PULSE_LENGTH) er zich voordoen binnen een 
 * vast tijdsframe. Als er spikes zijn (=ruis) of er zijn geen pulsen, dan wordt teruggekeerd.
 \*********************************************************************************************/
void WaitFree(void)
  {
  unsigned long TimeOutTimer=millis()+WAIT_FREE_RX_TIMEOUT;                     // tijd waarna de routine wordt afgebroken in milliseconden
  unsigned long WindowTimer;

  int x,y;
  
  Led(BLUE);

  do
    {
    WindowTimer=millis()+250;
    x=0;
    while(WindowTimer>millis())
      {
      if((*portInputRegister(IRport)&IRbit)==0)                                 // Puls op RF?
        {
        x++;
        if(pulseIn(PIN_IR_RX_DATA,LOW) < MIN_PULSE_LENGTH && x>0)               // Was het een spike?
          x=0;
        }
      if((*portInputRegister(RFport)&RFbit)==RFbit)                             // Pulse op IR
        {
        y++;
        if(pulseIn(PIN_RF_RX_DATA,HIGH) < MIN_PULSE_LENGTH && y>0)              // Was het een spike?
          y=0;
        }
      }
    }while(TimeOutTimer>millis() && (x>0 || y>0));                              // Zolang nog pulsen op RF of IR en nog geen timeout

  Led(RED);
  }

boolean Domoticz_getData(int idx, float *data)
{
  boolean success=false;
  char host[20];
  sprintf(host,"%u.%u.%u.%u",Settings.Server_IP[0],Settings.Server_IP[1],Settings.Server_IP[2],Settings.Server_IP[3]);

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ServerPort)) {
    Serial.println("connection failed");
    return false;
  }
  
  // We now create a URI for the request
  String url = "/json.htm?type=devices&rid=";
  url += idx;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");

  unsigned long timer=millis()+200;
  while(!client.available() && millis()<timer) {}
  
  // Read all the lines of the reply from server and print them to Serial

  while(client.available()){
    String line = client.readStringUntil('\n');
    if (line.substring(10,14) == "Data")
      {
        String strValue=line.substring(19);
        byte pos=strValue.indexOf(' ');
        strValue=strValue.substring(0,pos);
        strValue.trim();
        float value = strValue.toFloat();
        *data=value;
        Serial.println("Succes!");
        success=true;
      }
  }
  Serial.println("closing connection");
  return success;
}

boolean Domoticz_sendData(int idx, float value)
{
  boolean success=false;
  char host[20];
  sprintf(host,"%u.%u.%u.%u",Settings.Server_IP[0],Settings.Server_IP[1],Settings.Server_IP[2],Settings.Server_IP[3]);
  
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ServerPort)) {
    Serial.println("connection failed");
    return false;
  }
  
  // We now create a URI for the request
  String url = "/json.htm?type=command&param=udevice&idx=";
  url += idx;
  url += "&svalue=";
  url += value;

  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");

  unsigned long timer=millis()+200;
  while(!client.available() && millis()<timer) {}
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\n');
    if (line.substring(0,15) == "HTTP/1.0 200 OK")
      {
        Serial.println("Succes!");
        success=true;
      }
  }
  Serial.println("closing connection");
  return success;
}

