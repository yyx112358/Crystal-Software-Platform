#pragma once

#include "global.h"
#include <QtWidgets/QMainWindow>
#include "ui_Controller.h"


class Controller : public QMainWindow
{
	Q_OBJECT

public:
	Controller(QWidget *parent = Q_NULLPTR);

private:
	Ui::ControllerClass ui;
};
