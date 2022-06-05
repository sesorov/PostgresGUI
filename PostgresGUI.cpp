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
    this->dbManager.createUser(new_username, new_password, isAdmin);
    QMessageBox::information(this, "User created", "New user created successfully.");
}

// Database operations

void PostgresGUI::addDatabase()
{
    QString databaseName = ui.databaseNameLineEdit->text();
    if (this->currentUser.isEmpty())
    {
        QMessageBox::warning(this, "Access restricted", "Please, log in to act with databases.");
        return;
    }
    if (databaseName.isEmpty())
    {
        QMessageBox::warning(this, "No database selected", "Please, type database name.");
        return;
    }
    QString username = QString(this->currentUser.value("username").toString());
    QString password = QString(this->currentUser.value("password").toString());
    this->dbManager = DBManager(databaseName, QString("main"), this->currentUser);
    this->dbManager.create();
    ui.tableView->setModel(this->dbManager.getAll());
    ui.tableView->setShowGrid(true);
    ui.tableView->show();
    QMessageBox::information(this, "Connected to database", "Connected to database successfully.");
}

void PostgresGUI::dropDatabase()
{
    if (this->currentUser.isEmpty())
    {
        QMessageBox::warning(this, "Not logged in", "Please, log in to perform actions with database.");
        return;
    }
    if (this->dbManager.getDatabaseName().isEmpty())
    {
        QMessageBox::warning(this, "No database selected", "Please, select database in <Database operations> for removal.");
        return;
    }
    if (this->dbManager.drop())
    {
        QMessageBox::information(this, "Database removed", "Removed database successfully.");
    }
    else
    {
        QMessageBox::warning(this, "Access restricted", "Please, log in with administrator permissions to drop database.");
    }
    return;
}

void PostgresGUI::clearDatabase()
{
    if (this->currentUser.isEmpty())
    {
        QMessageBox::warning(this, "Not logged in", "Please, log in to perform actions with database.");
        return;
    }
    if (this->dbManager.getDatabaseName().isEmpty())
    {
        QMessageBox::warning(this, "No database selected", "Please, select database in <Database operations> for cleanup.");
        return;
    }
    if (this->dbManager.clean())
    {
        ui.tableView->setModel(this->dbManager.getAll());
        ui.tableView->setShowGrid(true);
        ui.tableView->show();
        QMessageBox::information(this, "Database cleaned", "Removed all records successfully.");
        return;
    }
    QMessageBox::warning(this, "Access restricted", "Please, log in with administrator permissions to clean database.");
    return;
}

// Records operations

void PostgresGUI::addRecord()
{
    QString name = ui.nameLineEdit->text();
    QString surname = ui.surnameLineEdit->text();
    QString phone = ui.phoneLineEdit->text();
    if (name.isEmpty() || surname.isEmpty() || phone.isEmpty())
    {
        QMessageBox::warning(this, "Fill in all gaps", "Please, fill all the gaps (name, surname, phone).");
        return;
    }
    if (this->currentUser.isEmpty())
    {
        QMessageBox::warning(this, "Not logged in", "Please, log in to perform actions with database.");
        return;
    }
    if (this->dbManager.getDatabaseName().isEmpty())
    {
        QMessageBox::warning(this, "No database selected", "Please, select database in <Database operations> for inserting.");
        return;
    }
    bool status = dbManager.insert(name, surname, phone);
    if (!status)
    {
        QMessageBox::warning(this, "Access restricted: view-only", "Please, log in with administrator permissions to perform insert.");
        return;
    }
    ui.tableView->setModel(this->dbManager.getAll());
    ui.tableView->setShowGrid(true);
    ui.tableView->show();
    QMessageBox::information(this, "New record added", "New record added successfully.");
}

void PostgresGUI::search()
{
    QString id = ui.idLineEdit->text();
    QString name = ui.nameLineEdit->text();
    QString surname = ui.surnameLineEdit->text();
    QString phone = ui.phoneLineEdit->text();

    if (id.isEmpty() && name.isEmpty() && surname.isEmpty() && phone.isEmpty())
    {
        QMessageBox::warning(this, "No data provided", "Please, fill at least one gap to search by.");
        return;
    }
    if (this->currentUser.isEmpty())
    {
        QMessageBox::warning(this, "Log in", "Please, log in to perform search.");
        return;
    }
    if (this->dbManager.getDatabaseName().isEmpty())
    {
        QMessageBox::warning(this, "No database selected", "Please, select database in <Database operations> to search in.");
        return;
    }
    ui.tableView->setModel(this->dbManager.search(id, name, surname, phone));
    ui.tableView->setShowGrid(true);
    ui.tableView->show();
    QMessageBox::information(this, "Search successful", "See results in the display table. If it is empty, nothing was found.");
}

void PostgresGUI::updateRecord()
{
    // search by one of these fields
    QString id = ui.idLineEdit->text();
    QString phone = ui.phoneLineEdit->text();

    // update these fields (and phone from above) (ID cannot be changed)
    QString name = ui.nameLineEdit->text();
    QString surname = ui.surnameLineEdit->text();

    if (id.isEmpty() && phone.isEmpty())
    {
        QMessageBox::warning(this, "No data provided", "Please, fill any of two unique gaps (Phone or ID) to search by to update.");
        return;
    }
    if (this->currentUser.isEmpty())
    {
        QMessageBox::warning(this, "Log in", "Please, log in to perform update.");
        return;
    }
    if (this->dbManager.getDatabaseName().isEmpty())
    {
        QMessageBox::warning(this, "No database selected", "Please, select database in <Database operations> for updating.");
        return;
    }
    bool status = dbManager.update(id, name, surname, phone);
    if (!status)
    {
        QMessageBox::warning(this, "Update error", "Please, check if the record with provided phone/id exists and you have administrator permissions.");
        return;
    }
    ui.tableView->setModel(this->dbManager.getAll());
    ui.tableView->setShowGrid(true);
    ui.tableView->show();
    QMessageBox::information(this, "Update successful", "Successfully updated record.");
}

void PostgresGUI::deleteRecord()
{
    // Delete by one/combination of these
    QString id = ui.idLineEdit->text();
    QString phone = ui.phoneLineEdit->text();
    QString name = ui.nameLineEdit->text();
    QString surname = ui.surnameLineEdit->text();

    if (id.isEmpty() && phone.isEmpty() && name.isEmpty() && surname.isEmpty())
    {
        QMessageBox::warning(this, "No data provided", "Please, fill at least one gap (name, surname, phone or ID) to search by to delete.");
        return;
    }
    if (this->currentUser.isEmpty())
    {
        QMessageBox::warning(this, "Log in", "Please, log in to perform removal.");
        return;
    }
    if (this->dbManager.getDatabaseName().isEmpty())
    {
        QMessageBox::warning(this, "No database selected", "Please, select database in <Database operations> for removal.");
        return;
    }
    bool status = dbManager.remove(id, name, surname, phone);
    if (!status)
    {
        QMessageBox::warning(this, "Delete error", "Please, check if the record with provided phone/id exists and you have administrator permissions.");
        return;
    }
    ui.tableView->setModel(this->dbManager.getAll());
    ui.tableView->setShowGrid(true);
    ui.tableView->show();
    QMessageBox::information(this, "Update successful", "Successfully updated record.");
}

// Constructor/destructor

PostgresGUI::PostgresGUI(QWidget *parent)
    : QMainWindow(parent), connections(QListWidget()), userManager(UserManager(QString("F:\\VS Projects\\PostgresGUI\\userData.json")))
{
    ui.setupUi(this);
}

PostgresGUI::~PostgresGUI()
{
    if (db.isOpen()) { db.close(); }
}
