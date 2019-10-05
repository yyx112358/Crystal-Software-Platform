#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TestAnything.h"

class TestAnything : public QMainWindow
{
	Q_OBJECT

public:
	TestAnything(QWidget *parent = Q_NULLPTR);

private:
	Ui::TestAnythingClass ui;
};
