#include <cstdlib>
#include <unistd.h>
#include <QDebug>
#include <QtTest/QTest>
#include "fmreceiver.h"
#include "registers.h"

FMReceiver::FMReceiver(QObject *parent):
    QObject(parent),
    m_frequency(0),
    m_volume(0),
    m_started(false)
{
    start();
    // init done; now play something
    goToChannel();
    setVolume();
}

FMReceiver::~FMReceiver()
{
    qDebug() << "Destroying fmReceiver";
    stop();
    if (m_thread)
        m_thread->wait(1000);
}

void FMReceiver::startAlarm()
{
    qDebug() << "\nstartAlarm";
    if (m_started) {
        qDebug() << "Radio already running!";
        return ;
    }

    // start the fm receiver
    start();
    // now play last radio station, with last volume  setup
    setVolume(m_volume);
    goToChannel(m_frequency);
}

void FMReceiver::start()
{
    qDebug() << "\nstart";
    if (m_started) {
        qDebug() << "Radio already running!";
        return;
    }
    m_started = true;

    set2WireMode();
    initSI4703();

    // si4703 initialized, start the RDS thread to get play station info
    m_thread = new QThread;
    m_RDSWorker = new RDSInfoThread(&m_handler, si4703_registers);
    m_RDSWorker->moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), m_RDSWorker, SLOT(process()));
    connect(m_RDSWorker, SIGNAL(finished()), m_thread, SLOT(quit()));
    connect(m_RDSWorker, SIGNAL(finished()), m_RDSWorker, SLOT(deleteLater()));
    connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
    m_thread->start();
}

void FMReceiver::stop()
{
    qDebug() << "\nstop";
    if (!m_started) {
        qDebug() << "Radio already stopped!";
        return;
    }
    m_started = false;
    // Clear the DMUTE bit to enable mute.
    // Set the ENABLE bit high and DISABLE bit high to set the powerdown state.
    m_handler.readRegisters(si4703_registers);
    si4703_registers[POWERCFG] = 0x0041;
    m_handler.updateRegisters(si4703_registers);

    // stop the RDS thread
    m_RDSWorker->stop(true);
}

void FMReceiver::set2WireMode()
{
    qDebug() << "\nset2WireMode";
    // GPIO23 on RPI -> RST on SI4703
    // Set up GPIO 23 and set to output
    system("echo \"23\" > /sys/class/gpio/export");
    system("echo \"out\" > /sys/class/gpio/gpio23/direction");

    // Write output
    system("echo \"0\" > /sys/class/gpio/gpio23/value");
    QTest::qSleep(100);
    system("echo \"1\" > /sys/class/gpio/gpio23/value");
    QTest::qSleep(100);
}

void FMReceiver::initSI4703()
{
    qDebug() << "\nInit IC";

    m_handler.readRegisters(si4703_registers); // Read the current register set
    si4703_registers[0x07] = 0x8100; // Enable the oscillator
    m_handler.updateRegisters(si4703_registers); // Update

    QTest::qSleep(500); // Wait for clock to settle - from AN230 page 9

    m_handler.readRegisters(si4703_registers); // Read the current register set
    si4703_registers[POWERCFG] = 0x4001; // Enable the IC
    si4703_registers[SYSCONFIG1] |= (1 << RDS); // Enable RDS

#ifdef IN_EUROPE
    si4703_registers[SYSCONFIG1] |= (1 << DE); // 50kHz Europe setup
    si4703_registers[SYSCONFIG2] |= (1 << SPACE0); // 100kHz channel spacing for Europe
#else
    si4703_registers[SYSCONFIG2] &= ~(1 << SPACE1 | 1 << SPACE0) ; // Force 200kHz channel spacing for USA
#endif

    si4703_registers[SYSCONFIG2] &= 0xFFF0; // Clear volume bits
    si4703_registers[SYSCONFIG2] |= 0x0001; // Set volume to lowest
    m_handler.updateRegisters(si4703_registers); // Update

    QTest::qSleep(110); // Max powerup time, from datasheet page 13
}

// Reads the current channel from READCHAN
// Returns a number like 973 for 97.3MHz
int FMReceiver::readChannel()
{
    m_handler.readRegisters(si4703_registers);
    int frequency = si4703_registers[READCHAN] & 0x03FF; // Mask out everything but the lower 10 bits

#ifdef IN_EUROPE
    //Freq(MHz) = 0.100(in Europe) * Channel + 87.5MHz
    //X = 0.1 * Chan + 87.5
    frequency *= 1; //98 * 1 = 98 - I know this line is silly, but it makes the code look uniform
#else
    //Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
    //X = 0.2 * Chan + 87.5
    channel *= 2; //49 * 2 = 98
#endif

    frequency += 875; //98 + 875 = 973

    if (m_frequency != frequency) {
        m_frequency = frequency;
        emit frequencyChanged(m_frequency);
    }

    return(frequency);
}

void FMReceiver::setVolume(const quint8 value)
{
    qDebug() << "\nSet volume to " << value;
    m_volume = value;
    if (value > 15) {
        qDebug() << "Cannot set volume. Must be between [0, 15]";
        return;
    }

    m_handler.readRegisters(si4703_registers);
    si4703_registers[SYSCONFIG2] &= 0xFFF0; // Clear volume bits
    si4703_registers[SYSCONFIG2] |= value;
    m_handler.updateRegisters(si4703_registers);
}

// Seeks out the next available station
// Returns the freq if it made it
// Returns zero if failed
bool FMReceiver::seek(quint8 seekDirection)
{
    qDebug() << "\n Seek " << seekDirection;
    m_handler.readRegisters(si4703_registers);

    //Set seek mode wrap bit
    si4703_registers[POWERCFG] |= (1 << SKMODE); //Allow wrap
//    si4703_registers[POWERCFG] &= ~(1 << SKMODE); //Disallow wrap - if you disallow wrap, you may want to tune to 87.5 first

    if (seekDirection == SEEK_DOWN)
        si4703_registers[POWERCFG] &= ~(1 << SEEKUP); //Seek down is the default upon reset
    else
        si4703_registers[POWERCFG] |= 1 << SEEKUP; //Set the bit to seek up

    si4703_registers[POWERCFG] |= (1 << SEEK); //Start seek

    m_handler.updateRegisters(si4703_registers); //Seeking will now start

    //Poll to see if STC is set
    while (1) {
        m_handler.readRegisters(si4703_registers);
        if ((si4703_registers[STATUSRSSI] & (1 << STC)) != 0)
            break; //Tuning complete!
    }
    qDebug() << "Trying station: " << readChannel();

    m_handler.readRegisters(si4703_registers);
    int valueSFBL = si4703_registers[STATUSRSSI] & (1 << SFBL); //Store the value of SFBL
    si4703_registers[POWERCFG] &= ~(1 << SEEK); //Clear the seek bit after seek has completed
    m_handler.updateRegisters(si4703_registers);

    //Wait for the si4703 to clear the STC as well
    for (int i = 0; i < 200; i++) {
        m_handler.readRegisters(si4703_registers);
        if ((si4703_registers[STATUSRSSI] & (1 << STC)) == 0)
            break; //Tuning complete!
        qDebug() << "Waiting...";
    }

    if (valueSFBL) { //The bit was set indicating we hit a band limit or failed to find a station
        qDebug() << "Seek limit hit"; //Hit limit of band during seek
        return(FAIL);
    }

    qDebug() << "Channel set to " << readChannel();
    return(SUCCESS);
}

// Given a channel, tune to it
// Channel is in MHz, so 973 will tune to 97.3MHz
// Note: gotoChannel will go to illegal channels (ie, greater than 110MHz)
// It's left to the user to limit these if necessary
// Actually, during testing the Si4703 seems to be internally limiting it at 87.5. Neat.
void FMReceiver::goToChannel(const unsigned int value)
{
    if (value < 875 || value > 1080) {
        qWarning() << "Frequency " << value << " outside of band [87.5 - 108MHz]";
        return;
    }
    qDebug() << "\nGo to channel " << value;
    // write channel reg 03h
    int newChannel = value;
    newChannel *= 10;
    newChannel -= 8750;

#ifdef IN_EUROPE
    newChannel /= 10; //980 / 10 = 98
#else
    newChannel /= 20; //980 / 20 = 49
#endif

    // These steps come from AN230 page 20 rev 0.5
    m_handler.readRegisters(si4703_registers);
    si4703_registers[CHANNEL] &= 0xFE00; // Clear out the channel bits
    si4703_registers[CHANNEL] |= newChannel; // Mask in the new channel
    si4703_registers[CHANNEL] |= (1 << TUNE); // Set the TUNE bit to start
    m_handler.updateRegisters(si4703_registers);

    // Poll to see if STC is set
    while (1)  {
        QTest::qSleep(500);
        m_handler.readRegisters(si4703_registers);
        if ((si4703_registers[STATUSRSSI] & (1 << STC)) != 0)
            break; // Tuning complete!
        qDebug() << "Tuning...";
    }

    m_handler.readRegisters(si4703_registers);
    si4703_registers[CHANNEL] &= ~(1 << TUNE); // Clear the tune after a tune has completed
    m_handler.updateRegisters(si4703_registers);

    // Wait for the si4703 to clear the STC as well
    for (int i = 0; i < 200; i++)  {
        m_handler.readRegisters(si4703_registers);
        if ((si4703_registers[STATUSRSSI] & (1 << STC)) == 0)
            break; //Tuning complete!
        qDebug() << "Waiting...";
    }

    qDebug() << "Channel set to " << readChannel();
}
