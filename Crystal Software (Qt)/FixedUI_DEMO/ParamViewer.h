#pragma once

#include <QDockWidget>
#include "ui_ParamViewer.h"

class ParamViewer : public QDockWidget
{
	Q_OBJECT

public:
	ParamViewer(QWidget *parent = Q_NULLPTR);
	~ParamViewer();

private:
	Ui::ParamViewer ui;
};
