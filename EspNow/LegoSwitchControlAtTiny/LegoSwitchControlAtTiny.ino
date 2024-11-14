//this test is designed to test the responsiveness of the button press for th ATTiny13A and by changing the output based on the direction of the switching
//The test shows that the ATTiny is capable of detecting a btn press without delay and changes the led that needs to light up and keepds the led on until the end switch is reached.
//This works well with the debouncing method implemented

//Agreements:
//end 1: direction in which the train goes straight 
//end 2: direction in which the train takes the deviation

//this needs to be controlled with the MicroCore board
//EEPROM is not retained
//BOD=4.3V
//INternal clock 1.2MHz


#define Btn PB0
#define en1A PB1      //PB1
#define en2A PB2

#define End1 PB4
#define End2 PB3

bool buttonActive=false;
long time2Switch= -1;
long buttonTimer =-1;

bool isSwitching=false;
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
      Move2End1();
    }
    time2Switch= millis();
    isSwitching=true;
}
void Move2End1()
{
  digitalWrite(towardsDirection1,HIGH);
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

void setup() 
{
  pinMode(en1A,OUTPUT);
  pinMode(en2A,OUTPUT);
  pinMode(End1,INPUT);
  pinMode(End2,INPUT);
  pinMode(Btn,INPUT_PULLUP);

  
  Move2End1();
}

void loop() 
{
  //detect the buttonpress  
  if(isSwitching==false)
  {
    CheckBtnState();
  }
  
  if((digitalRead(End1)==HIGH || digitalRead(End2)==HIGH)&&(millis()-time2Switch)>checkEndStopsTimeOut)
  {
    StopEngine();
    isSwitching=false;
    delay(100);
  }   
}
