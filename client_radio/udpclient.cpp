#include <QHostAddress>
#include "udpclient.h"
#include "commands.h"

#define SERVER_ADDRESS "10.0.0.1"
#define SERVER_PORT 1234

UDPClient::UDPClient()
{
    connect(&m_clientUDPSocket, &QUdpSocket::readyRead, this, &UDPClient::onReadyRead);
    connect(&m_clientUDPSocket, &QUdpSocket::bytesWritten, this, &UDPClient::onBytesWritten);
}

UDPClient::~UDPClient()
{
}

void UDPClient::onReadyRead()
{
    qDebug() << "onReadyRead";
    while (m_clientUDPSocket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_clientUDPSocket.pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_clientUDPSocket.readDatagram(datagram.data(), datagram.size(),
                                       &sender, &senderPort);
        qDebug() << "on ready read : " << datagram;
        processDatagram(datagram);
    }
}

void UDPClient::processDatagram(QString datagram)
{
    const QString command = datagram.left(datagram.indexOf('$'));
    qDebug() << "got command " << command;

    if (command == FREQUENCY_RESP) {
        const QString value = datagram.right(datagram.length() - datagram.indexOf('$') - 1);
        float frequency = value.toFloat() / 10;
        emit newFrequency(frequency);
    }
}

void UDPClient::sendCommand(QByteArray dataToSend)
{
    const QHostAddress serverAddress(SERVER_ADDRESS);
    qDebug() << "Command to send: " << dataToSend;
    m_clientUDPSocket.writeDatagram(dataToSend, serverAddress, SERVER_PORT);
}

void UDPClient::onBytesWritten(qint64 len)
{
    qDebug() << "On bytes written = " << len;
}
