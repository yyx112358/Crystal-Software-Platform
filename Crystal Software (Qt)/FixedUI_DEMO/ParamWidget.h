#pragma once

#include <QDockWidget>
#include "ui_ParamWidget.h"
#include "ParamView.h"

//ParamView包装类，唯一作用是把ParamView放进一个DockWidget
class ParamWidget : public QDockWidget
{
	Q_OBJECT

public:
	ParamWidget(ParamView::ROLE role, QWidget *parent = Q_NULLPTR);
	~ParamWidget();

	void SetName(QString name);
	ParamView*const view = nullptr;//直接暴露指针，图个省事
private:

	Ui::ParamWidget ui;
};
