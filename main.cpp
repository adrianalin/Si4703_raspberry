//Python Version 2 code
//The SI4703 reads starting at address 0x0A and starts writing to 0x02

//MANY THANKS to Nathan Seidle and his sketch example code at Sparkfun! I would have been lost without it.
//http://www.sparkfun.com/datasheets/BreakoutBoards/Si4703_Example.pde
//Here is the SI4703 Datasheet that explains the registers
//http://www.sparkfun.com/datasheets/BreakoutBoards/Si4702-03-C19-1.pdf

//Here is the SI4703 programming guide
//http://www.silabs.com/Support%20Documents/TechnicalDocs/AN230.pdf
//Anytime you READ the regsters it writes the command byte to the first byte of the
//SI4703 0x02 register. And then uses the list to write the 2nd bytes of register 0x02
//and continues until all the list is written!

//So the reg = i2c.read_i2c_block_data(address, zz, 32)  command
//writes the integer zz to the first byte of register 0x02.
// so after setting the first byte of 0x02 to what you want it to be you need to
//make sure every write or read keeps populating that byte with what you need there

#include <QCoreApplication>
#include <QObject>
#include <signal.h>
#include <unistd.h>
#include "server.h"
#include "fmreceiver.h"
#include "alarm.h"

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        qDebug() << "Caught sigint; Stopping the Si4703";
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    RadioServer rServer;
    FMReceiver fmReceiver;
    Alarm mAlarm;

    QObject::connect(&rServer, SIGNAL(volumeChanged(quint8)), &fmReceiver, SLOT(setVolume(quint8)));
    QObject::connect(&rServer, SIGNAL(started()), &fmReceiver, SLOT(start()));
    QObject::connect(&rServer, SIGNAL(stopped()), &fmReceiver, SLOT(stop()));
    QObject::connect(&rServer, SIGNAL(seek(quint8)), &fmReceiver, SLOT(seek(quint8)));

    QObject::connect(&fmReceiver, SIGNAL(frequencyChanged(int)), &rServer, SLOT(onFrequencyChanged(int)));

    QObject::connect(&rServer, SIGNAL(startAlarm(QDateTime)), &mAlarm, SLOT(start(QDateTime)));
    QObject::connect(&mAlarm, SIGNAL(triggered()), &fmReceiver, SLOT(start()));

    QObject::connect(&rServer, SIGNAL(goToChannel(uint)), &fmReceiver, SLOT(goToChannel(uint)));

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        qDebug() << "\ncan't catch SIGINT\n";

    return a.exec();
}
