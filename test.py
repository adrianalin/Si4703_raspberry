#Python Version 2 code
#The SI4703 reads starting at address 0x0A and starts writing to 0x02

#MANY THANKS to Nathan Seidle and his sketch example code at Sparkfun! I would have been lost without it.
#http://www.sparkfun.com/datasheets/BreakoutBoards/Si4703_Example.pde

#Here is the SI4703 Datasheet that explains the registers
#http://www.sparkfun.com/datasheets/BreakoutBoards/Si4702-03-C19-1.pdf

#Here is the SI4703 programming guide
#http://www.silabs.com/Support%20Documents/TechnicalDocs/AN230.pdf

#This is not finished code and needs a lot of work. I just wanted to prove it can
#work with the smbus to control the SI4703 2-wire interface. This code is just
#a proof of concept and really junk!

#Anytime you READ the regsters it writes the command byte to the first byte of the
#SI4703 0x02 register. And then uses the list to write the 2nd bytes of register 0x02
#and continues until all the list is written!

#So the reg = i2c.read_i2c_block_data(address, zz, 32)  command
#writes the integer zz to the first byte of register 0x02.
# so after setting the first byte of 0x02 to what you want it to be you need to
#make sure every write or read keeps populating that byte with what you need there

import RPi.GPIO as GPIO
import smbus
import time

i2c = smbus.SMBus(1) #use 0 for older RasPi

GPIO.setmode(GPIO.BCM) #board numbering
GPIO.setup(23, GPIO.OUT)
GPIO.setup(0, GPIO.OUT)  #SDA or SDIO

#put SI4703 into 2 wire mode (I2C)
GPIO.output(0,GPIO.LOW)
time.sleep(.1)
GPIO.output(23, GPIO.LOW)
time.sleep(.1)
GPIO.output(23, GPIO.HIGH)
time.sleep(.1)

address = 0x10 #address of SI4703 from I2CDetect utility

print "Initial Register Readings"
reg = i2c.read_i2c_block_data(address, 0, 32)
print reg

#write x8100 to reg 7 to activate oscellitor
list1 = [0,0,0,0,0,0,0,0,0,129,0]
w6 = i2c.write_i2c_block_data(address, 0, list1)
time.sleep(1)

#write x4001 to reg 2 to turn off mute and activate IC
list1 = [1]
#print list1
w6 = i2c.write_i2c_block_data(address, 64, list1)
time.sleep(.1)

#write volume
print "Doing VOlume lowest setting"
list1 = [1,0,0,0,0,0,13]
w6 = i2c.write_i2c_block_data(address, 64, list1)

#write channel
print "Setting Channel, pick a strong one"

nc = 886 #this is 101.1 The Fox In Kansas City Classic Rock!!
nc *= 10  #this math is for USA FM only
nc -= 8750
nc /= 20

list1 = [1,128, nc]
#set tune bit and set channel
w6 = i2c.write_i2c_block_data(address, 64, list1)
time.sleep(1) #allow tuner to tune
# clear channel tune bit
list1 = [1,0,nc]
w6 = i2c.write_i2c_block_data(address, 64, list1)

reg2 = i2c.read_i2c_block_data(address,64, 32)
print reg2  #just to show final register settings

#You should be hearing music now!
#Headphone Cord acts as antenna
