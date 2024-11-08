//this test is designed to test the responsiveness of the button press for th ATTiny13A and by changing the output based on the direction of the switching
//The test shows that the ATTiny is capable of detecting a btn press without delay and changes the led that needs to light up and keepds the led on until the end switch is reached.
//This works well with the debouncing method implemented

//Agreements:
//end 1: direction in which the train goes straight 
//end 2: direction in which the train takes the deviation

#define Btn PB0
#define en1A PB1      //PB1
#define en2A PB2

#define End1 PB4
#define End2 PB3

bool buttonActive=false;
long time2Switch= -1;
long buttonTimer =-1;
//bool DirectionWhenAtEnd1 = false;//was false
bool longPressActive=false;

int debounce = 20;
int checkEndStopsTimeOut=100;
int longPressTime = 200;

//this is the default direction:
//in the calibration fase we check which one is which:
int towardsDirection1=en1A;
int awayFromDirection1= en2A;

void UpdateSwitchState()
{
    StopEngine();
    if(digitalRead(End1)==HIGH)
    {
       digitalWrite(awayFromDirection1,HIGH);       
    }
    else
    {
      digitalWrite(towardsDirection1,HIGH);
    }
    time2Switch= millis();
}


void CheckBtnState() {
  if (digitalRead(Btn) == HIGH) {
    //button press is detected
    if (buttonActive == false) {
      buttonActive = true;
      buttonTimer = millis();
      return;
    }

    if ((millis() - buttonTimer > longPressTime))
     {
      //There is a long button press detected
      //=====================================
      longPressActive = true;
      return;
    }
  } else  //readbtn press is LOW
  {
    //Check that a button press is active
    if (buttonActive == true) {
      if (longPressActive == false) {
        //short press, do nothing
        
      } else {
        //Long press is detected:
        //=======================
        UpdateSwitchState();
      }
      //reset the button state
      buttonActive = false;
      return;
    }
  }
}

void StopEngine()
{
  digitalWrite(en1A, LOW);
  digitalWrite(en2A, LOW);
}
void Calibrating()
{
  StopEngine();

  //Here we calibrate what direction we should move in when we start up the switch:
  //===============================================================================
  if(digitalRead(End1)==HIGH || (digitalRead(End1)==LOW && digitalRead(End2)==LOW))
  {
      //move away from the end1 and check whether the direction is ok
      digitalWrite(awayFromDirection1,HIGH);  
      WaitWhileEndIsReached();
      if(digitalRead(End2)==HIGH)
      {
        return;
      }    
      awayFromDirection1=en1A;
      towardsDirection1 = en2A;
  }
  else if(digitalRead(End2)==HIGH)
  {
      digitalWrite(towardsDirection1,HIGH);
      WaitWhileEndIsReached();
      if(digitalRead(End1)==HIGH)
      {
        return;
      } 
      awayFromDirection1= en1A;
      towardsDirection1 = en2A;
  }
  
  StopEngine();
  
  //Move towards direction1:
  //========================
  digitalWrite(towardsDirection1,HIGH);

}
void WaitWhileEndIsReached()
{
  while(digitalRead(End1)==LOW && digitalRead(End2)==LOW)
  {
    delay(50);
  }
   StopEngine();
}
void setup() 
{
  pinMode(en1A,OUTPUT);
  pinMode(en2A,OUTPUT);
  pinMode(End1,INPUT);
  pinMode(End2,INPUT);
  pinMode(Btn,INPUT);

  
  Calibrating();
}

void loop() 
{
  //detect the buttonpress  
  CheckBtnState();
  
  if((digitalRead(End1)==HIGH || digitalRead(End2)==HIGH)&&(millis()-time2Switch)>checkEndStopsTimeOut)
  {
    StopEngine();
  }   
}
