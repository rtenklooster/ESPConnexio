//#######################################################################################################
//######################## PLUGIN-20 BMP0685 I2C Barometric Pressure Sensor  ############################
//#######################################################################################################

/*********************************************************************************************\
 * Support            : www.esp8266.nu
 * Date               : Apr 2015
 * Compatibility      : R003
 * Syntax             : "BMP085Read <Par2:Basis Variabele>"
 *********************************************************************************************
 * Technical description:
 * The BMP085 is a barometric pressure sensor connected through I2c (TwoWire)
 * This plugin uses two variables, 1 for temperature and 1 for pressure
 \*********************************************************************************************/
#define PLUGIN_ID_020        20
#define PLUGIN_NAME_020      "BMP085Read"

#define BMP085_I2CADDR           0x77
#define BMP085_ULTRAHIGHRES         3
#define BMP085_CAL_AC1           0xAA  // R   Calibration data (16 bits)
#define BMP085_CAL_AC2           0xAC  // R   Calibration data (16 bits)
#define BMP085_CAL_AC3           0xAE  // R   Calibration data (16 bits)    
#define BMP085_CAL_AC4           0xB0  // R   Calibration data (16 bits)
#define BMP085_CAL_AC5           0xB2  // R   Calibration data (16 bits)
#define BMP085_CAL_AC6           0xB4  // R   Calibration data (16 bits)
#define BMP085_CAL_B1            0xB6  // R   Calibration data (16 bits)
#define BMP085_CAL_B2            0xB8  // R   Calibration data (16 bits)
#define BMP085_CAL_MB            0xBA  // R   Calibration data (16 bits)
#define BMP085_CAL_MC            0xBC  // R   Calibration data (16 bits)
#define BMP085_CAL_MD            0xBE  // R   Calibration data (16 bits)
#define BMP085_CONTROL           0xF4 
#define BMP085_TEMPDATA          0xF6
#define BMP085_PRESSUREDATA      0xF6
#define BMP085_READTEMPCMD       0x2E
#define BMP085_READPRESSURECMD   0x34

uint8_t oversampling = BMP085_ULTRAHIGHRES;
int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
uint16_t ac4, ac5, ac6;

boolean bmp085_begin(void);
uint16_t bmp085_readRawTemperature(void);
uint32_t bmp085_readRawPressure(void);
int32_t bmp085_readPressure(void);
float bmp085_readTemperature(void);
uint8_t bmp085_read8(uint8_t a);
uint16_t bmp085_read16(uint8_t a);
void bmp085_write8(uint8_t a, uint8_t d);


boolean Plugin_020(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;

  switch(function)
  {
  case PLUGIN_INIT:
    {
      Serial.println("BMP init");
    //bmp085_begin();
    break;
    }

  case PLUGIN_COMMAND:
    {
    //UserVar[event->Par1 -1] = bmp085_readTemperature();
    //UserVar[event->Par1   ] = ((float)bmp085_readPressure())/100;
    success=true;
    break;
    }

  case PLUGIN_MMI_IN:
    {
    char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);
    if(GetArgv(string,TempStr,1))
      {
      if(strcasecmp(TempStr,PLUGIN_NAME_020)==0)
        {
        if(event->Par1 >0 && event->Par1<=USER_VARIABLES_MAX-1)
          {
          event->Type = ESP_TYPE_PLUGIN_COMMAND;
          event->Command = PLUGIN_ID_020;
          success=true;
          }
        }
      }
    free(TempStr);
    break;
    }

  case PLUGIN_MMI_OUT:
    {
    //strcpy(string,PLUGIN_NAME_020);
    //strcat(string," ");
    //strcat(string,int2str(event->Par1));
    sprintf(string,"%s",PLUGIN_NAME_020);
    sprintf(string,"%s%s",string," ");
    sprintf(string,"%s%s",string,int2str(event->Par1));
    break;
    }
  }      
  return success;
}

/*********************************************************************/
boolean bmp085_begin()
/*********************************************************************/
  { 
  if (bmp085_read8(0xD0) != 0x55) return false;

  /* read calibration data */
  ac1 = bmp085_read16(BMP085_CAL_AC1);
  ac2 = bmp085_read16(BMP085_CAL_AC2);
  ac3 = bmp085_read16(BMP085_CAL_AC3);
  ac4 = bmp085_read16(BMP085_CAL_AC4);
  ac5 = bmp085_read16(BMP085_CAL_AC5);
  ac6 = bmp085_read16(BMP085_CAL_AC6);

  b1 = bmp085_read16(BMP085_CAL_B1);
  b2 = bmp085_read16(BMP085_CAL_B2);

  mb = bmp085_read16(BMP085_CAL_MB);
  mc = bmp085_read16(BMP085_CAL_MC);
  md = bmp085_read16(BMP085_CAL_MD);
  }

/*********************************************************************/
uint16_t bmp085_readRawTemperature(void)
/*********************************************************************/
  {
  bmp085_write8(BMP085_CONTROL, BMP085_READTEMPCMD);
  delay(5);
  return bmp085_read16(BMP085_TEMPDATA);
  }

/*********************************************************************/
uint32_t bmp085_readRawPressure(void)
/*********************************************************************/
  {
  uint32_t raw;

  bmp085_write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (oversampling << 6));

  delay(26);

  raw = bmp085_read16(BMP085_PRESSUREDATA);
  raw <<= 8;
  raw |= bmp085_read8(BMP085_PRESSUREDATA+2);
  raw >>= (8 - oversampling);

  return raw;
  }

/*********************************************************************/
int32_t bmp085_readPressure(void)
/*********************************************************************/
  {
  int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
  uint32_t B4, B7;

  UT = bmp085_readRawTemperature();
  UP = bmp085_readRawPressure();

  // do temperature calculations
  X1=(UT-(int32_t)(ac6))*((int32_t)(ac5))/pow(2,15);
  X2=((int32_t)mc*pow(2,11))/(X1+(int32_t)md);
  B5=X1 + X2;

  // do pressure calcs
  B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ( (B6 * B6)>>12 )) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)ac1*4 + X3) << oversampling) + 2) / 4;

  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oversampling );

  if (B7 < 0x80000000) 
    {
    p = (B7 * 2) / B4;
    } 
  else 
   {
   p = (B7 / B4) * 2;
   }
  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

  p = p + ((X1 + X2 + (int32_t)3791)>>4);
  return p;
  }

/*********************************************************************/
float bmp085_readTemperature(void)
/*********************************************************************/
  {
  int32_t UT, X1, X2, B5;     // following ds convention
  float temp;

  UT = bmp085_readRawTemperature();

  // step 1
  X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) / pow(2,15);
  X2 = ((int32_t)mc * pow(2,11)) / (X1+(int32_t)md);
  B5 = X1 + X2;
  temp = (B5+8)/pow(2,4);
  temp /= 10;
  
  return temp;
  }

/*********************************************************************/
uint8_t bmp085_read8(uint8_t a)
/*********************************************************************/
  {
  uint8_t ret;

  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device 
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(); // end transmission
  
  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device 
  Wire.requestFrom(BMP085_I2CADDR, 1);// send data n-bytes read
  ret = Wire.read(); // receive DATA
  Wire.endTransmission(); // end transmission

  return ret;
  }

/*********************************************************************/
uint16_t bmp085_read16(uint8_t a)
/*********************************************************************/
  {
  uint16_t ret;

  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device 
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(); // end transmission
  
  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device 
  Wire.requestFrom(BMP085_I2CADDR, 2);// send data n-bytes read
  ret = Wire.read(); // receive DATA
  ret <<= 8;
  ret |= Wire.read(); // receive DATA
  Wire.endTransmission(); // end transmission

  return ret;
  }

/*********************************************************************/
void bmp085_write8(uint8_t a, uint8_t d)
/*********************************************************************/
  {
  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device 
  Wire.write(a); // sends register address to read from
  Wire.write(d);  // write data
  Wire.endTransmission(); // end transmission
  }
