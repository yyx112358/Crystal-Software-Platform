#include "stdafx.h"
#include "Controller.h"
#include <QMessageBox>
#include "GraphError.h"

Controller::Controller(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.actionStart, &QAction::triggered, this, &Controller::slot_Start);
}

Controller::~Controller()
{
}

void Controller::slot_Start()
{
	try
	{
		GRAPH_NOT_IMPLEMENT;
	}
	catch (GraphError&e)
	{
		QMessageBox::information(this, "Error", e.msg);
	}
}
