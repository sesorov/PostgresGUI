#include "PostgresGUI.h"
#include "DBManager.h"


// Current user

void PostgresGUI::login()
{
    QString login_username = ui.usernameLineEdit->text();
    QString login_password = ui.passwordLineEdit->text();

    if (login_username.isEmpty() || login_password.isEmpty())
    {
        QMessageBox::warning(this, "Error: fill all fields", "Login and password are required.");
        return;
    }
    if (!this->userManager.checkCredentials(login_username, login_password))
    {
        QMessageBox::warning(this, "Wrong credentials", "Login and/or password did not match.");
        return;
    }

    this->currentUser = this->userManager.getUserInfo(login_username, login_password);
    QMessageBox::information(this, "Logged in", "Logged in successfully.");
}

// Create user

void PostgresGUI::addUser()
{
    QString new_username = ui.newUsernameLineEdit->text();
    QString new_password = ui.newPasswordLineEdit->text();
    bool isAdmin = ui.isAdminCheckBox->checkState();

    if (new_username.isEmpty() || new_password.isEmpty())
    {
        QMessageBox::warning(this, "Error: fill all fields", "Login and password are required.");
        return;
    }
    if (this->userManager.checkCredentials(new_username, new_password))
    {
        QMessageBox::warning(this, "User exists", "User with such credentials already exists.");
        return;
    }
    if (this->dbManager.getDatabaseName().isEmpty())
    {
        QMessageBox::warning(this, "No database selected", "Please, select database in <Database operations> for new user.");
        return;
    }
    if (this->currentUser.isEmpty() || !this->currentUser.value("isAdmin").toBool())
    {
        QMessageBox::warning(this, "Access restricted", "Please, log in with administrator permissions to create a new user.");
        return;
    }

    this->userManager.addUser(this->dbManager.getDatabaseName(), new_username, new_password, isAdmin);
    QMessageBox::information(this, "User created", "New user created successfully.");
}

// Database operations

void PostgresGUI::addDatabase()
{
    if (this->currentUser.isEmpty())
    {
        QMessageBox::warning(this, "Access restricted", "Please, log in to act with databases.");
        return;
    }
    QString databaseName = ui.databaseNameLineEdit->text();
    QString username = QString(this->currentUser.value("username").toString());
    QString password = QString(this->currentUser.value("password").toString());
    this->dbManager = DBManager(databaseName, QString("main"), this->userManager.getConnectionString(databaseName, username, password));
    this->dbManager.create();
    ui.tableView->setModel(this->dbManager.getAll());
    ui.tableView->setShowGrid(true);
    ui.tableView->show();
    QMessageBox::information(this, "Connected to database", "Connected to database successfully.");
}

void PostgresGUI::dropDatabase()
{
    if (this->currentUser.isEmpty() || !this->currentUser.value("isAdmin").toBool())
    {
        QMessageBox::warning(this, "Access restricted", "Please, log in with administrator permissions to drop database.");
        return;
    }
    if (this->dbManager.getDatabaseName().isEmpty())
    {
        QMessageBox::warning(this, "No database selected", "Please, select database in <Database operations> for removal.");
        return;
    }
    this->dbManager.drop();
    QMessageBox::information(this, "Database removed", "Removed database successfully.");
}

PostgresGUI::PostgresGUI(QWidget *parent)
    : QMainWindow(parent), connections(QListWidget()), userManager(UserManager(QString("F:\\VS Projects\\PostgresGUI\\userData.json")))
{
    ui.setupUi(this);
}

PostgresGUI::~PostgresGUI()
{
    if (!db.isOpen()) { db.close(); }
}
