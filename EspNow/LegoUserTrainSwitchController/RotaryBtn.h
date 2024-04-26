#ifndef RotaryBtn_H
#define RotaryBtn_H
// MARK: Libraries
#include <Arduino.h>
// MARK: Class / Functions / Variables
class RotaryBtn {
public:
  // MARK: Lifecycle
  int ReadRotaryState();
  void SetCounter(int counterValue);
  void ReverseDirection();
  void ResetCounter();
  RotaryBtn(int rotationAInput, int rotationBInput, int pushInput, int deltaCounter, int minCounter, int maxCounter);
private:
  // MARK: Variables
  // Debuging Identifier
  int _counter=0;
  int _rotationAInput;
  int _rotationBInput;
  int _pushInput;
  int _lastState=0;
  int _deltaCounter;
  int _minCounter;
  int _maxCounter;  
};



#endif


  