#ifndef SHADOWREGISTERSHANDLING_H
#define SHADOWREGISTERSHANDLING_H

#include <QObject>
#include <QMutex>
#include <stdint.h>
#include "I2Cdev.h"
#include "registers.h"

class ShadowRegistersHandling: public QObject
{
    Q_OBJECT

public:
    ShadowRegistersHandling(QObject* parent = 0);
    ~ShadowRegistersHandling();

    void readRegisters(uint16_t* si4703_registers);
    void updateRegisters(uint16_t* si4703_registers);

private:
    uint8_t m_devAddr;
    QMutex m_mutex;
};

#endif // SHADOWREGISTERSHANDLING_H
