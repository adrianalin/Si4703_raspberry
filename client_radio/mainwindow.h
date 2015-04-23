#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "settings.h"
#include "udpclient.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onVolumeSliderChanged(int val);
    void onStartPressed();
    void onStopPressed();
    void onSetAlarm();
    void onSeekUp();
    void onSeekDown();
    void onFrequecyChanged(float frequency);
    void onGoToFrequency(float frequency);

private:
    void loadFromSettings();
    void saveToSettings();

private:
    UDPClient m_UDPclient;
    Settings m_settings;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
