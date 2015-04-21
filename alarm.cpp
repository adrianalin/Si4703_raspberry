#include "alarm.h"
#include <QDebug>

Alarm::Alarm()
{
    m_alarmTimer.setSingleShot(true);
    connect(&m_alarmTimer, SIGNAL(timeout()), this, SIGNAL(triggered()));
}

Alarm::~Alarm()
{

}

void Alarm::start(QDateTime targetDateTime)
{
    const QDateTime currentDateTime = QDateTime::currentDateTime();
    const int msecs = currentDateTime.msecsTo(targetDateTime);
    qDebug() << "currentDateTime = " << currentDateTime.toString();
    qDebug() << "Start alarm targetDateTime = " << targetDateTime.toString() << ", msecs = " << msecs;
    m_alarmTimer.start(msecs);
}

void Alarm::stop()
{
    if (m_alarmTimer.isActive())
        m_alarmTimer.stop();
}

