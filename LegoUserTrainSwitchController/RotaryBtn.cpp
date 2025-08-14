#include "RotaryBtn.h"



RotaryBtn::RotaryBtn(int rotationAInput, int rotationBInput, int pushInput)
  {        
    _rotationAInput=rotationAInput;
    _rotationBInput=rotationBInput;
    _pushInput=pushInput;
       
    pinMode(_rotationAInput, INPUT);
    pinMode(_rotationBInput, INPUT);
    pinMode(_pushInput, INPUT);

    _lastState = digitalRead(_rotationAInput);   
  }
  void RotaryBtn::RegisterCallBack(Motion motion,void (*callback)())
  {
    events[eventCount].motion = motion;
    events[eventCount].callback = callback;
    eventCount++;
  }
  void  RotaryBtn::CheckState()
  {
    CheckPressState();
    CheckRotaryState();
  } 
  void RotaryBtn::CheckPressState()
  {
    int btnState = digitalRead(_pushInput);
    //Serial.println(btnState);
    if(_btnPress!=btnState && btnState==1)
    {
      Motion motion=BtnPress;
      TriggerEvent(motion);      
    }
    _btnPress=btnState;
  }
  void  RotaryBtn::CheckRotaryState()
  {    
    int aState = digitalRead(_rotationAInput);     // Reads the "current" state of the outputA
    // If the previous and the current state of the outputA are different, that means a Pulse has occured
    if (aState != _lastState){     
      // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
      Motion _tempMotion;
      if (digitalRead(_rotationBInput) != aState) 
      { 
        _tempMotion=Positive;        
      } else 
      {
       _tempMotion=Negative;       
      }
       
      TriggerEvent(_tempMotion);
               
    }
    _lastState = aState;  
  }

  void  RotaryBtn::TriggerEvent(Motion motion) 
  {
        for (int i = 0; i < eventCount; i++) {
            if (events[i].motion == motion && events[i].callback) {
                events[i].callback(); // Call the callback function
            }
        }
  }


  
