#ifndef BtnPress_H
#define BtnPress_H
// MARK: Libraries
#include <Arduino.h>

class BtnPress{
public:
  // MARK: Lifecycle
  BtnPress(int pin, long waitTime);
  bool LongPressDetected();  
  
private:
 long longPressTime;
 int _pin;
 long buttonTimer;
 bool buttonActive;
 bool longPressActive;
};


#endif