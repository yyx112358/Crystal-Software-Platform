#pragma once

#include <QDockWidget>
#include "ui_ParamWidget.h"
#include "ParamView.h"

//ParamView��װ�࣬Ψһ�����ǰ�ParamView�Ž�һ��DockWidget
class ParamWidget : public QDockWidget
{
	Q_OBJECT

public:
	ParamWidget(ParamView::ROLE role, QWidget *parent = Q_NULLPTR);
	~ParamWidget();

	void SetName(QString name);
	ParamView*const view = nullptr;//ֱ�ӱ�¶ָ�룬ͼ��ʡ��
private:

	Ui::ParamWidget ui;
};
