#pragma once
#include <QString>
#include <QSqlDatabase>
#include <QVector>
#include <QSqlQuery>
#include <qsqlerror.h>
#include <qstandarditemmodel.h>
#include <QMap>
#include <qjsonobject>

class DBManager
{
	QString databaseName;
	QString tableName;
	QJsonObject user;
public:
	DBManager(QString databaseName, QString tableName, QJsonObject user) : databaseName(databaseName), tableName(tableName), user(user) {}
	DBManager() {};

	// General DB management options
	void createUser(QString username, QString password, bool admin);
	void clean();
	bool create();
	void createTable();
	bool drop();

	// Basic operations
	bool insert(QString name, QString surname, QString phone);
	bool remove();
	bool update(QString id = NULL, QString name = NULL, QString surname = NULL, QString phone = NULL);
	QStandardItemModel* search(QString id = NULL, QString name = NULL, QString surname = NULL, QString phone = NULL);
	QStandardItemModel* getAll();

	// Other
	QSqlDatabase getDatabase(QString customDBName = NULL);
	QString getDatabaseName();
	void setDefaultProcedures();
};
