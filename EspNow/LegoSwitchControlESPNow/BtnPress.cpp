#include "BtnPress.h"


BtnPress::BtnPress(int pin, long waitTime)
{
  _pin = pin;
  longPressTime = waitTime;
  buttonActive = false;
  pinMode(_pin,INPUT);
}
bool BtnPress::LongPressDetected()
{
 if (digitalRead(_pin) == HIGH) {
    //button press is detected
    if (buttonActive == false) {
      buttonActive = true;
      buttonTimer = millis();
      return false;
    }

    if ((millis() - buttonTimer > longPressTime)) {
      //There is a long button press detected
      //=====================================
      longPressActive = true;
      return true;
    }
  } else  //readbtn press is LOW
  {
    //Check that a button press is active
    if (buttonActive == true) 
    {
       buttonActive = false;
      if (longPressActive == false) 
      {
        //short press, do nothing
       
        return false;
      } else {
        //Long press is detected:
        //=======================
        longPressActive = false;
        return true;
      }
      //reset the button state
     
     
    }
    return false;
  }
}