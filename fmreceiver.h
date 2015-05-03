#ifndef FMRECEIVER_H
#define FMRECEIVER_H

#include <QObject>
#include "shadowregistershandling.h"
#include "rdsinfothread.h"

class FMReceiver: public QObject
{
    Q_OBJECT

public:
    FMReceiver(QObject* parent = 0);
    ~FMReceiver();

public slots:
    void goToChannel(const unsigned int value = 886);
    void setVolume(const quint8 value = 10);
    void stop();
    void start();
    void seek(quint8 seekDirection);
    void startAlarm();

signals:
    void frequencyChanged(int freq);
    void newSongInfo(QString name);
    void newRadioInfo(QString name);

private:
    // put SI4703 into 2 wire mode (I2C)
    void set2WireMode();
    void initSI4703();
    int readChannelFrequency();

private:
    int m_frequency;
    quint8 m_volume;
    bool m_started;
    uint16_t si4703_registers[16]; //There are 16 registers, each 16 bits large
    ShadowRegistersHandling m_handler;

    QThread* m_thread;
    RDSInfoThread* m_RDSWorker;
};

#endif // FMRECEIVER_H
