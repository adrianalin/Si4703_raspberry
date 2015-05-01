#ifndef RDSINFOTHREAD_H
#define RDSINFOTHREAD_H

#include <QThread>
#include "shadowregistershandling.h"
#include <QObject>

class ShadowRegistersHandling;

class RDSInfoThread: public QObject
{
    Q_OBJECT

public:
    RDSInfoThread(ShadowRegistersHandling* handler, uint16_t* registers, QObject* parent = 0);
    ~RDSInfoThread();

    void stop(bool stop);

public slots:
    void process();

signals:
    void finished();

private:
    uint16_t* si4703_registers;
    char m_decodedRadioStation[20];
    char m_decodedSong[20];
    ShadowRegistersHandling* m_handler;
    bool m_stop;
};

#endif // RDSINFOTHREAD_H
