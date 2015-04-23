#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QTime>
#include <QDateTime>

class RadioServer : public QObject
{
    Q_OBJECT

public:
    RadioServer(QObject* parent = 0);
    ~RadioServer();

signals:
    void volumeChanged(quint8 val);
    void stopped();
    void started();
    void startAlarm(QDateTime targetDateTime);
    void seek(quint8 upDown);
    void goToChannel(unsigned int frequency);

private slots:
    void onReadyRead();
    void onFrequencyChanged(int frequency);

private:
    void sendCommand(QByteArray dataToSend);

private:
    void processDatagram(QString datagram);
    QHostAddress m_clientAddress;
    quint16 m_clientPort;
    QHostAddress m_address;
    QUdpSocket m_UDPServerSocket;
};

#endif // SERVER_H
