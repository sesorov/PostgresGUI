#include "DBManager.h"

QSqlDatabase DBManager::getDatabase(QString customDBName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setPassword(user.value("password").toString());
    db.setUserName(user.value("username").toString());
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
    QString statementRights = isAdmin ? QString("GRANT ALL PRIVILEGES ON DATABASE %1 TO %2;\n"
                                                "GRANT ALL ON ALL TABLES IN SCHEMA public TO %2;\n"
                                                "GRANT ALL ON ALL SEQUENCES IN SCHEMA public TO %2;\n"
                                                "GRANT ALL ON ALL FUNCTIONS IN SCHEMA public TO %2;\n"
                                                "ALTER USER %2 WITH SUPERUSER;").arg(databaseName, username) : 
                                        QString("GRANT CONNECT ON DATABASE %1 TO %2;\n"
                                                "GRANT USAGE ON SCHEMA public TO %2;\n"
                                                "GRANT SELECT ON ALL TABLES IN SCHEMA public TO %2;\n"
                                                "GRANT SELECT ON ALL SEQUENCES IN SCHEMA public TO %2;\n").arg(databaseName, username);
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

bool DBManager::create()
{
    // Create database
    QSqlDatabase db = getDatabase("main");
    if (!db.isOpen()) db.open();
    QString statementCreateDatabase = QString("CREATE DATABASE %1;").arg(databaseName);
    QString statementExists = QString("select exists(SELECT datname FROM pg_catalog.pg_database WHERE lower(datname) = lower('%1'));").arg(databaseName);
    QSqlQuery query(db);

    query.exec(statementExists);
    bool exists = false;
    while (query.next()) {
        exists = query.value(0).toBool();
    }
    if (!exists)
    {
        bool output = query.exec(statementCreateDatabase);
        if (output) {
            qDebug() << "[SUCCESS] Connected to database " << databaseName << " successfully.";
            createTable();
            setDefaultProcedures();
            if (db.isOpen()) db.close();
        }
        else {
            qDebug() << "[ERROR] Database connection was not established: " + query.lastError().text();
            qDebug() << query.lastQuery();
        }
        // Create main table and set stored functions
        return output;
    }
    if (db.isOpen()) db.close();
    return exists;
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

void DBManager::setDefaultProcedures()
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) db.open();
    QSqlQuery query(db);

    // Insert data
    // SELECT InsertRecord('TestName2', 'TestSurname2', '+72224445566')
    QString statementInsertProcedure = QString("CREATE OR REPLACE FUNCTION InsertRecord(IN r_name text, IN r_surname text, IN r_phone text)\n"
                                               "RETURNS VOID AS\n$$\n"
                                               "BEGIN\nINSERT INTO %1(name,surname,phone) VALUES(r_name, r_surname, r_phone);\n"
                                               "END;\n$$\nlanguage plpgsql;").arg(tableName);
    QString statementSearchProcedure = QString("CREATE OR REPLACE FUNCTION Search(IN s_id INT DEFAULT -1, IN s_name text DEFAULT NULL, IN s_surname text DEFAULT NULL, IN s_phone text DEFAULT NULL)\n"
                                               "RETURNS TABLE(id INTEGER, name TEXT, surname TEXT, phone TEXT)\n"
                                               "AS\n$$\nBEGIN\nRETURN QUERY\n"
                                               "SELECT %1.id, %1.name, %1.surname, %1.phone FROM %1 WHERE ((s_id = -1 OR %1.id = s_id) AND (s_name = '' OR %1.name = s_name) "
                                               "AND (s_surname = '' OR %1.surname = s_surname) AND (s_phone = '' OR s_phone = %1.phone));\n"
                                               "END;\n$$\nlanguage plpgsql;").arg(tableName);
    QString statementUpdateProcedure = QString("CREATE OR REPLACE FUNCTION UpdateRecord(IN u_id INT DEFAULT -1, IN u_name text DEFAULT NULL, IN u_surname text DEFAULT NULL, IN u_phone text DEFAULT NULL)\n"
                                               "RETURNS VOID\nAS\n$$\nBEGIN\n"
                                               "IF u_id != -1 THEN\nUPDATE public.%1 SET\n"
                                               "name = COALESCE(NULLIF(u_name, ''), name),\n"
                                               "surname = COALESCE(NULLIF(u_surname, ''), surname),\n"
                                               "phone = COALESCE(NULLIF(u_phone, ''), phone)\n"
                                               "WHERE %1.id = u_id;\nELSE\nUPDATE public.%1 SET\n"
                                               "name = COALESCE(NULLIF(u_name, ''), name),\n"
                                               "surname = COALESCE(NULLIF(u_surname, ''), surname)\n"
                                               "WHERE %1.phone = u_phone;\nEND IF;\nEND;\n$$\nlanguage plpgsql;").arg(tableName);
    QString statementDeleteProcedure = QString("CREATE OR REPLACE FUNCTION DeleteRecord(IN d_id INT DEFAULT -1, IN d_name text DEFAULT NULL, IN d_surname text DEFAULT NULL, IN d_phone text DEFAULT NULL)\n"
                                               "RETURNS VOID\nAS\n$$\nBEGIN\n"
                                               "DELETE FROM %1 WHERE ((d_id = -1 OR %1.id = d_id) AND (d_name = '' OR %1.name = d_name)\n"
                                               "AND (d_surname = '' OR %1.surname = d_surname) AND (d_phone = '' OR d_phone = %1.phone));\n"
                                               "END;\n$$\nlanguage plpgsql;").arg(tableName);
    QString statementClearProcedure = QString("CREATE OR REPLACE FUNCTION Clear()\nRETURNS VOID\nAS\n$$\nBEGIN\nTRUNCATE %1;\nDELETE FROM %1;\nEND;\n$$\nlanguage plpgsql;").arg(tableName);

    if (query.exec(statementInsertProcedure)) {
        qDebug() << "[CONFIG] Added InsertRecord stored function.";
    }
    if (query.exec(statementSearchProcedure)) {
        qDebug() << "[CONFIG] Added Search stored function.";
    }
    if (query.exec(statementUpdateProcedure)) {
        qDebug() << "[CONFIG] Added UpdateRecord stored function.";
    }
    if (query.exec(statementDeleteProcedure)) {
        qDebug() << "[CONFIG] Added DeleteRecord stored function.";
    }
    if (query.exec(statementClearProcedure)) {
        qDebug() << "[CONFIG] Added Clear stored function.";
    }
}

bool DBManager::insert(QString name, QString surname, QString phone)
{
    QString statementInsert = QString("SELECT InsertRecord('%1', '%2', '%3')").arg(name, surname, phone);

    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) db.open();
    QSqlQuery query(db);

    return query.exec(statementInsert);
}

QStandardItemModel* DBManager::search(QString id, QString name, QString surname, QString phone)
{
    QStandardItemModel* records_match = new QStandardItemModel();
    QStringList labels = QString::fromUtf8("ID,Name,Surname,Phone").simplified().split(",");
    records_match->setHorizontalHeaderLabels(labels);
    records_match->setColumnCount(4);

    if (id.isEmpty()) id = QString("-1");

    QString statementSearch = QString("SELECT * FROM Search(%1, '%2', '%3', '%4')").arg(id, name, surname, phone);

    QSqlDatabase db = getDatabase();
    db.setDatabaseName(databaseName);
    if (!db.isOpen()) db.open();

    QSqlQuery query(db);
    bool status = query.exec(statementSearch);
    if (status)
    {
        qDebug() << "[SUCCESS] Found matching records.";
    }
    else
    {
        qDebug() << "[WARNING] Could not find records that satisfy provided query.";
        qDebug() << query.lastQuery();
        return records_match;
    }
    while (query.next()) {
        QList<QStandardItem*> items;

        items.append(new QStandardItem(query.value(0).toString()));
        items.append(new QStandardItem(query.value(1).toString()));
        items.append(new QStandardItem(query.value(2).toString()));
        items.append(new QStandardItem(query.value(3).toString()));
        records_match->appendRow(items);
    }
    db.close();
    return records_match;
}

bool DBManager::update(QString id, QString name, QString surname, QString phone)
{
    if (id.isEmpty()) id = QString("-1");

    QString statementUpdate = QString("SELECT UpdateRecord(%1, '%2', '%3', '%4')").arg(id, name, surname, phone);

    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) db.open();
    QSqlQuery query(db);

    return query.exec(statementUpdate);
}

bool DBManager::remove(QString id, QString name, QString surname, QString phone)
{
    if (id.isEmpty()) id = QString("-1");

    QString statementDelete = QString("SELECT DeleteRecord(%1, '%2', '%3', '%4')").arg(id, name, surname, phone);

    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) db.open();
    QSqlQuery query(db);

    return query.exec(statementDelete);
}

bool DBManager::drop()
{
    QSqlDatabase db = getDatabase();
    db.setDatabaseName("main");
    if (!db.isOpen()) db.open();
    QString dropOtherConnections = QString("SELECT pg_terminate_backend(pg_stat_activity.pid)\nFROM pg_stat_activity\nWHERE pg_stat_activity.datname = '%1'\nAND pid <> pg_backend_pid();").arg(databaseName);
    QString statementDropDatabase = QString("DROP DATABASE IF EXISTS %1;").arg(databaseName);
    QSqlQuery query(db);
    bool output = query.exec(dropOtherConnections) && query.exec(statementDropDatabase);
    if (output) {
        qDebug() << "[SUCCESS] Database " << databaseName << " has been dropped successfully.";
    }
    else {
        qDebug() << "[ERROR] Could not drop database:" + query.lastError().text();
        qDebug() << query.lastQuery();
    }
    db.close();
    return output;
}

bool DBManager::clean()
{
    QSqlDatabase db = getDatabase();
    db.setDatabaseName(databaseName);
    if (!db.isOpen()) db.open();

    QString statementClear = QString("SELECT Clear()");
    QSqlQuery query(db);
    bool status = query.exec(statementClear);
    db.close();
    return status;
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
