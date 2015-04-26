// some fixes here:
#include <stdio.h>

// workaround for strcasecmp, issue with lib of header files.??????
int strcasecmp(const char * str1, const char * str2) {
    int d = 0;
    while(1) {
        int c1 = tolower(*str1++);
        int c2 = tolower(*str2++);
        if(((d = c1 - c2) != 0) || (c2 == '\0')) {
            break;
        }
    }
    return d;
}

// workaround for float to string fix
long getDecimal(float val)
{
 int intPart = int(val);
 long decPart = 1000*(val-intPart);
 if(decPart>0)return(decPart);
 else if(decPart<0)return((-1)*decPart);
 else if(decPart=0)return(00);
}

/*******************************************************************************************************\
* Deze funktie parsed een string zoals die door de gebruiker wordt opgegeven. De commando's
* worden gechecked op geldigheid en ter uitvoering doorgegeven.
* Commando's en events worden gescheiden met een puntkomma.
\*******************************************************************************************************/
int ExecuteLine(char *Line, byte Port)
{
  ProcessingStatus(true);

  char *Command = (char*)malloc(INPUT_COMMAND_SIZE);
  char *TmpStr1 = (char*)malloc(INPUT_COMMAND_SIZE);
  char *TmpStr2 = (char*)malloc(INPUT_LINE_SIZE);
  int CommandPos;
  int LinePos;
  int w, x, y;
  int EventlistWriteLine = 0;
  byte error = 0, State_EventlistWrite = 0;
  unsigned long a;
  struct NodoEventStruct EventToExecute, TempEvent;

  Transmission_SendToUnit = Transmission_SendToAll;                             // Als de SendTo wel/niet permanent, dan unitnummer overnemen of uitschakelen.

  if(Transmission_SendToAll)                                                    // Als SendTo wel permanent ingeschakeld, dan queue leegmaken anders eventuele oude queue inhoud meeverzonden met SendTo
    QueuePosition=0;

  if (false)                                                        // verwerking van commando's is door gebruiker tijdelijk geblokkeerd door FileWrite commando
  {
  }
  else
  {
    CommandPos = 0;
    LinePos = 0;
    int LineLength = strlen(Line);

    while (LinePos <= LineLength && error == 0)
    {
      char LineChar = Line[LinePos];

      if (LineChar == '!')                                                      // Comment teken. hierna verder niets meer doen.
      {
        LinePos = LineLength + 1;                                               // ga direct naar einde van de regel.
      }

      // Commando compleet als puntkomma (scheidt opdrachten) of einde string.
      if ((LineChar == '!' || LineChar == ';' || LineChar == 0) && CommandPos > 0 || CommandPos == (INPUT_COMMAND_SIZE - 1))
      {
        Command[CommandPos] = 0;
        CommandPos = 0;

        error = Str2Event(Command, &EventToExecute);                            // Commando's in tekst format moeten worden omgezet naar een Nodo event.
        EventToExecute.Port = Port;

        // Enkele comando's kennen een afwijkende behandeling. Dit zijn commando's die niet uitgevoerd
        // kunnen worden door ExecuteCommand() omdat we in de Nodo geen strings kunnen doorgeven in de
        // eventstruct, Deze commando's separaat parsen en direct hier uitvoeren.
        if (error == MESSAGE_UNKNOWN_COMMAND)
        {
          error = 0;                                                            // nieuwe poging.

          ClearEvent(&EventToExecute);
          GetArgv(Command, TmpStr1, 1);
          EventToExecute.Command = str2cmd(TmpStr1);                            // Bouw een nieuw event op.

          if (GetArgv(Command, TmpStr1, 2))                                     // Haal Par1 uit het commando.
          {
            EventToExecute.Par1 = str2cmd(TmpStr1);
            if (!EventToExecute.Par1)
              EventToExecute.Par1 = str2int(TmpStr1);
          }

          if (GetArgv(Command, TmpStr1, 3))                                     // Haal Par2 uit het commando.
          {
            EventToExecute.Par2 = str2cmd(TmpStr1);
            if (!EventToExecute.Par2)
              EventToExecute.Par2 = str2int(TmpStr1);
          }
          x = EventToExecute.Command;
          EventToExecute.Command = 0;                                           // Toegevoegd voor de werking van Sendto.

          switch (x)
          {
            case CMD_EVENTLIST_WRITE:
              EventToExecute.Type = ESP_TYPE_COMMAND;
              EventToExecute.Command = CMD_EVENTLIST_WRITE;
              if (EventToExecute.Par1 <= EventlistMax)
              {
                EventlistWriteLine = EventToExecute.Par1;
                State_EventlistWrite = 1;
              }
              else
                error = MESSAGE_INVALID_PARAMETER;
              EventToExecute.Command = 0;
              break;

            case CMD_SERVER_IP:
              EventToExecute.Type=ESP_TYPE_COMMAND;
              if(GetArgv(Command,TmpStr1,2))
                if(!str2ip(TmpStr1,Settings.Server_IP))
                  error=MESSAGE_INVALID_PARAMETER;
              break;

            case CMD_SERVER_PORT:
              EventToExecute.Type=ESP_TYPE_COMMAND;
              if(GetArgv(Command,TmpStr1,2))
                Settings.ServerPort=str2int(TmpStr1);
              break;

            case CMD_WIFI_SSID:
              {
                EventToExecute.Type = ESP_TYPE_COMMAND;
                TmpStr1[0] = 0;
                GetArgv(Command, TmpStr1, 2);
                TmpStr1[25] = 0;                                                  // voor geval de string te lang is.
                strcpy(Settings.WifiSSID,TmpStr1);
                break;
              }

            case CMD_WIFI_KEY:
              {
                EventToExecute.Type = ESP_TYPE_COMMAND;
                TmpStr1[0] = 0;
                GetArgv(Command, TmpStr1, 2);
                TmpStr1[25] = 0;                                                  // voor geval de string te lang is.
                strcpy(Settings.WifiKey,TmpStr1);
                break;
              }

            default:
              {
                // Ingevoerde commando is niet gevonden.
                // Loop de devices langs om te checken if er een hit is. Zo ja, dan de struct
                // met de juiste waarden gevuld. Is er geen hit, dan keert PluginCall() terug met een false.
                // in dat geval kijken of er een commando op SDCard staat
                if (!PluginCall(PLUGIN_MMI_IN, &EventToExecute, Command))
                {
                  // Als het geen regulier commando was EN geen commando met afwijkende MMI en geen Plugin en geen alias, dan kijken of file op SDCard staat)
                  // Voer bestand uit en verwerking afbreken als er een foutmelding is.
                  error = MESSAGE_UNKNOWN_COMMAND;

                  // als script niet te openen, dan is het ingevoerde commando ongeldig.

                  EventToExecute.Command = 0;
                }

                if (error)
                {
                  //strcpy(TmpStr2, Command);
                  //strcat(TmpStr2, "?");
                  sprintf(TmpStr2,"%s",Command);
                  sprintf(TmpStr2,"%s%s",TmpStr2,"?");
                  Serial.println(TmpStr2);
                }
              }
          }// switch(command...@2
        }

        if (EventToExecute.Command && error == 0)
        {
          // Er kunnen zich twee situaties voordoen:
          //
          // A: Event is voor deze Nodo en moet gewoon worden uitgevoerd;
          // B: SendTo is actief. Event moet worden verzonden naar een andere Nodo. Hier wordt de Queue voor gebruikt.

          if (State_EventlistWrite == 0) // Gewoon uitvoeren
          {
            if (1 == 1)
            {
              EventToExecute.Port = Port;
              EventToExecute.Direction = VALUE_DIRECTION_INPUT;
              error = ProcessEvent(&EventToExecute);
            }
            else
            {
              if (EventToExecute.Command)                                       // geen lege events in de queue plaatsen
              {
                QueueAdd(&EventToExecute);                                      // Plaats in queue voor latere verzending.
              }
            }
            continue;
          }

          if (State_EventlistWrite == 2)
          {
            UndoNewNodo();// Status NewNodo verwijderen indien van toepassing
            if (!Eventlist_Write(EventlistWriteLine, &TempEvent, &EventToExecute))
            {
              RaiseMessage(MESSAGE_EVENTLIST_FAILED, EventlistWriteLine);
              break;
            }
            State_EventlistWrite = 0;
            continue;
          }

          if (State_EventlistWrite == 1)
          {
            TempEvent = EventToExecute; // TempEvent = >Event< dat moet worden weggeschreven in de eventlist;
            State_EventlistWrite = 2;
          }
        }
      }// if(LineChar.

      // Tekens toevoegen aan commando zolang er nog ruimte is in de string
      if (LineChar != ';' && CommandPos < (INPUT_COMMAND_SIZE - 1) )
        Command[CommandPos++] = LineChar;
      LinePos++;
    }// while(LinePos...

  }// einde regel behandeling

  free(TmpStr2);
  free(TmpStr1);
  free(Command);

  // Verwerk eventuele events die in de queue zijn geplaatst.
  if (error == 0)
    QueueProcess();

  return error;
}


/*********************************************************************************************\
 * Print een event naar de opgegeven poort. Dit kan zijn:
 *
 * VALUE_ALL, VALUE_SOURCE_SERIAL, VALUE_SOURCE_TELNET, VALUE_SOURCE_FILE
 *
 \*********************************************************************************************/
void PrintEvent(struct NodoEventStruct *Event, byte Port)
{

  // Systeem events niet weergeven.
  if (Event->Type == ESP_TYPE_SYSTEM || Event->Type == 0)
    return;

//  char* StringToPrint = (char*)malloc(128);
//  char* TmpStr = (char*)malloc(INPUT_LINE_SIZE);
  char StringToPrint[128];
  char TmpStr[128];

  StringToPrint[0] = 0; // als start een lege string

  // Direction
  if (Event->Direction)
  {
    // todo strcat(StringToPrint, cmd2str(Event->Direction));
    sprintf(StringToPrint,"%s%s",StringToPrint,cmd2str(Event->Direction));
    // todo strcat(StringToPrint, "=");
    sprintf(StringToPrint,"%s%s",StringToPrint,"=");

    // Poort
    // todo strcat(StringToPrint, cmd2str(Event->Port));
    sprintf(StringToPrint,"%s%s",StringToPrint,cmd2str(Event->Port));

    if (Event->Port == VALUE_SOURCE_EVENTLIST)
    {
      // print de nesting diepte van de eventlist en de regel uit de eventlist.
      // todo strcat(StringToPrint, "(");
      sprintf(StringToPrint,"%s%s",StringToPrint,"(");
      // todo strcat(StringToPrint, int2str(ExecutionDepth - 1));
      sprintf(StringToPrint,"%s%s",StringToPrint,int2str(ExecutionDepth - 1));
      // todo strcat(StringToPrint, ".");
      sprintf(StringToPrint,"%s%s",StringToPrint,".");
      // todo strcat(StringToPrint, int2str(ExecutionLine));
      sprintf(StringToPrint,"%s%s",StringToPrint,int2str(ExecutionLine));
      //todo strcat(StringToPrint, ")");
      sprintf(StringToPrint,"%s%s",StringToPrint,")");
    }
    // todo strcat(StringToPrint, "; ");
    sprintf(StringToPrint,"%s%s",StringToPrint,"; ");
  }

  // Unit
  // todo strcat(StringToPrint, cmd2str(VALUE_UNIT));
  sprintf(StringToPrint,"%s%s",StringToPrint,cmd2str(VALUE_UNIT));

  // todo strcat(StringToPrint, "=");
  sprintf(StringToPrint,"%s%s",StringToPrint,"=");
  if (Event->Direction == VALUE_DIRECTION_OUTPUT)
  // todo strcat(StringToPrint, int2str(Event->DestinationUnit));
    sprintf(StringToPrint,"%s%s",StringToPrint,int2str(Event->DestinationUnit));
  else
    // todo strcat(StringToPrint, int2str(Event->SourceUnit));
    sprintf(StringToPrint,"%s%s",StringToPrint,int2str(Event->SourceUnit));

  // Event
  // todo strcat(StringToPrint, "; ");
  sprintf(StringToPrint,"%s%s",StringToPrint,"; ");
  // todo strcat(StringToPrint, Text_14);
  sprintf(StringToPrint,"%s%s",StringToPrint,Text_14);

  Event2str(Event, TmpStr);

  //todo  strcat(StringToPrint, TmpStr);
  sprintf(StringToPrint,"%s%s",StringToPrint,TmpStr);

   Serial.println(StringToPrint);  // stuur de regel naar Serial en/of naar Ethernet

  //free(TmpStr);
  //free(StringToPrint);
}

/**********************************************************************************************\
 * Print de welkomsttekst van de Nodo.
 \*********************************************************************************************/
const char Text_welcome[] = "ESP Connexio V0.1, R%03d, Home=%d, ThisUnit=%d";
void PrintWelcome(void)
{
  char *TempString = (char*)malloc(128);

  // Print Welkomsttekst
  Serial.println(Text_22);
  Serial.println(Text_01);
  Serial.println(Text_02);
  Serial.println(Text_03);
  Serial.println(Text_04);

  sprintf(TempString,Text_welcome, ESP_BUILD, HOME_ESP, Settings.Unit);

  Serial.println(TempString);
  Serial.println(Text_22);
  free(TempString);
}

#define PAR1_INT           1
#define PAR1_TEXT          2
#define PAR1_MESSAGE       3
#define PAR2_INT           4
#define PAR2_TEXT          5
#define PAR2_FLOAT         6
#define PAR2_INT_HEX       7
#define PAR2_DIM           8
#define PAR2_WDAY          9
#define PAR2_TIME         10
#define PAR2_DATE         11
#define PAR2_ALARMENABLED 12
#define PAR2_INT8         13
#define PAR3_INT          14
#define PAR4_INT          15
#define PAR5_INT          16
#define PAR3_TEXT         17

void Event2str(struct NodoEventStruct *Event, char* EventString)
{
  int x;
  EventString[0] = 0;
  char* str = (char*)malloc(INPUT_COMMAND_SIZE);
  str[0] = 0; // als er geen gevonden wordt, dan is de string leeg

  // Er kunnen een aantal parameters worden weergegeven. In een kleine tabel wordt aangegeven op welke wijze de parameters aan de gebruiker
  // moeten worden getoond. Het is niet per defiitie zo dat de interne Par1, Par2 en Par3 ook dezelfe parameters zijn die aan de gebruiker
  // worden getoond.
  byte ParameterToView[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  // Plugins hebben een eigen MMI, Roep het device aan voor verwerking van de parameter PLUGIN_MMI_OUT.
  // zoek het device op in de devices tabel en laat de string vullen. Als het niet gelukt is om de string te
  // vullen dan behandelen als een regulier event/commando
  if (Event->Type == ESP_TYPE_PLUGIN_COMMAND || Event->Type == ESP_TYPE_PLUGIN_EVENT)
  {
    strcpy(EventString, "?");
    PluginCall(PLUGIN_MMI_OUT, Event, EventString);
  }

  if (EventString[0] == 0)
  {
    // todo strcpy(EventString, cmd2str(Event->Command));
    sprintf(EventString,"%s%s",EventString,cmd2str(Event->Command));
    // todo strcat(EventString, " ");
    sprintf(EventString,"%s%s",EventString," ");

    switch (Event->Command)
    {
      // Par1 en Par2 samengesteld voor weergave van COMMAND <nummer> , <analoge waarde>
      case CMD_BREAK_ON_VAR_EQU:
      case CMD_BREAK_ON_VAR_LESS:
      case CMD_BREAK_ON_VAR_MORE:
      case CMD_BREAK_ON_VAR_NEQU:
      case CMD_VARIABLE_SET:
      case CMD_VARIABLE_INC:
      case CMD_VARIABLE_DEC:
      case CMD_VARIABLE_MULTIPLY:
      case EVENT_VARIABLE:
        ParameterToView[0] = PAR1_INT;
        ParameterToView[1] = PAR2_FLOAT;
        break;

      // Par2 als hex, geen andere parameters.
      case VALUE_HWCONFIG:
        ParameterToView[0] = PAR2_INT_HEX;
        break;

      case CMD_WIRED_SMITTTRIGGER:
      case CMD_WIRED_THRESHOLD:
      case VALUE_WIRED_ANALOG:
      case CMD_DOMOTICZ_GET:
      case CMD_DOMOTICZ_SET:
        ParameterToView[0] = PAR1_INT;
        ParameterToView[1] = PAR2_INT;
        break;

      // Par2 als decimale int.
      case VALUE_BUILD:
      case VALUE_FREEMEM:
        ParameterToView[0] = PAR2_INT;
        break;

      // Par1 als waarde en par2 als tekst
      case CMD_VARIABLE_SEND:
      case CMD_WIRED_PULLUP:
      case CMD_WIRED_OUT:
      case VALUE_UNIT:
      case EVENT_WIRED_IN:
        ParameterToView[0] = PAR1_INT;
        ParameterToView[1] = PAR2_TEXT;
        break;

      // Par1 als tekst en par2 als tekst
      case CMD_OUTPUT:
        ParameterToView[0] = PAR1_TEXT;
        ParameterToView[1] = PAR2_TEXT;
        break;

      case CMD_VARIABLE_GET:
        ParameterToView[0] = PAR1_INT;
        ParameterToView[1] = PAR2_INT8;
        ParameterToView[2] = PAR3_INT;

        break;

      case EVENT_WILDCARD:
        ParameterToView[0] = PAR1_TEXT;
        ParameterToView[1] = PAR2_TEXT;
        ParameterToView[2] = PAR3_INT;
        break;

      // Par1 als tekst en par2 als getal
      case CMD_STATUS:
        ParameterToView[0] = PAR1_TEXT;
        ParameterToView[1] = PAR2_INT;
        break;

      // Par1 als tekst en par2 niet
      case CMD_SEND_EVENT:
      case CMD_DEBUG:
        ParameterToView[0] = PAR1_TEXT;
        break;

      // Par1 als waarde en par2 niet
      case CMD_EVENTLIST_SHOW:
      case CMD_EVENTLIST_ERASE:
      case EVENT_TIMER:
      case EVENT_BOOT:
      case CMD_DELAY:
      case VALUE_SOURCE_PLUGIN:
      case EVENT_NEWNODO:
      case CMD_VARIABLE_SAVE:
      case CMD_VARIABLE_TOGGLE:
      case CMD_UNIT_SET:
      case CMD_VARIABLE_PULSE_TIME:
      case CMD_VARIABLE_PULSE_COUNT:
        ParameterToView[0] = PAR1_INT;
        break;

      // Par1 ls int, Par2 als tekst, Par3(uit Par2) als tekst.
      case CMD_SENDTO:
        if (Event->Par1 == 0)
        {
          Event->Par1 = VALUE_OFF;
          ParameterToView[0] = PAR1_TEXT;
        }
        else
        {
          ParameterToView[0] = PAR1_INT;
          ParameterToView[1] = PAR2_TEXT;
          ParameterToView[3] = PAR3_TEXT;
        }
        break;

      // geen parameters.
      case CMD_REBOOT:
      case CMD_RESET:
      case CMD_STOP:
      case CMD_SETTINGS_SAVE:
        break;

      // Twee getallen en een aanvullende tekst
      case EVENT_MESSAGE:
        ParameterToView[0] = PAR1_MESSAGE;
        break;

      // Par1 als waarde en par2 als waarde
      default:
        ParameterToView[0] = PAR1_INT;
        ParameterToView[1] = PAR2_INT;
    }

    for (byte p = 0; p <= 3; p++)
    {
      switch (ParameterToView[p])
      {
        case PAR1_INT:
          // todo strcat(EventString, int2str(Event->Par1));
          sprintf(EventString,"%s%s",EventString,int2str(Event->Par1));
          break;

        case PAR1_TEXT:
          // todo strcat(EventString, cmd2str(Event->Par1));
          sprintf(EventString,"%s%s",EventString,cmd2str(Event->Par1));
          break;

        case PAR1_MESSAGE:
          // todo strcat(EventString, int2str(Event->Par1));
          sprintf(EventString,"%s%s",EventString,int2str(Event->Par1));
          // todo strcat(EventString, ",");
          sprintf(EventString,"%s%s",EventString,",");
          // todo strcat(EventString, int2str(Event->Par2));
          sprintf(EventString,"%s%s",EventString,int2str(Event->Par2));

          if (Event->Par1 <= MESSAGE_MAX)
          {
            // todo strcat(EventString, ": ");
            sprintf(EventString,"%s%s",EventString,": ");
            // todo strcpy(str, MessageText_tabel[Event->Par1]);
            sprintf(str,"%s",MessageText_tabel[Event->Par1]);
            // todo strcat(EventString, str);
            sprintf(EventString,"%s%s",EventString,str);
          }
          break;

        case PAR2_INT:
          // todo strcat(EventString, int2str(Event->Par2));
          sprintf(EventString,"%s%s",EventString,int2str(Event->Par2));
          break;

        case PAR2_INT8:
          strcat(EventString, int2str(Event->Par2 & 0xff));
          break;

        case PAR3_INT:
          strcat(EventString, int2str((Event->Par2 >> 8) & 0xff));
          break;

        case PAR4_INT:
          strcat(EventString, int2str((Event->Par2 >> 16) & 0xff));
          break;

        case PAR5_INT:
          strcat(EventString, int2str((Event->Par2 >> 24) & 0xff));
          break;

        case PAR2_TEXT:
          // todo strcat(EventString, cmd2str(Event->Par2 & 0xff));
          sprintf(EventString,"%s%s",EventString,cmd2str(Event->Par2));
          break;

        case PAR3_TEXT:
          strcat(EventString, cmd2str((Event->Par2 >> 8) & 0xff));
          break;

        case PAR2_DIM:
          if (Event->Par2 == VALUE_OFF || Event->Par2 == VALUE_ON)
            strcat(EventString, cmd2str(Event->Par2)); // Print 'On' of 'Off'
          else
            strcat(EventString, int2str(Event->Par2));
          break;

        case PAR2_FLOAT:
          {
          // Een float en een unsigned long zijn beide 4bytes groot. We gebruiken ruimte van Par2 om een float in op te slaan
          float f;
          memcpy(&f, &Event->Par2, 4);

          // Als de float (bij benadering) .00 is, dan alleen de cijfers voor de komma weergeven.
          x = 0;
          if ((((int)(f * (1000)) - ((int)f) * (1000))) != 0)
            x = 3;
            
          // todo dtostrf(f, 0, x,EventString+strlen(EventString));

          // workaround 1, also does not work!:
          //char temp[16];
          //sprintf(temp,"%F",f);
          //sprintf(EventString,"%s%s",EventString,temp);

          // workaround 2: works ...
          char temp[16];
          String stringVal = ""; 
          stringVal+=String(int(f))+ "."+String(getDecimal(f));
          char charVal[stringVal.length()+1];
          stringVal.toCharArray(charVal,stringVal.length()+1);
          sprintf(EventString,"%s%s",EventString,charVal);
          }
          break;

        case PAR2_INT_HEX:
          strcat(EventString, int2strhex(Event->Par2));
          break;

      }

      if (ParameterToView[p + 1] != 0) // komt er nog een vervolgparameter, dan een komma als scheidingsteken.
        // todo strcat(EventString, ",");
        sprintf(EventString,"%s%s",EventString,",");
    }
  }
  free(str);
}


/*******************************************************************************************************\
* Deze funktie parsed een string zoals die door de gebruiker wordt opgegeven. Commando, Par1 en Par2
* van struct ResultEvent worden gevuld.
\*******************************************************************************************************/
boolean Str2Event(char *Command, struct NodoEventStruct *ResultEvent)
{
  char *TmpStr1 = (char*)malloc(INPUT_COMMAND_SIZE);
  int w, x, y;
  byte error = 0;
  unsigned long a;
  //struct NodoEventStruct EventToExecute,TempEvent;

  // Bouw een nieuw event op.
  ClearEvent(ResultEvent);
  GetArgv(Command, TmpStr1, 1);
  ResultEvent->Command = str2cmd(TmpStr1);

  // Haal Par1 uit het commando.
  if (GetArgv(Command, TmpStr1, 2))
  {
    ResultEvent->Par1 = str2cmd(TmpStr1);
    if (!ResultEvent->Par1)
      ResultEvent->Par1 = str2int(TmpStr1);
  }

  // Haal Par2 uit het commando.
  if (GetArgv(Command, TmpStr1, 3))
  {
    ResultEvent->Par2 = str2cmd(TmpStr1);
    if (!ResultEvent->Par2)
      ResultEvent->Par2 = str2int(TmpStr1);
  }

  switch (ResultEvent->Command)
  {
    //test; geen, altijd goed
    case EVENT_USEREVENT:
      ResultEvent->Type = ESP_TYPE_EVENT;
      break;

    // altijd goed
    case CMD_EVENTLIST_SHOW:
    case CMD_EVENTLIST_ERASE:
    case CMD_STOP:
    case CMD_RESET:
    case CMD_REBOOT:
    case CMD_SETTINGS_SAVE:
    case CMD_STATUS:
    case CMD_DELAY:
    case CMD_SOUND:
    case CMD_SEND_USEREVENT:
    case CMD_DOMOTICZ_GET:
    case CMD_DOMOTICZ_SET:
    case CMD_WIFI_CONNECT:
    case CMD_WIFI_DISCONNECT:
    case CMD_WIFI_SCAN:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      break;

    case EVENT_MESSAGE:
      ResultEvent->Type = ESP_TYPE_EVENT;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > MESSAGE_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_TIMER_SET:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 == 0 || ResultEvent->Par1 > TIMER_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_TIMER_SET_VARIABLE:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 > TIMER_MAX || ResultEvent->Par2 < 1 || ResultEvent->Par2 > USER_VARIABLES_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case EVENT_NEWNODO:
    case EVENT_BOOT:
      ResultEvent->Type = ESP_TYPE_EVENT;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > UNIT_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_UNIT_SET:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > UNIT_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_ANALYSE_SETTINGS:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > 50)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_VARIABLE_PULSE_TIME:
    case CMD_VARIABLE_PULSE_COUNT:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > USER_VARIABLES_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_BREAK_ON_VAR_LESS_VAR:
    case CMD_BREAK_ON_VAR_MORE_VAR:
    case CMD_VARIABLE_VARIABLE:
    case CMD_VARIABLE_ADD_VARIABLE:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > USER_VARIABLES_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      if (ResultEvent->Par2 < 1 || ResultEvent->Par2 > USER_VARIABLES_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_VARIABLE_SET_WIRED_ANALOG:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > USER_VARIABLES_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      if (ResultEvent->Par2 < 1 || ResultEvent->Par2 > WIRED_PORTS)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_TIMER_RANDOM:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > TIMER_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case EVENT_TIMER:
      ResultEvent->Type = ESP_TYPE_EVENT;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > TIMER_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    // test:ResultEvent->Par1 binnen bereik maximaal beschikbare wired poorten.
    case EVENT_WIRED_IN:
      ResultEvent->Type = ESP_TYPE_EVENT;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > WIRED_PORTS)
        error = MESSAGE_INVALID_PARAMETER;
      if (ResultEvent->Par2 != VALUE_ON && ResultEvent->Par2 != VALUE_OFF)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_WIRED_OUT:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 > WIRED_PORTS)
        error = MESSAGE_INVALID_PARAMETER;
      if (ResultEvent->Par2 != VALUE_ON && ResultEvent->Par2 != VALUE_OFF)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_WIRED_PULLUP:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > WIRED_PORTS)
        error = MESSAGE_INVALID_PARAMETER;
      if (ResultEvent->Par2 != VALUE_ON && ResultEvent->Par2 != VALUE_OFF)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_SEND_EVENT:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par2 == 0);
      ResultEvent->Par2 = VALUE_ALL;
      switch (ResultEvent->Par1)
      {
        case VALUE_ALL:
        case VALUE_SOURCE_I2C:
        case VALUE_SOURCE_IR:
        case VALUE_SOURCE_RF:
        case VALUE_SOURCE_HTTP:
          break;
        default:
          error = MESSAGE_INVALID_PARAMETER;
      }
      break;

    case CMD_VARIABLE_SEND:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 > USER_VARIABLES_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      switch (ResultEvent->Par2)
      {
        case 0:
          ResultEvent->Par2 = VALUE_ALL;
          break;
        case VALUE_ALL:
        case VALUE_SOURCE_I2C:
        case VALUE_SOURCE_IR:
        case VALUE_SOURCE_RF:
        case VALUE_SOURCE_HTTP:
          break;

        default:
          error = MESSAGE_INVALID_PARAMETER;
      }
      break;

    case EVENT_WILDCARD:
      ResultEvent->Type = ESP_TYPE_EVENT;
      switch (ResultEvent->Par1)
      {
        case VALUE_ALL:
        case VALUE_SOURCE_CLOCK:
        case VALUE_SOURCE_SYSTEM:
        case VALUE_SOURCE_EVENTLIST:
        case VALUE_SOURCE_WIRED:
        case VALUE_SOURCE_I2C:
        case VALUE_SOURCE_IR:
        case VALUE_SOURCE_RF:
        case VALUE_SOURCE_SERIAL:
        case VALUE_SOURCE_PLUGIN:
          break;
        default:
          error = MESSAGE_INVALID_PARAMETER;
      }

      switch (ResultEvent->Par2)
      {
        case VALUE_ALL:
        case EVENT_USEREVENT:
        case EVENT_TIMER:
        case EVENT_WIRED_IN:
        case EVENT_VARIABLE:
        case EVENT_NEWNODO:
        case EVENT_MESSAGE:
        case EVENT_BOOT:
          break;
        default:
          error = MESSAGE_INVALID_PARAMETER;
      }

      if (GetArgv(Command, TmpStr1, 4))
        ResultEvent->Par2 |= (str2int(TmpStr1) << 8);

      break;

    case CMD_OUTPUT:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 != VALUE_SOURCE_I2C && ResultEvent->Par1 != VALUE_SOURCE_IR && ResultEvent->Par1 != VALUE_SOURCE_RF && ResultEvent->Par1 != VALUE_SOURCE_HTTP)
        error = MESSAGE_INVALID_PARAMETER;
      if (ResultEvent->Par2 != VALUE_OFF && ResultEvent->Par2 != VALUE_ON && (ResultEvent->Par2 != VALUE_ALL && ResultEvent->Par1 != VALUE_SOURCE_HTTP))
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_VARIABLE_TOGGLE:
    case CMD_VARIABLE_SAVE:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 > USER_VARIABLES_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_VARIABLE_SET:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 > USER_VARIABLES_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      else if (GetArgv(Command, TmpStr1, 3)) // waarde van de variabele
        ResultEvent->Par2 = float2ul(atof(TmpStr1));
      else
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_VARIABLE_GET: // VariableGet <Variabele>, <Unit>, <VariabeleBron>
      ResultEvent->Type = ESP_TYPE_COMMAND;
      error = MESSAGE_INVALID_PARAMETER;

      if (ResultEvent->Par1 > 0 && ResultEvent->Par1 <= USER_VARIABLES_MAX)
      {
        if (ResultEvent->Par2 > 0 && ResultEvent->Par2 <= UNIT_MAX)
        {
          if (GetArgv(Command, TmpStr1, 4)) // VariabeleBron
          {
            x = str2int(TmpStr1);
            if (x > 0 && x <= USER_VARIABLES_MAX)
            {
              ResultEvent->Par2 |= (x << 8);
              error = 0;
            }
          }
        }
      }
      break;

    case CMD_BREAK_ON_VAR_EQU:
    case CMD_BREAK_ON_VAR_LESS:
    case CMD_BREAK_ON_VAR_MORE:
    case CMD_BREAK_ON_VAR_NEQU:
    case CMD_VARIABLE_DEC:
    case CMD_VARIABLE_INC:
    case CMD_VARIABLE_MULTIPLY:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > USER_VARIABLES_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      else if (GetArgv(Command, TmpStr1, 3)) // waarde van de variabele
        ResultEvent->Par2 = float2ul(atof(TmpStr1));
      else
        error = MESSAGE_INVALID_PARAMETER;
      break;


    case EVENT_VARIABLE:
      ResultEvent->Type = ESP_TYPE_EVENT;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > USER_VARIABLES_MAX)
        error = MESSAGE_INVALID_PARAMETER;
      else if (GetArgv(Command, TmpStr1, 3)) // waarde van de variabele
        ResultEvent->Par2 = float2ul(atof(TmpStr1));
      else
        error = MESSAGE_INVALID_PARAMETER;
      break;

    case VALUE_WIRED_ANALOG:
    case CMD_WIRED_THRESHOLD:
    case CMD_WIRED_SMITTTRIGGER:
      ResultEvent->Type = ESP_TYPE_COMMAND;
      if (ResultEvent->Par1 < 1 || ResultEvent->Par1 > WIRED_PORTS)
        error = MESSAGE_INVALID_PARAMETER;
      else if (GetArgv(Command, TmpStr1, 3))
        ResultEvent->Par2 = str2int(TmpStr1);
      break;

    default:
      {
        error = MESSAGE_UNKNOWN_COMMAND;
      }
  }

  free(TmpStr1);
  return error;
}

