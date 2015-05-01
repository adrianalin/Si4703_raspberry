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
                    qDebug() << "Decoded radio station = " << m_decodedRadioStation;
                    memset(m_decodedRadioStation, 0, sizeof(m_decodedRadioStation));
                }
                m_decodedRadioStation[index * 2] = Dh;
                m_decodedRadioStation[index * 2 + 1] = Dl;
            } else if (grptype == TYP2AGRP) {
                const int index = 4 * si4703_registers[RDSB] & 0x000F;
                int ix = 0;
                if (index == 0) {
                    qDebug() << "Decoded music = " << m_decodedSong;
                    memset(m_decodedSong, 0, sizeof(m_decodedSong));
                }
                m_decodedSong[index + ix++] = Ch;
                m_decodedSong[index + ix++] = Cl;
                m_decodedSong[index + ix++] = Dh;
                m_decodedSong[index + ix++] = Dl;
            }

            QTest::qSleep(40);
        } else {
            QTest::qSleep(30);
        }
    }
    emit finished();
}
