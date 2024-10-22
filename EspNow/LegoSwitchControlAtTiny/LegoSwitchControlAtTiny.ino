#include <avr/io.h>
#define SwitchControl 0
#define Direction1 1      //PB1
#define Direction2 2      //PB2
#define End1 3            //PB3
#define End2 4            //PB4
//#define Power 5           //PB5

bool DirectionWhenAtEnd1 = true;//was false
bool need2Switch= false;
bool buttonActive= false;
long millis_switch;
long switchingTimeOut = 50;

void DetermineStartPosition()
{
  
  //Determine the start position:
  //=============================
  if(digitalRead(End1)==1)
  {
    DirectionWhenAtEnd1=true;
    return;
  }
  if(digitalRead(End2)==1)
  {
    DirectionWhenAtEnd1=false;
    return;
  }
  //unkown position, switch to a position:
  //======================================
  UpdateSwitchState();
}

void UpdateSwitchState() {
 
  StopMotor();

  if (DirectionWhenAtEnd1) {    
    digitalWrite(Direction2,HIGH);
  
  } else {
    digitalWrite(Direction1,HIGH);    
  }
  need2Switch = true;
  //digitalWrite(Power,HIGH); 
  millis_switch= millis();
  //millis_switch=millis();

}
void StopMotor()
{
  //set all to low
  digitalWrite(Direction1,LOW); 
  digitalWrite(Direction2,LOW); 
  //digitalWrite(Power,LOW); 
 
}

void CheckBtnState() 
{
   if (digitalRead(SwitchControl)==1) {
    //button press is detected
    if (buttonActive == false) {
      buttonActive = true;
      return;
    }    
  } else  //readbtn press is LOW
  {
    //Check that a button press is active
    if (buttonActive == true) {
     
      //Blink(1);
      UpdateSwitchState();
      
      //reset the button state
      buttonActive = false;
      return;
    }
  }
}


void setup() {
  millis_switch= millis();
  //millis_pre = millis();
  pinMode(Direction1,OUTPUT);
  pinMode(Direction2,OUTPUT);
//  pinMode(Power,OUTPUT);
  
  pinMode(SwitchControl,INPUT);
  pinMode(End1,INPUT);
  pinMode(End2,INPUT);

  StopMotor();

  //digitalWrite(Led, HIGH);
  DetermineStartPosition();
}

void loop() 
{
  //detect the buttonpress  
  CheckBtnState();
  

   if( (millis()-millis_switch>switchingTimeOut) &&need2Switch)
   {  

     //Check if end1/2 is Reached
    //=======================
    if((digitalRead(End1)==1)||(digitalRead(End2)==1 ))
    {
      
      StopMotor();
      //update the direction:
      //=====================
      DirectionWhenAtEnd1=!DirectionWhenAtEnd1;

      need2Switch=false;
    }    
  }
}
