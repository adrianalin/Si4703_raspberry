#include <cstdlib>
#include <unistd.h>
#include <QDebug>
#include <QtTest/QTest>
#include "fmreceiver.h"

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

FMReceiver::FMReceiver(uint8_t addr):
    m_devAddr(addr)
{

}

FMReceiver::~FMReceiver()
{

}

void FMReceiver::checkRDS()
{
    qDebug() << "\nCheck RDS";
    for (int i = 0; i < 200; i++) {
        readRegisters();
        if (si4703_registers[STATUSRSSI] & (1 << RDSR)) {
            const uint8_t blockerrors = (si4703_registers[STATUSRSSI] & 0x0600) >> 9; // Mask in BLERA
            if (blockerrors == 1)
                qDebug() << " (1-2 RDS errors)";
            else if (blockerrors == 2)
                qDebug() << " (3-5 RDS errors)";
            else if (blockerrors == 3)
                qDebug() << " (6+ RDS errors)";

            char Ah, Al, Bh, Bl, Ch, Cl, Dh, Dl;
            Ah = (si4703_registers[RDSA] & 0xFF00) >> 8;
            Al = (si4703_registers[RDSA] & 0x00FF);

            Bh = (si4703_registers[RDSB] & 0xFF00) >> 8;
            Bl = (si4703_registers[RDSB] & 0x00FF);

            Ch = (si4703_registers[RDSC] & 0xFF00) >> 8;
            Cl = (si4703_registers[RDSC] & 0x00FF);

            Dh = (si4703_registers[RDSD] & 0xFF00) >> 8;
            Dl = (si4703_registers[RDSD] & 0x00FF);

            qDebug() << "RDS: " << Bh << Bl << Ch << Cl << Dh << Dl;
            QTest::qSleep(40);
        } else {
            QTest::qSleep(30);
        }
    }
}

void FMReceiver::set2WireMode()
{
    // GPIO23 on RPI -> RST on SI4703
    // Set up GPIO 23 and set to output
    system("echo \"23\" > /sys/class/gpio/export");
    system("echo \"out\" > /sys/class/gpio/gpio23/direction");

    // Write output
    system("echo \"0\" > /sys/class/gpio/gpio23/value");
    QTest::qSleep(500);
    system("echo \"1\" > /sys/class/gpio/gpio23/value");
    QTest::qSleep(500);
}

void FMReceiver::readRegisters()
{
    // Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
    // We want to read the entire register set from 0x0A to 0x09 = 32 bytes.
    uint8_t initialReadings[32];
    int i = 0;
    I2Cdev::readBytes(m_devAddr, 64, 32, initialReadings);

    // Remember, register 0x0A comes in first so we have to shuffle the array around a bit
    for (int x = 0x0A ; ; x++) { // Read in these 32 bytes
        if (x == 0x10)
            x = 0; // Loop back to zero
        si4703_registers[x] = initialReadings[i] << 8;
        i++;
        si4703_registers[x] |= initialReadings[i];
        i++;
        if (x == 0x09)
            break; // We're done!
    }
}

// Write the current 6 control registers (0x02 to 0x07) to the Si4703
// It's a little weird, you don't write an I2C address
// The Si4703 assumes you are writing to 0x02 first, then increments
void FMReceiver::updateRegisters()
{
    uint8_t dataToSend[12];
    int i = 0;
    // A write command automatically begins with register 0x02 so no need to send a write-to address
    // First we send the 0x02 to 0x07 control registers
    // In general, we should not write to registers 0x08 and 0x09
    for (int regSpot = 0x02; regSpot < 0x08; regSpot++) {
        const uint8_t high_byte = si4703_registers[regSpot] >> 8;
        const uint8_t low_byte = si4703_registers[regSpot] & 0x00FF;

        dataToSend[i] = high_byte;
        i++;
        dataToSend[i] = low_byte;
        i++;
    }


    printf("Update registers: dataToSend = ");
    for (int j=0; j<12; j++) {
        printf("%x ", dataToSend[j]);
    }
    printf("\n");

    I2Cdev::writeBytes(m_devAddr, dataToSend[0], sizeof(dataToSend) - 1, &dataToSend[1]);
}

void FMReceiver::setOsc()
{
    qDebug() << "\nSet Oscilator";
    readRegisters();
    // write x8100 to reg 7 to activate oscilator
    // Enable the oscillator, from AN230 page 12 (rev 0.9)
    si4703_registers[0x07] = 0x8100;
    updateRegisters();
}

void FMReceiver::enableIC()
{
    qDebug() << "\nEnable IC";
    readRegisters(); // Read the current register set
    si4703_registers[POWERCFG] = 0x4001; // Enable the IC
    si4703_registers[SYSCONFIG1] |= (1 << RDS); // Enable RDS
    si4703_registers[SYSCONFIG2] &= 0xFFF0; // Clear volume bits
    si4703_registers[SYSCONFIG2] |= 0x0001; // Set volume to lowest
    updateRegisters(); //Update
}

void FMReceiver::setVolume(const uint8_t value)
{
    qDebug() << "\nSet volume to " << value;
    if (value > 15) {
        qDebug() << "Cannot set volume. Must be between [0, 15]";
        return;
    }

    readRegisters();
    si4703_registers[SYSCONFIG2] &= 0xFFF0; // Clear volume bits
    si4703_registers[SYSCONFIG2] |= value;
    updateRegisters(); // Update
}

// Given a channel, tune to it
// Channel is in MHz, so 973 will tune to 97.3MHz
// Note: gotoChannel will go to illegal channels (ie, greater than 110MHz)
// It's left to the user to limit these if necessary
// Actually, during testing the Si4703 seems to be internally limiting it at 87.5. Neat.
void FMReceiver::goToChannel(const unsigned int value)
{
    qDebug() << "\nGo to channel " << value;
    // write channel reg 03h
    uint8_t newChannel = value;
    newChannel *= 10;
    newChannel -= 8750;
    newChannel /= 20;

    // These steps come from AN230 page 20 rev 0.5
    readRegisters();
    si4703_registers[CHANNEL] &= 0xFE00; // Clear out the channel bits
    si4703_registers[CHANNEL] |= newChannel; // Mask in the new channel
    si4703_registers[CHANNEL] |= (1 << TUNE); // Set the TUNE bit to start
    updateRegisters();

    // Poll to see if STC is set
    while(1) {
        QTest::qSleep(500);
        readRegisters();
        if ((si4703_registers[STATUSRSSI] & (1 << STC)) != 0)
            break; // Tuning complete!
        qDebug() << "Tuning...";
    }

    readRegisters();
    si4703_registers[CHANNEL] &= ~(1 << TUNE); // Clear the tune after a tune has completed
    updateRegisters();

    // Wait for the si4703 to clear the STC as well
    while(1) {
        readRegisters();
        if ((si4703_registers[STATUSRSSI] & (1 << STC)) == 0)
            break; //Tuning complete!
        qDebug() << "Waiting...";
    }
}

void FMReceiver::init()
{
    set2WireMode();

    QTest::qSleep(500);
    setOsc();
    QTest::qSleep(500);
    enableIC();
    QTest::qSleep(500);
    setVolume();
    QTest::qSleep(500);
    goToChannel();

    qDebug() << "end init";
}

void FMReceiver::stop()
{
    // Clear the DMUTE bit to enable mute.
    // Set the ENABLE bit high and DISABLE bit high to set the powerdown state.
    readRegisters();
    si4703_registers[POWERCFG] = 0x0041;
    updateRegisters();
    exit(0);
}

