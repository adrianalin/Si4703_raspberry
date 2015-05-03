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
    void setSongInfo(char* name);
    void pauseReadingRDSInfo(bool set);

public slots:
    void process();

signals:
    void finished();
    void newSongInfo(QString name);
    void newRadioInfo(QString name);

private:
    uint16_t* si4703_registers;
    QString m_radioInfo;
    QString m_songInfo;
    char decodedRadioStation[20];
    char decodedSong[20];
    ShadowRegistersHandling* m_handler;
    bool m_stop;
    bool m_pauseRading;  //if no radio station is selected don't try to read RDS info
};

#endif // RDSINFOTHREAD_H
