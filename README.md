# Si4703_raspberry

This is a simple application just for setting up (with i2c) the Si4703 fm 
receiver module using the Raspberry Pi. The only dependency is the 
I2Cdev.h and I2Cdev.cpp needed for i2c communication. I used qmake and Qt to 
build this application just for ease of use.

You will also need to enable the i2c on raspberry.

Things that works: setting volume, RDS (don't know how to use this), 
seek UP/DOWN, set channel.
