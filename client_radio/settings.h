#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QDateTime>

class Settings: public QObject
{
    Q_OBJECT

public:
    Settings(QObject* parent=0);
    ~Settings();

    QDateTime alarm() const;
    void setAlarm(const QDateTime &alarm);

    int volume() const;
    void setVolume(int volume);

    bool save();
    bool load();

    float selectedRadioStation() const;
    void setSelectedRadioStation(float selectedRadioStation);

private:
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

private:
    QDateTime m_alarm;
    int m_volume;
    float m_selectedFrequency;
};

#endif // SETTINGS_H
