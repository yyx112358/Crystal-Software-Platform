#pragma once
#include "global.h"
#include <QMainWindow>
#include "ui_Controller.h"

class FRAMEWORK_EXPORT Controller : public QMainWindow
{
	Q_OBJECT

public:
	Controller(QWidget *parent = Q_NULLPTR);
	~Controller();

private:
	Ui::Controller ui;

	void slot_Start();
};
