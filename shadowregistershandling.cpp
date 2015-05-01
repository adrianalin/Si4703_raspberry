#include "shadowregistershandling.h"
#include <QMutexLocker>

ShadowRegistersHandling::ShadowRegistersHandling(QObject *parent):
    QObject(parent),
    m_devAddr(DEV_ADDR)
{

}

ShadowRegistersHandling::~ShadowRegistersHandling()
{

}

void ShadowRegistersHandling::readRegisters(uint16_t *si4703_registers)
{
    QMutexLocker locker(&m_mutex);
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
void ShadowRegistersHandling::updateRegisters(uint16_t *si4703_registers)
{
    QMutexLocker locker(&m_mutex);
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

    I2Cdev::writeBytes(m_devAddr, dataToSend[0], sizeof(dataToSend) - 1, &dataToSend[1]);
}
