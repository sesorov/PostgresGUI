#pragma once

#include <QMap>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QJsonDocument>


class UserManager
{
    QJsonDocument userData;
    QJsonArray users;
    QString fileName;
public:
    QJsonArray getUsers();
    UserManager(QString);
    void addUser(QString username, QString connectionString);
};

