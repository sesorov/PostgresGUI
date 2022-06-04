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
    void addUser(QString databaseName, QString username, QString password, bool isAdmin = false);
    bool checkCredentials(QString username, QString password);
    QJsonObject getUserInfo(QString username, QString password);
};
