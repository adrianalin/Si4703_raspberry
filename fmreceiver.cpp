#include <cstdlib>
#include <unistd.h>
#include <QDebug>
#include <QtTest/QTest>
#include "fmreceiver.h"

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

FMReceiver::FMReceiver(QObject *parent, uint8_t addr):
    QObject(parent),
    m_devAddr(addr),
    m_started(false)
{
    start();
}

FMReceiver::~FMReceiver()
{
    qDebug() << "Destroying stuff";
    stop();
}

void FMReceiver::start()
{
    if (m_started) {
        qDebug() << "Radio already running!";
        return;
    }
    m_started = true;

    set2WireMode();
    initSI4703();
    // init done; now play something
    goToChannel();
    setVolume();
}

void FMReceiver::stop()
{
    if (!m_started) {
        qDebug() << "Radio already stopped!";
        return;
    }
    m_started = false;
    // Clear the DMUTE bit to enable mute.
    // Set the ENABLE bit high and DISABLE bit high to set the powerdown state.
    readRegisters();
    si4703_registers[POWERCFG] = 0x0041;
    updateRegisters();
}

void FMReceiver::set2WireMode()
{
    // GPIO23 on RPI -> RST on SI4703
    // Set up GPIO 23 and set to output
    system("echo \"23\" > /sys/class/gpio/export");
    system("echo \"out\" > /sys/class/gpio/gpio23/direction");

    // Write output
    system("echo \"0\" > /sys/class/gpio/gpio23/value");
    QTest::qSleep(100);
    system("echo \"1\" > /sys/class/gpio/gpio23/value");
    QTest::qSleep(100);
}

void FMReceiver::initSI4703()
{
    qDebug() << "\nInit IC";

    readRegisters(); // Read the current register set
    si4703_registers[0x07] = 0x8100; // Enable the oscillator
    updateRegisters(); // Update

    QTest::qSleep(500); // Wait for clock to settle - from AN230 page 9

    readRegisters(); // Read the current register set
    si4703_registers[POWERCFG] = 0x4001; // Enable the IC
    si4703_registers[SYSCONFIG1] |= (1 << RDS); // Enable RDS

#ifdef IN_EUROPE
    si4703_registers[SYSCONFIG1] |= (1 << DE); // 50kHz Europe setup
    si4703_registers[SYSCONFIG2] |= (1 << SPACE0); // 100kHz channel spacing for Europe
#else
    si4703_registers[SYSCONFIG2] &= ~(1 << SPACE1 | 1 << SPACE0) ; // Force 200kHz channel spacing for USA
#endif

    si4703_registers[SYSCONFIG2] &= 0xFFF0; // Clear volume bits
    si4703_registers[SYSCONFIG2] |= 0x0001; // Set volume to lowest
    updateRegisters(); // Update

    QTest::qSleep(110); // Max powerup time, from datasheet page 13
}

// Reads the current channel from READCHAN
// Returns a number like 973 for 97.3MHz
int FMReceiver::readChannel()
{
    readRegisters();
    int frequency = si4703_registers[READCHAN] & 0x03FF; // Mask out everything but the lower 10 bits

#ifdef IN_EUROPE
    //Freq(MHz) = 0.100(in Europe) * Channel + 87.5MHz
    //X = 0.1 * Chan + 87.5
    frequency *= 1; //98 * 1 = 98 - I know this line is silly, but it makes the code look uniform
#else
    //Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
    //X = 0.2 * Chan + 87.5
    channel *= 2; //49 * 2 = 98
#endif

    frequency += 875; //98 + 875 = 973

    if (m_frequency != frequency) {
        m_frequency = frequency;
        emit frequencyChanged(m_frequency);
    }

    return(frequency);
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

void FMReceiver::readRegisters()
{
    // Anytime you READ the regsters it writes the command byte to the first byte of the
    // SI4703 0x02 register. And then uses the list to write the 2nd bytes of register 0x02
    // and continues until all the list is written!
    // Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
    // We want to read the entire register set from 0x0A to 0x09 = 32 bytes.
    uint8_t initialReadings[32];
    int i = 0;
    I2Cdev::readBytes(m_devAddr, si4703_registers[POWERCFG] >> 8, 32, initialReadings);

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

void FMReceiver::setVolume(const quint8 value)
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

// Seeks out the next available station
// Returns the freq if it made it
// Returns zero if failed
bool FMReceiver::seek(quint8 seekDirection)
{
    qDebug() << "\n Seek";
    readRegisters();

    //Set seek mode wrap bit
    si4703_registers[POWERCFG] |= (1 << SKMODE); //Allow wrap
    //    si4703_registers[POWERCFG] &= ~(1 << SKMODE); //Disallow wrap - if you disallow wrap, you may want to tune to 87.5 first

    if (seekDirection == SEEK_DOWN)
        si4703_registers[POWERCFG] &= ~(1 << SEEKUP); //Seek down is the default upon reset
    else
        si4703_registers[POWERCFG] |= 1 << SEEKUP; //Set the bit to seek up

    si4703_registers[POWERCFG] |= (1 << SEEK); //Start seek

    updateRegisters(); //Seeking will now start

    //Poll to see if STC is set
    while(1) {
        readRegisters();
        if ((si4703_registers[STATUSRSSI] & (1 << STC)) != 0)
            break; //Tuning complete!
    }
    qDebug() << "Trying station: " << readChannel();

    readRegisters();
    int valueSFBL = si4703_registers[STATUSRSSI] & (1 << SFBL); //Store the value of SFBL
    si4703_registers[POWERCFG] &= ~(1 << SEEK); //Clear the seek bit after seek has completed
    updateRegisters();

    //Wait for the si4703 to clear the STC as well
    while(1) {
        readRegisters();
        if ((si4703_registers[STATUSRSSI] & (1 << STC)) == 0)
            break; //Tuning complete!
        qDebug() << "Waiting...";
    }

    if (valueSFBL) { //The bit was set indicating we hit a band limit or failed to find a station
        qDebug() << "Seek limit hit"; //Hit limit of band during seek
        return(FAIL);
    }

    qDebug() << "Channel set to " << readChannel();
    return(SUCCESS);
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
    int newChannel = value;
    newChannel *= 10;
    newChannel -= 8750;

#ifdef IN_EUROPE
    newChannel /= 10; //980 / 10 = 98
#else
    newChannel /= 20; //980 / 20 = 49
#endif

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

    qDebug() << "Station set to " << readChannel();
}

