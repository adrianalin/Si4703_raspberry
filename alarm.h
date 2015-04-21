#ifndef ALARM_H
#define ALARM_H

#include <QObject>
#include <QTime>
#include <QTimer>

class Alarm : public QObject
{
    Q_OBJECT

public:
    Alarm();
    ~Alarm();

    void stop();

public slots:
    void start(QDateTime targetDateTime);

signals:
    void triggered();

private:
    QTimer m_alarmTimer;
};

#endif // ALARM_H
