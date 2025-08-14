# Introduction
This project started out as a hobby project and got a tiny bit out of hand. The goal of the project was to create a custom controller for lego power functions trains. 
The specific goal was to get rid of the infrared controller and use Wifi instead. The Wifi turned out to be an issue since the connection time of the arduino boards with wifi can take a while and the attention span of 
kids is rather limited. Hence the current controller is now working with the ESPNOW protocol. 
Currently the project contains 3 seperate modules:
+ The train controller
+ The switch controller
+ The user controller(UI)
The following paragraphs describe the seperate modules.

## The train controller
The train controller is the hardware and software that is onboard the train itself and it controls the power functions motor. This module is build around the QTPY-C3 ESP32 platform. 
The form factor was the driving force behind the choice of platform. Namely the entire contraption (including battery) needs to have a similar size as the battery box of a powerfunctions motor. This puts some
serious constraints on the design. The platform also needs a few additional features with respect to the original power functions versions:
+ An emergency stop function: once this command is given, all trains need to stop as quickly as possible. This is a software thing and does not impact the overal design
+ An shutdown feature: once this command is given. The train stops and the entire circuit needs to be shut down. This has an impact on the hardware of the project in such a way that the ESP can shut itself down.
+ The basic idea for this controller was first found on https://mattzobricks.com/. A must see for Lego train enthousiasts. On this site there was a section where a similar controller as mine was created. This controller
  did not fit my exact needs since it was used together with the Rocrail platform. But it served as a firm basis for the controller described here.
 
 ### Hardware
 The board is build up around the following two modules:
 + QTPY-C3 ESP32
 + TB6612 Dual motor driver (TB6612FNG). Instead of just using the chip, I used the existing ready made breakout board
 + Custom build PCB(I'm not an electronics engineer so I'm sure that things could have been done better and more efficiently)
 + 3D printed casing in which you can snap in the PCB board as well as fit the lego power functions connectors. The shape of the box is heavily inspired on the original power functions box and the box on https://mattzobricks.com/.
  
## The swith controller
The switch controller is the hard and software that is used to control the switch. We use end switches in order to detect whether the switch is in its final position. This works a bit easer since each type of lego
switch has its own movement which might take a little more time to make the actual switch.

### Hardware
Main parts:
+ ATTiny85A
+ L9110 motor controller
+ Small lego engine
+ Two end switches
+ Custom PCB
+ 3D printed casing for the PCB.

## The user controller(UI)
This is the handheld controller that is used to control the trains(for now). We can control the switches as well, but the UI is specifically designed to quickly change between trains in question. You can have multple trains
that are connected to the controller. The only limit is the memory of the ESP32.

### Hardware
The design of the UI is pretty straight forward and contains of:
+ A rotary button:
  + Rotating the button speeds up or slows down the train
  + Pushing the rotary button switches to the next train that is registered.
+ Two buttons:
  + Emergency stop button: stops all trains
  + A shut down button: shuts down all trains
This controller is also build around the QTPY-C3 board. 
  
### Software
The software uses the ESPNOW protocol the communicate with all the trains. The controller has two functions:
+ Listens for new trains: Upon startup of a train (battery is on) it will start sending connection requests to the controller. This is done every few seconds until the controller sends an accept message to the train.
  After this accept request the train will send its status every 10s to the controller. The trains need to be identified and the unique identifier used is the mac-address of the ESP32.
  This mac-address together with the current state of the train(e.g. speed, battery voltage) are kept as well.
+ Controls the current train: Every time the rotary button is used, a message is send that reflects the changes. This message is only send to the train that is active at the moment. The state of the current train is also updated once the
    rotary button is used. Every new train creates a new entry in the dictionary
If one of the two other buttons is pressed, the message is send to all the trains.

# Future work



