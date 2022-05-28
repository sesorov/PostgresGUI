#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PostgresGUI.h"

class PostgresGUI : public QMainWindow
{
    Q_OBJECT

public:
    PostgresGUI(QWidget *parent = Q_NULLPTR);

private:
    Ui::PostgresGUIClass ui;
};
