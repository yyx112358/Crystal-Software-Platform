#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FixedUI_DEMO.h"

class FixedUI_DEMO : public QMainWindow
{
	Q_OBJECT

public:
	FixedUI_DEMO(QWidget *parent = Q_NULLPTR);

private:
	Ui::FixedUI_DEMOClass ui;
};
