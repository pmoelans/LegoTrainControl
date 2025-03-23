#include "StatusLed.h"

StatusLed::StatusLed(int onnTime, int offTime, int pin,int fastBlinkCount, int slowBlinkCount)
{
  _onnTimeMsec= onnTime;
  _offTimeMsec= offTime;
  _currentOnTime= onnTime;
  _currentOffTime= offTime;
  _pin= pin;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin,HIGH);
  _currentState=1;
  _fastBlinkCount = fastBlinkCount;
  _slowBlinkCount = slowBlinkCount;
}
void StatusLed::StartFastBlink()
{
 // Serial.println("fast Blink active");
  _currentOnTime= _onnTimeMsec/5;
  _currentOffTime= _offTimeMsec/5;  
  _isBlinkingFast=true;
  SetBlinkParam(_fastBlinkCount); 
}

void StatusLed::SetBlinkParam(int blinkCount)
{
  _blinkCount=blinkCount;
  _currentBlinks=0;
  _isBlinking=true;  
  _millisState= millis();
}
void StatusLed::StartSlowBlink()
{  
  //Serial.println("Slow Blink active");
  _currentOnTime= _onnTimeMsec;
  _currentOffTime= _offTimeMsec;
  SetBlinkParam(_slowBlinkCount); 
}
void StatusLed::StopBlink()
{
  _isBlinking=false;
  digitalWrite(_pin,HIGH);
}
void StatusLed::UpdateState()
{
  if(_isBlinking==false)
  {
    return;
  }
  
  if(_currentBlinks>=_blinkCount)
  {
    //Check if we are blinking fast, if so, turn it off and start blinking slowly. Else stop blink
    if(_isBlinkingFast)
    {
      _isBlinkingFast= false;
      //switch to slow blink
      StartSlowBlink();
      return;
    }
    //Stop blinking
    StopBlink();
    return;
  }
    
  //Check if the onnTime is passed
  if((_currentState==1)&&((millis()-_millisState)>_currentOnTime))
  {
    digitalWrite(_pin,LOW);
    _millisState= millis();
    _currentState=0;   
  }

  //Check if the offtime has passed
  if((_currentState==0)&&((millis()-_millisState)>_currentOffTime))
  {
    digitalWrite(_pin,HIGH);
    _millisState= millis();
    _currentState=1;
    _currentBlinks+=1;
  }

}
