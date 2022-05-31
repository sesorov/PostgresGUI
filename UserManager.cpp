#include "UserManager.h"

void UserManager::addUser(QString databaseName, QString username, QString password)
{
	QString connectionString = QString("Driver={PostgreSQL Unicode};Server=localhost;Database=%1;Uid=%2;Pwd=%3;").arg(databaseName, username, password);

	// Get (or create) current users array from json
	QJsonObject dataObject = userData.object();
	QJsonArray usersArray = dataObject["users"].toArray();

	// Prepare new user info
	QJsonObject newUser;
	newUser["username"] = username;
	newUser["password"] = password;
	newUser["connectionString"] = connectionString;

	// Add new user
	usersArray.append(newUser);
	dataObject["users"] = usersArray;

	// Save data
	userData.setObject(dataObject);
	QFile jsonFile(fileName);
	jsonFile.open(QIODevice::WriteOnly);
	jsonFile.write(userData.toJson());
	jsonFile.close();
}

bool UserManager::checkCredentials(QString username, QString password)
{
	QJsonObject dataObject = userData.object();
	QJsonArray usersArray = dataObject["users"].toArray();
	bool exists = false;
	for(const auto userInfo: usersArray)
	{
		QJsonObject element = userInfo.toObject();
		if (element.value("username") == username && element.value("password") == password)
		{ 
			exists = true;
			break;
		}
	}
	return exists;
}

QJsonObject UserManager::getUserInfo(QString username, QString password)
{
	QJsonObject dataObject = userData.object();
	QJsonArray usersArray = dataObject["users"].toArray();
	QJsonObject user;
	for (const auto userInfo : usersArray)
	{
		QJsonObject element = userInfo.toObject();
		if (element.value("username") == username && element.value("password") == password)
		{
			return element;
		}
	}
	return user;
}

QJsonArray UserManager::getUsers() {
	return users;
}

UserManager::UserManager(QString fileName) : fileName(fileName) {
	// Opening JSON file
	QFileInfo fileInfo(fileName);
	QDir::setCurrent(fileInfo.path());
	QFile jsonFile(fileName);
	if (!jsonFile.open(QIODevice::ReadOnly)) { return; } // readonly

	QByteArray saveData = jsonFile.readAll();
	userData = QJsonDocument::fromJson(saveData);
	QJsonObject dataObject = userData.object();
	users = dataObject["users"].toArray();
}
