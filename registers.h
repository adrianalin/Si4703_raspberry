#ifndef REGISTERS_H
#define REGISTERS_H

#define DEV_ADDR 0x10

#define IN_EUROPE

#define FAIL  0
#define SUCCESS  1

#define SEEK_DOWN  0 //Direction used for seeking. Default is down
#define SEEK_UP  1

//Define the register names
#define DEVICEID 0x00
#define CHIPID  0x01
#define POWERCFG  0x02
#define CHANNEL  0x03
#define SYSCONFIG1  0x04
#define SYSCONFIG2  0x05
#define STATUSRSSI  0x0A
#define READCHAN  0x0B
#define RDSA  0x0C
#define RDSB  0x0D
#define RDSC  0x0E
#define RDSD  0x0F

//Register 0x02 - POWERCFG
#define SMUTE  15
#define DMUTE  14
#define SKMODE  10
#define SEEKUP  9
#define SEEK  8

//Register 0x03 - CHANNEL
#define TUNE  15

//Register 0x04 - SYSCONFIG1
#define RDS  12
#define DE  11

//Register 0x05 - SYSCONFIG2
#define SPACE1  5
#define SPACE0  4

//Register 0x0A - STATUSRSSI
#define RDSR  15
#define STC  14
#define SFBL  13
#define AFCRL  12
#define RDSS  11
#define STEREO  8

// RDS decoding
#define GRPTYP 11
#define TYP0AGRP 0b00000
#define TYP0BGRP 0b00001
#define TYP1AGRP 0b00010
#define TYP1BGRP 0b00011
#define TYP2AGRP 0b00100
#define TYP2BGRP 0b00101
#define TYP4AGRP 0b01000

#endif // REGISTERS_H

