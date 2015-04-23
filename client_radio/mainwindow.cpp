#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commands.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

    connect(ui->volumeSlider, &QSlider::valueChanged, this, &MainWindow::onVolumeSliderChanged);
    connect(ui->startButton, &QPushButton::pressed, this, &MainWindow::onStartPressed);
    connect(ui->stopButton, &QPushButton::pressed, this, &MainWindow::onStopPressed);
    connect(ui->alarmButton, &QPushButton::pressed, this, &MainWindow::onSetAlarm);
    connect(ui->seekDownButton, &QPushButton::pressed, this, &MainWindow::onSeekDown);
    connect(ui->seekUpButton, &QPushButton::pressed, this, &MainWindow::onSeekUp);
    connect(&m_UDPclient, &UDPClient::newFrequency, this, &MainWindow::onFrequecyChanged);

    loadFromSettings();
}

MainWindow::~MainWindow()
{
    qDebug() << "Destroy obbbbbjjjject";
    saveToSettings();
    delete ui;
}

void MainWindow::saveToSettings()
{
    m_settings.setAlarm(ui->dateTimeEdit->dateTime());
    m_settings.setVolume(ui->volumeSlider->value());
    m_settings.setSelectedRadioStation(ui->frequencyLineEdit->text().toFloat());

    m_settings.save();
}

void MainWindow::loadFromSettings()
{
    m_settings.load();

    // start the fm receiver
    onStartPressed();

    // load volume
    ui->volumeSlider->setValue(m_settings.volume());

    // load alarm time
    ui->dateTimeEdit->setDateTime(m_settings.alarm());
    emit ui->alarmButton->pressed(); // to trigger the command send

    // load the selected radio station
    ui->frequencyLineEdit->setText(QString::number(m_settings.selectedRadioStation()));
    onGoToFrequency(ui->frequencyLineEdit->text().toFloat());
}

void MainWindow::onGoToFrequency(float frequency)
{
    const QString command = CHANNEL_CMD + QString("$") + QString::number(frequency);
    m_UDPclient.sendCommand(command.toLatin1());
}

void MainWindow::onSetAlarm()
{
    const QString command = ALARM_CMD + QString("$") + ui->dateTimeEdit->dateTime().toString();
    m_UDPclient.sendCommand(command.toLatin1());
}

void MainWindow::onStartPressed()
{
    const QString command = START_CMD + QString("$");
    m_UDPclient.sendCommand(command.toLatin1());
}

void MainWindow::onStopPressed()
{
    const QString command = STOP_CMD + QString("$");
    m_UDPclient.sendCommand(command.toLatin1());
}

void MainWindow::onVolumeSliderChanged(int val)
{
    const QString command = VOLUME_CMD + QString("$") + QString::number(val);
    m_UDPclient.sendCommand(command.toLatin1());
}

void MainWindow::onSeekDown()
{
    const QString command = SEEK_CMD + QString("$") + QString::number(0);
    m_UDPclient.sendCommand(command.toLatin1());
}

void MainWindow::onSeekUp()
{
    const QString command = SEEK_CMD + QString("$") + QString::number(1);
    m_UDPclient.sendCommand(command.toLatin1());
}

void MainWindow::onFrequecyChanged(float frequency)
{
    ui->frequencyLineEdit->setText(QString::number(frequency));
}
