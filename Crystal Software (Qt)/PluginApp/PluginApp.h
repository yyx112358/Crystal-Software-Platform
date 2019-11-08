#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PluginApp.h"

class PluginApp : public QMainWindow
{
	Q_OBJECT

public:
	PluginApp(QWidget *parent = Q_NULLPTR);

private:
	Ui::PluginAppClass ui;
};
