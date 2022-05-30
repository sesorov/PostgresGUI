#include "UserManager.h"

void UserManager::addUser(QString username, QString connectionString)
{
	// Get (or create) current users array from json
	QJsonObject dataObject = userData.object();
	QJsonArray usersArray = dataObject["users"].toArray();

	// Prepare new user info
	QJsonObject newUser;
	newUser["username"] = username;
	newUser["connectionString"] = connectionString;

	// Add new user
	usersArray.append(newUser);
	dataObject["users"] = usersArray;
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
