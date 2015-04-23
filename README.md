# Si4703_raspberry

There are 2 branches:

* master - this is just the Si4073 configuration code;

* UDPServer_radio_controller - this branch consists of 2 Qt projects:
	- 1 the server application wraped around the Si1073 module to allow
	remote control;
	- 2 the client application used to connect to the server and control
	the fm receiver;

This is a simple application just for setting up (with i2c) the Si4703 fm 
receiver module using the Raspberry Pi. The only dependency is the 
I2Cdev.h and I2Cdev.cpp needed for i2c communication. I used qmake and Qt to 
build this application just for ease of use.

You will also need to enable the i2c on raspberry. To do this, edit
/boot/config.txt, and add dtparam=i2c1=on. Also add i2c-bcm2708
i2c-dev to /etc/modules.

Things that works: setting volume, RDS (don't know how to use this),
seek UP/DOWN, set channel.

This code is mostly from
http://www.sparkfun.com/datasheets/BreakoutBoards/Si4703_Example.pde

* For this project i used the https://www.sparkfun.com/products/10663
(SparkFun FM Tuner Evaluation Board - Si4703)

* Raspberry Pi to Si4703 connections I used:

- Rpi--------------Si4703
- 3V3-------------->3.3V
- GND-------------->GND
- SDA-------------->SDIO
- SCL-------------->SCLK
- GPIO23----------->RST
