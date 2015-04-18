#include <cstdlib>
#include <unistd.h>
#include <QDebug>
#include "fmreceiver.h"

FMReceiver::FMReceiver(uint8_t addr):
    m_devAddr(addr)
{

}

FMReceiver::~FMReceiver()
{

}

void FMReceiver::set2WireMode()
{
    // GPIO23 on RPI -> RST on SI4703
    // Set up GPIO 23 and set to output
    system("echo \"23\" > /sys/class/gpio/export");
    system("echo \"out\" > /sys/class/gpio/gpio23/direction");

    // Write output
    system("echo \"0\" > /sys/class/gpio/gpio23/value");
    sleep(1);
    system("echo \"1\" > /sys/class/gpio/gpio23/value");
    sleep(1);
}

void FMReceiver::readRegisters()
{
    // Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
    // We want to read the entire register set from 0x0A to 0x09 = 32 bytes.
    uint8_t initialReadings[32];
    int i = 0;
    I2Cdev::readBytes(m_devAddr, 0, 32, initialReadings);

    printf("initialReadings = ");
    for (uint j = 0; j < 32; j++) {
        printf("%x, ", initialReadings[j]);
    }
    printf("\n");

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

    printf("reg = ");
    for (uint j = 0; j < 16; j++) {
        printf("%x, ", si4703_registers[j]);
    }
    printf("\n");
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
    readRegisters();
    // write x8100 to reg 7 to activate oscilator
    // Enable the oscillator, from AN230 page 12 (rev 0.9)
    si4703_registers[0x07] = 0x8100;
    updateRegisters();
}

void FMReceiver::turnOffMute()
{
    // write x4001 to reg 2 to turn off mute and activate IC
    //
    uint8_t data[1] = {1}; // writes 1 to lower byte of reg 2
    I2Cdev::writeBytes(m_devAddr, 64, sizeof(data), data); // writes 64 to upper byte of reg 2
}

void FMReceiver::setVolume(const uint8_t value)
{
    // "set volume"
    uint8_t data[7] = {1, 0, 0, 0, 0, 0, 13};
    if (value > 15) {
        qDebug() << "Volume must be between [0, 15]";
        return;
    }
    qDebug() << "Setting Volume value to " << value;
    data[6] = value;
    I2Cdev::writeBytes(m_devAddr, 64, sizeof(data), data);
}

void FMReceiver::goToChannel(const unsigned int value)
{
    qDebug() << "Setting channel to " << value;
    // write channel reg 03h
    uint8_t newChannel = value;
    newChannel *= 10;
    newChannel -= 8750;
    newChannel /= 20;

    uint8_t data[3] = {1,128, newChannel};
    I2Cdev::writeBytes(m_devAddr, 64, sizeof(data), data);

    // clear channel tune bit
    sleep(1);
    data[1] = 0;
    I2Cdev::writeBytes(m_devAddr, 64, sizeof(data), data);
}

void FMReceiver::init()
{
    set2WireMode();

    sleep(2);
    setOsc();


    sleep(2);
    turnOffMute();
    sleep(2);
    setVolume();
    sleep(2);
    goToChannel();
    sleep(2);

    qDebug() << "end init";
}

