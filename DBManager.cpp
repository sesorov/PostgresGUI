#include "DBManager.h"

QSqlDatabase DBManager::getDatabase(QString customDBName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setPassword("3719");
    db.setUserName("postgres");
    db.setDatabaseName(customDBName.isEmpty() ? databaseName : customDBName);
    tableName = "main";
    return db;
}

QString DBManager::getDatabaseName()
{
    return databaseName;
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
        qDebug() << "[ERROR] Could not add user " << username << ": " + query.lastError().text();
        qDebug() << query.lastQuery();
    }
    db.close();
}

void DBManager::create()
{
    // Create database
    QSqlDatabase db = getDatabase("main");
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
    createTable();
    if (db.isOpen()) db.close();
}

void DBManager::createTable()
{
    QMap<QString, QString> columns;
    columns["id"] = "SERIAL PRIMARY KEY";
    columns["name"] = "TEXT NOT NULL";
    columns["surname"] = "TEXT NOT NULL";
    columns["phone"] = "TEXT NOT NULL UNIQUE";

    QString statementCreateTable = QString("CREATE TABLE %1 ( \n").arg(tableName);
    QString lastCol = columns.keys().back();
    for (auto column : columns.keys())
    {
        if (column != lastCol) statementCreateTable += QString("%1 %2,\n").arg(column, columns.value(column));
        else statementCreateTable += QString("%1 %2").arg(column, columns.value(column));
    }
    statementCreateTable += " \n );";

    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) db.open();
    QSqlQuery query(db);
    if (query.exec(statementCreateTable)) {
        qDebug() << "[SUCCESS] Created table " << tableName << " successfully.";
    }
    else {
        qDebug() << "[ERROR] Table creation error: " + query.lastError().text();
        qDebug() << query.lastQuery();
    }
    db.close();
}

void DBManager::drop()
{
    QSqlDatabase db = getDatabase();
    db.setDatabaseName("main");
    if (!db.isOpen()) db.open();
    QString dropOtherConnections = QString("SELECT pg_terminate_backend(pg_stat_activity.pid)\nFROM pg_stat_activity\nWHERE pg_stat_activity.datname = '%1'\nAND pid <> pg_backend_pid();").arg(databaseName);
    QString statementDropDatabase = QString("DROP DATABASE IF EXISTS %1;").arg(databaseName);
    QSqlQuery query(db);
    if (query.exec(dropOtherConnections) && query.exec(statementDropDatabase)) {
        qDebug() << "[SUCCESS] Database " << databaseName << " has been dropped successfully.";
    }
    else {
        qDebug() << "[ERROR] Could not drop database:" + query.lastError().text();
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

QStandardItemModel* DBManager::getAll()
{
    QStandardItemModel* records = new QStandardItemModel();
    QStringList labels = QString::fromUtf8("ID,Name,Surname,Phone").simplified().split(",");
    records->setHorizontalHeaderLabels(labels);
    records->setColumnCount(4);

    QSqlDatabase db = getDatabase();
    db.setDatabaseName(databaseName);
    if (!db.isOpen()) db.open();

    QString statementGetAll = QString("SELECT id,name,surname,phone FROM %1;").arg(tableName);
    QSqlQuery query(db);
    bool status = query.exec(statementGetAll);
    if (status)
    {
        qDebug() << "[SUCCESS] Accessed all records.";
    }
    else
    {
        qDebug() << "[ERROR] Could not read data: ";
        qDebug() << query.lastQuery();
        return records;
    }
    while (query.next()) {
        QList<QStandardItem*> items;

        items.append(new QStandardItem(query.value(0).toString()));
        items.append(new QStandardItem(query.value(1).toString()));
        items.append(new QStandardItem(query.value(2).toString()));
        items.append(new QStandardItem(query.value(3).toString()));
        records->appendRow(items);
    }
    db.close();
    return records;
}
