#include "rdsinfothread.h"
#include "registers.h"
#include "QDebug"
#include <QTest>

RDSInfoThread::RDSInfoThread(ShadowRegistersHandling *handler, uint16_t *registers, QObject *parent):
    QObject(parent),
    si4703_registers(registers),
    m_handler(handler),
    m_stop(false),
    m_pauseRading(false)
{
    Q_ASSERT(m_handler);
    qDebug() << "Create RDSInfoThread";
}

RDSInfoThread::~RDSInfoThread()
{
    qDebug() << "Destroy RDSInfoThread";
}

void RDSInfoThread::stop(bool stop)
{
    m_stop = stop;
}

void RDSInfoThread::pauseReadingRDSInfo(bool set)
{
    m_pauseRading = set;
}

void RDSInfoThread::setRadioStation(char *name)
{
    static int i=0;
    i++;
    if (i < 5)
        return ;
    i = 0;
    const QString radioInfo = QString::fromUtf8(name).simplified();
    if (m_radioInfo == radioInfo)
        return;
    m_radioInfo = radioInfo;
    emit newRadioInfo(m_radioInfo);
    qDebug() << "Radio info = " << m_radioInfo;
}

void RDSInfoThread::setSongInfo(char *name)
{
    static QString lastSongInfo;
    const QString string = QString::fromUtf8(name).simplified();
    if (string.length() > 0)
        m_songInfo.append(string);

    if (m_songInfo.length() > 25) {
        if (lastSongInfo == m_songInfo)
            return;
        emit newSongInfo(m_songInfo);
        qDebug() << "Song info = " << m_songInfo;
        lastSongInfo = m_songInfo;
        m_songInfo.clear();
    }
}

void RDSInfoThread::process()
{
    qDebug() << "\nCheck RDS";
    m_stop = false;
    while (!m_stop) {
        if (m_pauseRading) { // if no channel is set makes no sens to read RDS
            qDebug() << "Pause RDS info read";
            QTest::qSleep(1000);
            continue;
        }

        m_handler->readRegisters(si4703_registers);
        if (si4703_registers[STATUSRSSI] & (1 << RDSR)) {
            const char Ch = (si4703_registers[RDSC] & 0xFF00) >> 8;
            const char Cl = (si4703_registers[RDSC] & 0x00FF);
            const char Dh = (si4703_registers[RDSD] & 0xFF00) >> 8;
            const char Dl = (si4703_registers[RDSD] & 0x00FF);

            const int grptype = si4703_registers[RDSB] >> GRPTYP;
            if (grptype == TYP0AGRP) {
                const int index = si4703_registers[RDSB] & 0x03;
                if (index == 0) {
                    setRadioStation(decodedRadioStation);
                    memset(decodedRadioStation, 0, sizeof(decodedRadioStation));
                }
                decodedRadioStation[index * 2] = Dh;
                decodedRadioStation[index * 2 + 1] = Dl;
            } else if (grptype == TYP2AGRP) {
                const int index = 4 * si4703_registers[RDSB] & 0x000F;
                int ix = 0;
                if (index == 0) {
                    setSongInfo(decodedSong);
                    memset(decodedSong, 0, sizeof(decodedSong));
                }
                decodedSong[index + ix++] = Ch;
                decodedSong[index + ix++] = Cl;
                decodedSong[index + ix++] = Dh;
                decodedSong[index + ix++] = Dl;
            }

            QTest::qSleep(40);
        } else {
            QTest::qSleep(30);
        }
    }
    emit finished();
}
