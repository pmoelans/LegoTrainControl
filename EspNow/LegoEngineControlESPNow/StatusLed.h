#ifndef StatusLed_H
#define StatusLed_H
// MARK: Libraries
#include <Arduino.h>

class StatusLed {
public:
  // MARK: Lifecycle
  StatusLed(int onnTimeMsec, int offTimeMsec, int pin, int fastBlinkCount, int slowBlinkCount);
  void StartSlowBlink();
  void StartFastBlink();  
  void StopBlink();
  void UpdateState();
private:
  int _onnTimeMsec;
  int _offTimeMsec;
  int _currentOnTime;
  int _currentOffTime;
  int _isBlinking;
  int _pin;
  long _millisState;
  int _currentState = 0;
  int _blinkCount=0;
  int _slowBlinkCount=0;
  int _fastBlinkCount=0;
  bool _isBlinkingFast = false;
  int _currentBlinks=0;
  void SetBlinkParam(int blinkCount);
};


#endif