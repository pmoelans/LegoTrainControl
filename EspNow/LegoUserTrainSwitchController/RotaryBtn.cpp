#include "RotaryBtn.h"

RotaryBtn::RotaryBtn(int rotationAInput, int rotationBInput, int pushInput, int deltaCounter, int minCounter, int maxCounter)
  {        
    _rotationAInput=rotationAInput;
    _rotationBInput=rotationBInput;
    _pushInput=pushInput;
    _deltaCounter=deltaCounter;
    _minCounter = minCounter;
    _maxCounter=maxCounter;
    
    pinMode(_rotationAInput, INPUT);
    pinMode(_rotationBInput, INPUT);
    pinMode(_pushInput, INPUT);

    _lastState = digitalRead(_rotationAInput);   
  }
  void RotaryBtn::ReverseDirection()
  {
    //Change the direction of the counter in software:
    _deltaCounter=(-1)*_deltaCounter;

  }
  int  RotaryBtn::ReadRotaryState()
  {
    int aState = digitalRead(_rotationAInput); // Reads the "current" state of the outputA
    // If the previous and the current state of the outputA are different, that means a Pulse has occured
    if (aState != _lastState){     
      // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
     
      if (digitalRead(_rotationBInput) != aState) { 
        _counter+=_deltaCounter;
        
      } else {
        _counter -=_deltaCounter;
       
      }

      if(_counter>_maxCounter)
        {
          _counter=_maxCounter;
        }
         if(_counter<_minCounter)
        {
          _counter= _minCounter;
        }
    
      
    } 
    _lastState = aState; // Updates the previous state of the outputA with the current state
    return _counter;
  }
  void  RotaryBtn::ResetCounter()
  {
    _counter=0;
  }
  void RotaryBtn::SetCounter(int counterValue)
  {
    _counter=counterValue;
  }
