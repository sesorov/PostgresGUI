#include "PostgresGUI.h"
#include "DBManager.h"


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

PostgresGUI::PostgresGUI(QWidget *parent)
    : QMainWindow(parent), connections(QListWidget()), userManager(UserManager(QString("F:\\VS Projects\\PostgresGUI\\userData.json")))
{
    this->model = new QStandardItemModel();
    ui.setupUi(this);
}

PostgresGUI::~PostgresGUI()
{
    if (!db.isOpen()) { db.close(); }
}
