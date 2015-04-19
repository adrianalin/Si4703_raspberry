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

#include "fmreceiver.h"
#include <QDebug>
#include <QCoreApplication>
#include <iostream>
#include <signal.h>
#include <unistd.h>

FMReceiver fmReceiver(0x10);

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        qDebug() << "Caught sigint; Stopping the Si4703";
        fmReceiver.stop();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        qDebug() << "\ncan't catch SIGINT\n";

    int val;
    while (1) {
        std::cout << "Seek: ";
        std::cin >> val;
        fmReceiver.seek(val);
    }

    return a.exec();
}
