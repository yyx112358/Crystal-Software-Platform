#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CrystalSoftware.h"

class CrystalSoftware : public QMainWindow
{
	Q_OBJECT

public:
	CrystalSoftware(QWidget *parent = Q_NULLPTR);

private:
	Ui::CrystalSoftwareClass ui;
};
