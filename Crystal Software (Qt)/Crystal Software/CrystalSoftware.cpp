#include "stdafx.h"
#include "CrystalSoftware.h"
#include "..\Algorithm\AlgorithmController.h"

CrystalSoftware::CrystalSoftware(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	AlgorithmController ac(nullptr);

	connect(ui.actionStart, &QAction::triggered, [this]()
	{
		AlgorithmController ac(this);
	});
}
