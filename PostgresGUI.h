#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PostgresGUI.h"
#include <QListWidget>
#include <QProcess>
#include <QStandardItemModel>
#include <QTableView>
#include <QSql>
#include <qsqlerror.h>
#include <QtSql/qsqldatabase.h>
#include <QMessageBox>
#include <iostream>
#include <qsqlquery.h>
#include <qinputdialog>
#include <qpushbutton.h>
#include <QVBoxLayout>
#include <QLabel>
#include <qfile.h>
#include <qfiledialog.h>
#include <qjsondocument>
#include <qjsonobject>
#include <qjsonarray>

class PostgresGUI : public QMainWindow
{
    Q_OBJECT

public slots:
    void addUser();

public:
    PostgresGUI(QWidget *parent = Q_NULLPTR);
    ~PostgresGUI();

private:
    Ui::PostgresGUIClass ui;
};
