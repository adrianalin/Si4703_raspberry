#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>

class UDPClient: public QObject
{
    Q_OBJECT

public:
    UDPClient();
    ~UDPClient();

public:
    void sendCommand(QByteArray dataToSend);

signals:
    void newFrequency(float frequency);

private slots:
    void onReadyRead();
    void onBytesWritten(qint64 len);

private:
    void processDatagram(QString datagram);

private:
    QUdpSocket m_clientUDPSocket;
};

#endif // UDPCLIENT_H
