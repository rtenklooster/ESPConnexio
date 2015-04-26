/*********************************************************************************************\
 * Interrupt handler for RF messages
\*********************************************************************************************/
void RF_ISR()
{
  //if (RawSignal.Available) return;

  byte state=0;
  static byte counter=0;
  static unsigned long TimeStamp=0;
  static unsigned long TimeStampStart=0;
  unsigned long TimeElapsed=0;

  state = digitalRead(PIN_RF_RX_DATA);
  if ((TimeStampStart==0) && (state==0)) return;
  
  if (TimeStampStart==0) // First bit in sequence, only record timestamp
    {
      TimeStamp=micros();
      TimeStampStart = micros();
      RawSignal.Number=0;
      return;
    }

  TimeElapsed=micros()-TimeStamp;
  TimeStamp = micros();

  if (TimeElapsed > MIN_PULSE_LENGTH)
    {
      if (counter==0) counter=3; else counter++;
      RawSignal.Pulses[counter]=TimeElapsed/RawSignal.Multiply;
    }
  else
    {
      counter=0;
      TimeStampStart=0;
    }
    
  if (TimeElapsed > 5000 ) //nodo SIGNAL_TIMEOUT_RF is in mSec, we need it here in uSec
    {
      if (counter > MIN_RAW_PULSES)
        {
          NodoEventStruct E;
          ClearEvent(&E);
          RawSignal.Number=counter;
          if( (PluginCall(PLUGIN_RAWSIGNAL_IN,&E,0)) || RawSignal_2_ESP(&E) )
            {
              E.Port=VALUE_SOURCE_RF;
              QueueAdd(&E);
              RawSignal.Available = true;
              TimeStampStart=0;
              counter=0;
            }
          else
            {
              counter=0;
              TimeStampStart=0;
            }
        }
      else
        {
          counter=0;
          TimeStampStart=0;
        }
    }
    
  if (counter > 250)
   {
     counter=0;
     TimeStampStart=0;
   }
}

void debug(void)
{
    for (byte x=1; x < RawSignal.Number; x++)
      {
        Serial.print(RawSignal.Pulses[x]*RawSignal.Multiply);
        Serial.print(',');
      }
      Serial.println();
}

