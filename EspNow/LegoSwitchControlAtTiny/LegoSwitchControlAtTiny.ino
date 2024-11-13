//this method only relies on the reset button being activated for the switching action.

//Agreements:
//end 1: direction in which the train goes straight 
//end 2: direction in which the train takes the deviation

//for programming we need the Microcore Attiny13 board
//BOD 4.3V
//1.2MHz internal clock
//EEPROM not retained

#define Btn PB0
#define en1A PB1      //PB1
#define en2A PB2

#define End1 PB4
#define End2 PB3

bool buttonActive=false;
long time2Switch= -1;
bool need2Move2End2=false;
bool need2Move2End1 = false;

int checkEndStopsTimeOut=400;
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
      need2Move2End2=true;
      digitalWrite(awayFromDirection1,HIGH);       
    }
    else
    {
     Move2End1();
    }
    time2Switch= millis();
}
void Move2End1()
{
  need2Move2End1=true;
  digitalWrite(towardsDirection1,HIGH);

}



void StopEngine()
{
  digitalWrite(en1A, LOW);
  digitalWrite(en2A, LOW);
  need2Move2End2=false;
  need2Move2End1=false;
}


void setup() 
{
  pinMode(en1A,OUTPUT);
  pinMode(en2A,OUTPUT);
  pinMode(End1,INPUT);
  pinMode(End2,INPUT);
  UpdateSwitchState();
  //delay(50);
  //Move2End1();
}

void loop() 
{
  //detect the buttonpress  
  //CheckBtnState();

  //if(isSwitching)
  //{
    if((digitalRead(End1)==HIGH && need2Move2End1))// || digitalRead(End2)==HIGH)&&(millis()-time2Switch)>checkEndStopsTimeOut)
    {
      StopEngine();
    }  
    if((digitalRead(End2)==HIGH && need2Move2End2))// || digitalRead(End2)==HIGH)&&(millis()-time2Switch)>checkEndStopsTimeOut)
    {
      StopEngine();
    } 
    if((millis()-time2Switch)>2000)
    {
      StopEngine();
    }
}
