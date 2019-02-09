# Si4703_raspberry

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

Rpi---------------Si4703

3V3-------------->3.3V

GND-------------->GND

SDA-------------->SDIO

SCL-------------->SCLK

GPIO23----------->RST
