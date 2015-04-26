#define ESP_PULSE_0                 500  // PWM: Tijdsduur van de puls bij verzenden van een '0' in uSec.
#define ESP_PULSE_MID              1000  // PWM: Pulsen langer zijn '1'
#define ESP_PULSE_1                1500  // PWM: Tijdsduur van de puls bij verzenden van een '1' in uSec. (3x ESP_PULSE_0)
#define ESP_SPACE                   500  // PWM: Tijdsduur van de space tussen de bitspuls bij verzenden van een '1' in uSec.   

/*********************************************************************************************\
 * Deze routine zendt een RAW code via RF. 
 * De inhoud van de buffer RawSignal moet de pulstijden bevatten. 
 * RawSignal.Number het aantal pulsen*2
 \*********************************************************************************************/
void RawSendRF(void)
  {
  int x;
  digitalWrite(PIN_RF_RX_VCC,LOW);                                              // Spanning naar de RF ontvanger uit om interferentie met de zender te voorkomen.
  digitalWrite(PIN_RF_TX_VCC,HIGH);                                             // zet de 433Mhz zender aan

  delay(TRANSMITTER_STABLE_TIME);                                               // kleine pauze om de zender de tijd te geven om stabiel te worden 

  // LET OP: In de Arduino versie 1.0.1 zit een bug in de funktie delayMicroSeconds(). Als deze wordt aangeroepen met een nul dan zal er
  // een pause optreden van 16 milliseconden. Omdat het laatste element van RawSignal af sluit met een nul (omdat de space van de stopbit 
  // feitelijk niet bestaat) zal deze bug optreden. Daarom wordt deze op 1 gezet om de bug te omzeilen. 
  RawSignal.Pulses[RawSignal.Number]=1;

  for(byte y=0; y<RawSignal.Repeats; y++)                                       // herhaal verzenden RF code
    {
    x=1;
    noInterrupts();
    while(x<RawSignal.Number)
      {
      digitalWrite(PIN_RF_TX_DATA,HIGH);
      delayMicroseconds(RawSignal.Pulses[x++]*RawSignal.Multiply-5);            // min een kleine correctie  
      digitalWrite(PIN_RF_TX_DATA,LOW);
      delayMicroseconds(RawSignal.Pulses[x++]*RawSignal.Multiply-7);            // min een kleine correctie
      }
    interrupts();
    delay(RawSignal.Delay);// Delay buiten het gebied waar de interrupts zijn uitgeschakeld! Anders werkt deze funktie niet.
    }

  delay(TRANSMITTER_STABLE_TIME);                                               // kleine pause zodat de ether even schoon blijft na de stopbit

  digitalWrite(PIN_RF_TX_VCC,LOW); // zet de 433Mhz zender weer uit
  digitalWrite(PIN_RF_RX_VCC,HIGH); // Spanning naar de RF ontvanger weer aan.
  }


/*********************************************************************************************\
 * Deze routine zendt een RawSignal via IR. 
 * De inhoud van de buffer RawSignal moet de pulstijden bevatten. 
 * RawSignal.Number het aantal pulsen*2
 * Pulsen worden verzonden op en draaggolf van 38Khz.
 *
 * LET OP: Deze routine is speciaal geschreven voor de Arduino Mega1280 of Mega2560 met een
 * klokfrequentie van 16Mhz.
 \*********************************************************************************************/

void RawSendIR(void)
  {
  int pulse;                                                                    // pulse (bestaande uit een mark en een space) uit de RawSignal tabel die moet worden verzonden
  int mod;                                                                      // pulsenteller van het 38Khz modulatie signaal
  
  delay(10);                                                                    // kleine pause zodat verzenden event naar de USB poort gereed is, immers de IRQ's worden tijdelijk uitgezet
  
  // LET OP: In de Arduino versie 1.0.1 zit een bug in de funktie delayMicroSeconds(). Als deze wordt aangeroepen met een nul dan zal er
  // een pause optreden van 16 milliseconden. Omdat het laatste element van RawSignal af sluit met een nul (omdat de space van de stopbit 
  // feitelijk niet bestaat) zal deze bug optreden. Daarom wordt deze op 1 gezet om de bug te omzeilen. 
  RawSignal.Pulses[RawSignal.Number]=1;
  
  for(int repeat=0; repeat<RawSignal.Repeats; repeat++)                         // herhaal verzenden IR code
    {
    pulse=1;
    noInterrupts();
    while(pulse<(RawSignal.Number))
      {
      // Mark verzenden. Bereken hoeveel pulsen van 26uSec er nodig zijn die samen de lengte van de mark/space zijn.
      mod=(RawSignal.Pulses[pulse++]*RawSignal.Multiply)/26;                    // delen om aantal pulsen uit te rekenen

      while(mod)
        {
        // Hoog
        #if ESP_MEGA
        bitWrite(PORTH,0, HIGH);
        #else
        // todo bitWrite(PORTB,3, HIGH);
        #endif

        delayMicroseconds(12);
        // todo __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");// per nop 62.6 nano sec. @16Mhz
  
        // Laag
        #if ESP_MEGA
        bitWrite(PORTH,0, LOW);    
        #else
        // todo bitWrite(PORTB,3, LOW);
        #endif
        delayMicroseconds(12);
        // todo __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");// per nop 62.6 nano sec. @16Mhz
        mod--;
        }

      // Laag
      delayMicroseconds(RawSignal.Pulses[pulse++]*RawSignal.Multiply);
      }
    interrupts(); // interupts weer inschakelen.  
    delay(RawSignal.Delay);// Delay buiten het gebied waar de interrupts zijn uitgeschakeld! Anders werkt deze funktie niet
    }
  }

/*********************************************************************************************\
 * Deze routine berekend de RAW pulsen van een Nodo event en plaatst deze in de buffer RawSignal
 * RawSignal.Bits het aantal pulsen*2+startbit*2
 \*********************************************************************************************/

// Definieer een datablock die gebruikt wordt voor de gegevens die via de ether verzonden moeten worden.
struct DataBlockStruct
  {
  byte Version;
  byte SourceUnit;
  byte DestinationUnit;
  byte Flags;
  byte Type;
  byte Command;
  byte Par1;
  unsigned long Par2;
  byte Checksum;
  };  

void ESP_2_RawSignal(struct NodoEventStruct *Event)
  {
  struct DataBlockStruct DataBlock;

  byte BitCounter           = 1;
  RawSignal.Repeats         = 1;                                                // 1 pulsenreeks. Nodo signaal heeft geen herhalingen
  RawSignal.Delay           = 0;                                                // Geen repeats, dus delay tussen herhalingen ook niet relevant
  RawSignal.Multiply        = 25;

  Checksum(Event);

  DataBlock.SourceUnit      = Event->SourceUnit | (HOME_ESP<<5);  
  DataBlock.DestinationUnit = Event->DestinationUnit;
  DataBlock.Flags           = Event->Flags;
  DataBlock.Type            = Event->Type;
  DataBlock.Command         = Event->Command;
  DataBlock.Par1            = Event->Par1;
  DataBlock.Par2            = Event->Par2;
  DataBlock.Checksum        = Event->Checksum;;
  DataBlock.Version         = ESP_VERSION_MINOR;
  
  byte *B=(byte*)&DataBlock;

  // begin met een lange startbit. Veilige timing gekozen zodat deze niet gemist kan worden
  RawSignal.Pulses[BitCounter++]=(ESP_PULSE_1 *2)/RawSignal.Multiply; 
  RawSignal.Pulses[BitCounter++]=(ESP_SPACE   *2)/RawSignal.Multiply;

  for(byte x=0;x<sizeof(struct DataBlockStruct);x++)
    {
    for(byte Bit=0; Bit<=7; Bit++)
      {
      if((*(B+x)>>Bit)&1)
        RawSignal.Pulses[BitCounter++]=ESP_PULSE_1/RawSignal.Multiply; 
      else
        RawSignal.Pulses[BitCounter++]=ESP_PULSE_0/RawSignal.Multiply;   
      RawSignal.Pulses[BitCounter++]=ESP_SPACE/RawSignal.Multiply;             
      }
    }

  RawSignal.Pulses[BitCounter-1]=ESP_SPACE/RawSignal.Multiply; // pauze tussen de pulsreeksen
  RawSignal.Number=BitCounter;
  }

// De while() loop waar de statemask wordt getest doorloopt een aantal cycles per milliseconde. Dit is afhankelijk
// van de kloksnelheid van de Arduino. Deze routine is in de praktijk geklokt met een processorsnelheid van
// 16Mhz. Naast de doorlijktijd van de while() loop en er ook nog overhead die moet worden opgetelt bij de 
// uiteindelijk gemeten pulstijd. Tijden zijn in de praktijk uitgeklokt met een analyser, echter per arduino
// kunnen er kleine verschillen optreden. Timings gemeten aan de IR_RX_DATA en RF_RX_DATA ingangen. Eigenschappen
// van de ontvangers kunnen eveneens van invloed zijn op de pulstijden.
 
const unsigned long LoopsPerMilli=345;
const unsigned long Overhead=0;  

/**********************************************************************************************\
 * Haal de pulsen en plaats in buffer. 
 * bij de TSOP1738 is in rust is de uitgang hoog. StateSignal moet LOW zijn
 * bij de 433RX is in rust is de uitgang laag. StateSignal moet HIGH zijn
 * 
 \*********************************************************************************************/

// Omdat deze routine tijdkritisch is halen we de gebruikte variabelen op globaal niveau
// zodat ze niet bij iedere functie-call opnieuw geinitialiseerd hoeven te worden. dit scheelt 
// verwerkingstijd. Als er geen signaal is, neemt deze funktie (incl. de call) 
int RawCodeLength=0;
unsigned long PulseLength=0;
unsigned long numloops=0;
unsigned long maxloops=0;

boolean Ftoggle=false;
uint8_t Fbit=0;
uint8_t Fport=0;
uint8_t FstateMask=0;

boolean FetchSignal(byte DataPin, boolean StateSignal)
  {
  uint8_t Fbit = digitalPinToBitMask(DataPin);
  uint8_t Fport = digitalPinToPort(DataPin);
  uint8_t FstateMask = (StateSignal ? Fbit : 0);

  if((*portInputRegister(Fport) & Fbit) == FstateMask)                          // Als er signaal is
    {
    // Als het een herhalend signaal is, dan is de kans groot dat we binnen hele korte tijd weer in deze
    // routine terugkomen en dan midden in de volgende herhaling terecht komen. Daarom wordt er in dit
    // geval gewacht totdat de pulsen voorbij zijn en we met het capturen van data beginnen na een korte 
    // rust tussen de signalen.Op deze wijze wordt het aantal zinloze captures teruggebracht.
    
    if(RawSignal.Time)                                                          //  Eerst een snelle check, want dit bevindt zich in een tijdkritisch deel...
      {    
      if(RawSignal.Repeats && (RawSignal.Time+SIGNAL_REPEAT_TIME)>millis())     // ...want deze check duurt enkele micro's langer!
        {
        // digitalWrite(PIN_WIRED_OUT_2,HIGH);                                  // DEBUG: Wired-2 hoog gedurende capturing data
        PulseLength=micros()+SIGNAL_TIMEOUT*1000;                               // Wachttijd
    
        while((RawSignal.Time+SIGNAL_REPEAT_TIME)>millis() && PulseLength>micros())
          if((*portInputRegister(Fport) & Fbit) == FstateMask)
             PulseLength=micros()+SIGNAL_TIMEOUT*1000;
        
        while((RawSignal.Time+SIGNAL_REPEAT_TIME)>millis() &&  (*portInputRegister(Fport) & Fbit) != FstateMask);
        // digitalWrite(PIN_WIRED_OUT_2,LOW);                                   // DEBUG: Wired-2 hoog gedurende capturing data
        }
      }
    RawCodeLength=1;                                                            // We starten bij 1, dit om legacy redenen. Vroeger had element 0 een speciaal doel.
    Ftoggle=false;
    maxloops = SIGNAL_TIMEOUT * LoopsPerMilli;  
  
    // digitalWrite(PIN_WIRED_OUT_2,HIGH);                                      // DEBUG:Wired-2 hoog gedurende capturing data
  
    do{                                                                         // lees de pulsen in microseconden en plaats deze in de tijdelijke buffer RawSignal
      numloops = 0;
      while(((*portInputRegister(Fport) & Fbit) == FstateMask) ^ Ftoggle)       // while() loop *A*
        if(numloops++ == maxloops)
          break;                                                                // timeout opgetreden
  
      PulseLength=((numloops + Overhead)* 1000) / LoopsPerMilli;                // Bevat nu de pulslengte in microseconden
      if(PulseLength<MIN_PULSE_LENGTH)
        break;
        
      Ftoggle=!Ftoggle;    
      RawSignal.Pulses[RawCodeLength++]=PulseLength/(unsigned long)(Settings.RawSignalSample); // sla op in de tabel RawSignal
      }
    while(RawCodeLength<RAW_BUFFER_SIZE && numloops<=maxloops);                 // Zolang nog ruimte in de buffer, geen timeout en geen stoorpuls
  
    // digitalWrite(PIN_WIRED_OUT_2,LOW);                                       //DEBUG:
  
    if(RawCodeLength>=MIN_RAW_PULSES)
      {
      ///digitalWrite(PIN_WIRED_OUT_4,HIGH);                                    // DEBUG: Een Spike op Wired-4 bevestigd een capture van een geldig signaal
      RawSignal.RepeatChecksum=false;                                           // 
      RawSignal.Repeats=0;                                                      // Op dit moment weten we nog niet het type signaal, maar de variabele niet ongedefinieerd laten.
      RawSignal.Multiply=Settings.RawSignalSample;                              // Ingestelde sample groote.
      RawSignal.Number=RawCodeLength-1;                                         // Aantal ontvangen tijden (pulsen *2)
      RawSignal.Pulses[RawSignal.Number]=0;                                     // Laatste element bevat de timeout. Niet relevant.
      RawSignal.Time=millis();
      // digitalWrite(PIN_WIRED_OUT_4,LOW);                                     // DEBUG:
      return true;
      }
    else
      RawSignal.Number=0;    
    }

  return false;
  }


boolean AnalyzeRawSignal(struct NodoEventStruct *E)
  {
  ClearEvent(E);
  boolean Result=false;
  
  if(RawSignal_2_ESP(E))                                                       // Is het een Nodo signaal?
    {
    // Als er een Nodo signaal is binnengekomen, dan weten we zeker dat er een Nodo in het landschap is die tijd nodig heeft om
    // weer terug te schakelen naar de ontvangstmode. Dit kost (helaas) enige tijd. Zorg er voor dat er gedurende deze tijd
    // even geen Nodo event wordt verzonden anders wordt deze vrijwel zeker gemist.
    HoldTransmission=millis()+ESP_TX_TO_RX_SWITCH_TIME;
    Result=true;
    }

  if(!Transmission_ESPOnly)
    {
    if(!Result && PluginCall(PLUGIN_RAWSIGNAL_IN,E,0))                          // Loop de devices langs. Indien een device dit nodig heeft, zal deze het rawsignal gebruiken en omzetten naar een geldig event.
      Result=true;
      
    if(!Result && RawSignal_2_32bit(E))                                         // als er geen enkel geldig signaaltype uit de pulsenreeks kon worden gedestilleerd, dan resteert niets anders dan deze weer te geven als een RawSignal.
      {
      if(Settings.RawSignalReceive==VALUE_ON                                    // Signaal event als de setting RawSignalReceive op On staat
        )
        Result=true;
      }
    }    

  return Result;
  }


/**********************************************************************************************\
 * Deze functie genereert uit een willekeurig gevulde RawSignal afkomstig van de meeste 
 * afstandsbedieningen een (vrijwel) unieke bit code.
 * Zowel breedte van de pulsen als de afstand tussen de pulsen worden in de berekening
 * meegenomen zodat deze functie geschikt is voor PWM, PDM en Bi-Pase modulatie.
 * LET OP: Het betreft een unieke hash-waarde zonder betekenis van waarde.
 \*********************************************************************************************/
boolean RawSignal_2_32bit(struct NodoEventStruct *event)
  {
  int x;
  unsigned int MinPulse=0xffff;
  unsigned int MinSpace=0xffff;
  unsigned long CodeM=0L;
  unsigned long CodeS=0L;

  // In enkele gevallen is uitzoeken van het RawSignal zinloos
  if(RawSignal.Number < MIN_RAW_PULSES) return false;  
  
  // zoek de kortste tijd (PULSE en SPACE). Start niet direct vanaf de eerste puls omdat we anders kans 
  // lopen een onvolledige startbit te pakken. Ook niet de laatste, want daar zit de niet bestaande
  // space van de stopbit in.
  for(x=5;x<RawSignal.Number-2;x+=2)
    {
    if(RawSignal.Pulses[x]  < MinPulse)MinPulse=RawSignal.Pulses[x]; // Zoek naar de kortste pulstijd.
    if(RawSignal.Pulses[x+1]< MinSpace)MinSpace=RawSignal.Pulses[x+1]; // Zoek naar de kortste spacetijd.
    }

  // De kortste pulsen zijn gevonden. Dan een 'opslag' zodat alle korte pulsen er royaal
  // onder vallen maar niet de lengte van een lange puls passeren.
  MinPulse+=(MinPulse*RAWSIGNAL_TOLERANCE)/100;
  MinSpace+=(MinSpace*RAWSIGNAL_TOLERANCE)/100;

  // Data kan zowel in de mark als de space zitten. Daarom pakken we beide voor data opbouw.
  for(x=3;x<=RawSignal.Number;x+=2)
    {
    CodeM = (CodeM<<1) | (RawSignal.Pulses[x]   > MinPulse);
    CodeS = (CodeS<<1) | (RawSignal.Pulses[x+1] > MinSpace);    
    }

  // Data kan zowel in de mark als de space zitten. We nemen de grootste waarde voor de data.
  if(CodeM > CodeS)  
    event->Par2=CodeM;
  else
    event->Par2=CodeS;

  event->SourceUnit=0;  
  event->DestinationUnit=0;
  event->Type=ESP_TYPE_RAWSIGNAL;
  event->Command=EVENT_RAWSIGNAL;
  event->Par1=0;

  RawSignal.Repeats        = 1;                                                 // het is een herhalend signaal. Bij ontvangst herhalingen onderdukken 
  RawSignal.RepeatChecksum = false;
    
  return true;
  }


/*********************************************************************************************\
 * Deze routine berekent de uit een RawSignal een NODO code
 * Geeft een false retour als geen geldig NODO signaal
 \*********************************************************************************************/
boolean RawSignal_2_ESP(struct NodoEventStruct *Event)
  {
  byte b,x,y,z;

  if(RawSignal.Number!=16*sizeof(struct DataBlockStruct)+2)                     // Per byte twee posities + startbit.
    return false;
    
  struct DataBlockStruct DataBlock;
  byte *B=(byte*)&DataBlock;                                                    // B wijst naar de eerste byte van de struct
  z=3;                                                                          // RawSignal pulse teller: 0=niet gebruiktaantal, 1=startpuls, 2=space na startpuls, 3=1e pulslengte. Dus start loop met drie.

  for(x=0;x<sizeof(struct DataBlockStruct);x++)                                 // vul alle bytes van de struct 
    {
    b=0;
    for(y=0;y<=7;y++)                                                           // vul alle bits binnen een byte
      {
      if((RawSignal.Pulses[z]*RawSignal.Multiply)>ESP_PULSE_MID)      
        b|=1<<y;                                                                // LSB in signaal wordt als eerste verzonden
      z+=2;
      }
    *(B+x)=b;
    }

  if(DataBlock.SourceUnit>>5!=HOME_ESP)
    return false;

  RawSignal.Repeats    = 0;                                                     // het is geen herhalend signaal. Bij ontvangst hoeven herhalingen dus niet onderdrukt te worden.
  Event->SourceUnit=DataBlock.SourceUnit&0x1F;                                  // Maskeer de bits van het Home adres.
  Event->DestinationUnit=DataBlock.DestinationUnit;
  Event->Flags=DataBlock.Flags;
  Event->Type=DataBlock.Type;
  Event->Command=DataBlock.Command;
  Event->Par1=DataBlock.Par1;
  Event->Par2=DataBlock.Par2;
  Event->Version=DataBlock.Version;
  Event->Checksum=DataBlock.Checksum;

  if(Checksum(Event))
    return true;

  return false; 
  }

