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
    void setRadioStation(char* name);
    void setSong(char* name);

public slots:
    void process();

signals:
    void finished();


private:
    uint16_t* si4703_registers;
    QString m_decodedRadioStation;
    QString m_decodedSong;
    char decodedRadioStation[20];
    char decodedSong[20];
    ShadowRegistersHandling* m_handler;
    bool m_stop;
};

#endif // RDSINFOTHREAD_H
