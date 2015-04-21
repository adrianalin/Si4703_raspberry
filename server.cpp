#include "server.h"

#define MY_ADDRESS "10.0.0.1"
#define PORT 1234

RadioServer::RadioServer(QObject *parent) : QObject(parent)
{
    qDebug() << "Start server";
    m_address.setAddress(MY_ADDRESS);
    if (m_UDPServerSocket.bind(m_address, PORT)) {
//        qDebug() << "Failed to bind";
//        exit(1);
    }
    connect(&m_UDPServerSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

RadioServer::~RadioServer()
{
}

void RadioServer::onFrequencyChanged(int frequency)
{
    const QString command = "Frequency$" + QString::number(frequency);
    sendCommand(command.toLatin1());
}

void RadioServer::sendCommand(QByteArray dataToSend)
{
    qDebug() << "Command to send: " << dataToSend;
    m_UDPServerSocket.writeDatagram(dataToSend, m_clientAddress, m_clientPort);
}

void RadioServer::onReadyRead()
{
    qDebug() << "onReadyRead";
    while (m_UDPServerSocket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_UDPServerSocket.pendingDatagramSize());
        m_UDPServerSocket.readDatagram(datagram.data(), datagram.size(),
                                       &m_clientAddress, &m_clientPort);
        qDebug() << "on ready read : " << datagram;
        processDatagram(datagram);
    }
}

void RadioServer::processDatagram(QString datagram)
{
    const QString command = datagram.left(datagram.indexOf('$'));
    qDebug() << "got command " << command;

    if (command == "Volume") {
        const QString value = datagram.right(datagram.length() - datagram.indexOf('$') - 1);
        emit volumeChanged(value.toUInt());
    } else if (command == "Stop") {
        emit stopped();
    } else if (command == "Start") {
        emit started();
    } else if (command == "Alarm") {
        const QString value = datagram.right(datagram.length() - datagram.indexOf('$') - 1);
        const QDateTime targetDateTime = QDateTime::fromString(value);
        qDebug() << "Got time " << targetDateTime.toString();
        emit startAlarm(targetDateTime);
    } else if (command == "Seek") {
        const QString value = datagram.right(datagram.length() - datagram.indexOf('$') - 1);
        qDebug() << "value = " << value;
        emit seek(value.toInt());
    }
}
