#pragma once
#include <QString>
#include <QSqlDatabase>
#include <QVector>
#include <QSqlQuery>
#include <qsqlerror.h>
#include <qstandarditemmodel.h>
#include <QMap>

class DBManager
{
	QString databaseName;
	QString tableName;
	QString connector;
public:
	DBManager(QString databaseName, QString tableName, QString connector) : databaseName(databaseName), tableName(tableName), connector(connector) {}
	DBManager() {};

	// General DB management options
	void createUser(QString username, QString password, bool admin);
	void clean();
	void create();
	void createTable();
	void drop();

	// Basic operations
	bool insert();
	bool remove();
	bool update();
	void search();
	QStandardItemModel* getAll();

	// Other
	QSqlDatabase getDatabase(QString customDBName = NULL);
	QString getDatabaseName();
};
