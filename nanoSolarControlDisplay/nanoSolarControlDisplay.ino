#include <SevenSegmentAsciiMap.h>
#include <SevenSegmentExtended.h>
#include <SevenSegmentFun.h>
#include <SevenSegmentTM1637.h>

#define SF_SOLAR (float)(1.0 + 47000.0/6800.0)
#define ADC_LSB (float)(5.0/1024)

#define HEAT_TIME 180  // in seconds
#define PUMP_TIME 30   // in seconds

const byte PIN_RELAY = 4;   // Relay control output  
const byte PIN_CLK = 3;     // define CLK pin (any digital pin)
const byte PIN_DIO = 2;     // define DIO pin (any digital pin)
const byte PIN_TEMP = 7;    // Temperature input
const byte PIN_SOLAR = 6;   // Solar panel voltage
const uint8_t degChar = 0x63;
const uint8_t degCharColon = 0xE3;
const uint8_t CChar = 0x39;
const uint8_t VChar = 0x3E;


SevenSegmentExtended display(PIN_CLK, PIN_DIO);
unsigned int a = 0;

// 10k NTC characterization data (10kOhm @ 25degC)
const float ntcAdcVal[15] = {613,597,558,514,473,437,422,409,388,374,363,356,351,347, 345};
const float ntcTemp[15]   = {-25,-20,-10,0.0,10 ,20 ,25 ,30 ,40 ,50 ,60 ,70 ,80 , 90 , 100.0};
const unsigned int numNtcVal = 15;
float rdTemp = 0.0;
float rdSolar= 0.0;
unsigned int adcVal = 0;

unsigned int secondCnt = 0;
unsigned int displayCnt = 0;
bool bPumpOn = false;

// Use Interpolation to find the temperature from ADC value 
float getNtcTemp(float adcRdVal)
{
  unsigned int ind1 = 0;
  unsigned int ind2 = 0;
  unsigned int i;
  float adcVal = 0.0;
  float slope = 1.0;
  if(adcRdVal > ntcAdcVal[0])
  {
    adcVal = ntcAdcVal[0];
  }
  else
  if (adcRdVal < ntcAdcVal[numNtcVal-1])
  {
    adcVal = ntcAdcVal[numNtcVal-1];
  }
  else
  {   
    adcVal = adcRdVal;
  }  
  for(i=0;i<numNtcVal;i++)
  {
    ind2 = i;
    ind1 = i;
    if(adcVal == ntcAdcVal[i])
    {
        break;
    }
    else    
    if(adcVal > ntcAdcVal[i])
    {
        ind1 = i-1;
        break;
    }
  }    
  if(ind1 == ind2)
  {
    return ntcTemp[ind1];
  }    
  else
  {
    slope = (ntcTemp[ind1] - ntcTemp[ind2])/(ntcAdcVal[ind1] - ntcAdcVal[ind2]);
    return (-slope * (ntcAdcVal[ind1]-adcVal) + ntcTemp[ind1]);
  }    
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_TEMP, 7);
  display.begin();            // initializes the display
  display.setBacklight(100);  // set the brightness to 100 %
  secondCnt = 0;
  displayCnt = 0;
  bPumpOn = false;
}

void loop() {

  switch(displayCnt)
  {
    case 0:  
      digitalWrite(LED_BUILTIN, true);
      adcVal=analogRead(PIN_TEMP);
      rdTemp = getNtcTemp((float)adcVal);
      display.printNumber((int)rdTemp,false,false,false);
      if (bPumpOn)
      {
        display.setColonOn(true);
      }
      else
      {
        display.setColonOn(false);
      } 
      display.printRaw(degChar,2);  
      display.printRaw(CChar,3);
      displayCnt++;
      break;
    case 1: 
      digitalWrite(LED_BUILTIN, false);
      adcVal=analogRead(PIN_SOLAR);
      rdSolar = (float)adcVal * ADC_LSB * SF_SOLAR;
      display.printNumber((int)rdSolar,false,false,false);
      display.printRaw(VChar,3);
      displayCnt++;
    default:
      displayCnt = 0;  
      break;
  }
  delay(1000);
  secondCnt++;
  if (secondCnt < HEAT_TIME)
  {
    digitalWrite(PIN_RELAY, false);
    bPumpOn = false;
  }
  else if(secondCnt >= HEAT_TIME && secondCnt <= HEAT_TIME + PUMP_TIME)
  {
    digitalWrite(PIN_RELAY, true);
    bPumpOn = true;
  }
  else if (secondCnt > HEAT_TIME + PUMP_TIME)
  {
    secondCnt = 0;
    digitalWrite(PIN_RELAY, false);
    bPumpOn = false;
  }
  
}
