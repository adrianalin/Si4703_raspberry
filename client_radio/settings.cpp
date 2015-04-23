#include <QDebug>
#include <QString>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include "commands.h"
#include "settings.h"

Settings::Settings(QObject *parent):
    QObject(parent),
    m_alarm(QDateTime()),
    m_volume(0),
    m_selectedFrequency(.0)
{

}

Settings::~Settings()
{

}

QDateTime Settings::alarm() const
{
    return m_alarm;
}

void Settings::setAlarm(const QDateTime &alarm)
{
    m_alarm = alarm;
}
int Settings::volume() const
{
    return m_volume;
}

void Settings::setVolume(int volume)
{
    m_volume = volume;
}

float Settings::selectedRadioStation() const
{
    return m_selectedFrequency;
}

void Settings::setSelectedRadioStation(float selectedRadioStation)
{
    m_selectedFrequency = selectedRadioStation;
}

void Settings::read(const QJsonObject &json)
{
    if (json.contains(VOLUME_CMD)) {
        m_volume = json[VOLUME_CMD].toInt();
    } else {
        qWarning() << "Json does not contain key" << VOLUME_CMD;
    }
    if (json.contains(ALARM_CMD)) {
        m_alarm = QDateTime::fromString(json[ALARM_CMD].toString());
    } else {
        qWarning() << "Json does not contain key " << ALARM_CMD;
    }
    if (json.contains(FREQUENCY_RESP)) {
        m_selectedFrequency = json[FREQUENCY_RESP].toDouble();
    } else {
        qWarning() << "Json does not contain key " << FREQUENCY_RESP;
    }
}

void Settings::write(QJsonObject &json) const
{
    json[VOLUME_CMD] = m_volume;
    json[ALARM_CMD] = m_alarm.toString();
    json[FREQUENCY_RESP] = m_selectedFrequency;
}

bool Settings::save()
{
    QFile saveFile(QStringLiteral("save.json"));
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }
    QJsonObject saveObject;
    write(saveObject);
    QJsonDocument saveDoc(saveObject);
    saveFile.write(saveDoc.toJson());

    return true;
}

bool Settings::load()
{
    QFile loadFile(QStringLiteral("save.json"));
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }
    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    read(loadDoc.object());

    return true;
}
