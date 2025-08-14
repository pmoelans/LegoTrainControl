#ifndef RotaryBtn_H
#define RotaryBtn_H
// MARK: Libraries
#include <Arduino.h>
#include "enums.h"
// MARK: Class / Functions / Variables
class RotaryBtn {
  struct Event {
    Motion motion;
    void (*callback)();
};
public:
  // MARK: Lifecycle
  RotaryBtn(int rotationAInput, int rotationBInput, int pushInput);
  void  CheckState();
  //Callback function that needs to be called for a given motion
  void RegisterCallBack(Motion motion,void (*callback)());
private:
  // MARK: Variables
  // Debuging Identifier
  int _counter=0;
  int _rotationAInput;
  int _rotationBInput;
  int _pushInput;
  int _lastState=0;
  bool _btnPress=false;

  void  TriggerEvent(Motion motion);
  Event events[10]; // Array to store events, adjust size as needed
  int eventCount = 0;   

  void CheckRotaryState();
  void CheckPressState();
};



#endif


  