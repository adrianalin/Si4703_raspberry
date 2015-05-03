#include "rdsinfothread.h"
#include "registers.h"
#include "QDebug"
#include <QTest>

RDSInfoThread::RDSInfoThread(ShadowRegistersHandling *handler, uint16_t *registers, QObject *parent):
    QObject(parent),
    si4703_registers(registers),
    m_handler(handler),
    m_stop(false)
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

void RDSInfoThread::setRadioStation(char *name)
{
    static quint8 i = 0;
    if (i == 5) {
        i = 0;
        QString string = QString::fromUtf8(name);
        m_decodedRadioStation = string;
        m_decodedRadioStation.clear();
    }
    i++;
}

void RDSInfoThread::setSong(char *name)
{
    static quint8 i = 0;
    QString string = QString::fromUtf8(name);
    if (string.length() > 0)
        m_decodedSong.append(string);
    if (m_decodedSong.length() > 25) {
        qDebug() << m_decodedSong << endl;
        m_decodedSong.clear();
    }
    i++;
}

void RDSInfoThread::process()
{
    qDebug() << "\nCheck RDS";
    m_stop = false;
    while (!m_stop) {
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
                    setSong(decodedSong);
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
