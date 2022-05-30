#include "DBManager.h"

QSqlDatabase getDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setPassword("3719");
    db.setUserName("postgres");
    return db;
}

void DBManager::createUser(QString username, QString password, bool isAdmin)
{
    QSqlDatabase db = getDatabase();
    QString statementCreateUser = QString("CREATE USER %1 WITH ENCRYPTED PASSWORD '%2';").arg(username, password);
    QString statementRights = isAdmin ? QString("GRANT ALL PRIVILEGES ON DATABASE %1 TO %2").arg(databaseName, username) : QString("GRANT SELECT ON %1 TO %2;").arg(tableName, username);
    if (!db.isOpen()) db.open();
    QSqlQuery query(db);
    if (query.exec(statementCreateUser) && query.exec(statementRights)) {
        qDebug() << "[SUCCESS] User " << username << " was added successfully. Admin: " << (isAdmin ? "yes." : "no.");
    }
    else {
        qDebug() << "[ERROR] Could add user " << username << ": " + query.lastError().text();
        qDebug() << query.lastQuery();
    }
    db.close();
}

void DBManager::create()
{
    // Create database
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) db.open();
    QString statementCreateDatabase = QString("CREATE DATABASE %1;").arg(databaseName);
    QSqlQuery query(db);
    if (query.exec(statementCreateDatabase)) {
        qDebug() << "[SUCCESS] Connected to database " << databaseName << " successfully.";
    }
    else {
        qDebug() << "[ERROR] Database connection was not established: " + query.lastError().text();
        qDebug() << query.lastQuery();
    }
    // Create main table
    db.setDatabaseName(databaseName);
    QString statementCreateTable = QString("CREATE TABLE %1 \n").arg(tableName);
    if (query.exec(statementCreateTable)) {
        qDebug() << "[SUCCESS] Created table " << tableName << " successfully.";
    }
    else {
        qDebug() << "[ERROR] Table creation error: " + query.lastError().text();
        qDebug() << query.lastQuery();
    }
    db.close();
}

void DBManager::clean()
{
    QSqlDatabase db = getDatabase();
    db.setDatabaseName(databaseName);
    if (!db.isOpen()) db.open();

    QString statementClear = QString("DELETE * FROM %1;").arg(tableName);
    QSqlQuery query(db);
    if (query.exec(statementClear)) {
        qDebug() << "[SUCCESS] Table " << tableName << " has been cleaned successfully.";
    }
    else {
        qDebug() << "[ERROR] Could not clear table " << tableName << ": " + query.lastError().text();
        qDebug() << query.lastQuery();
    }
    db.close();
}
