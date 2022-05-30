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

	// General DB management options
	void createUser(QString username, QString password, bool admin);
	void clean();
	void create();
	void drop();

	// Basic operations
	bool insert();
	bool remove();
	bool update();
	void search();
	void getAll();
};
