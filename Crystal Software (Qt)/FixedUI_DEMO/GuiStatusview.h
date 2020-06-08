#pragma once

#include <QWidget>
#include "ui_GuiStatusview.h"

class GuiStatusview : public QWidget
{
	Q_OBJECT

public:
	GuiStatusview(QWidget *parent = Q_NULLPTR);
	~GuiStatusview();

	void slot_Reset();
	//void slot_ShowState(State_E sta);
	void slot_SetProgress(float progress);
signals:
	void sig_Close();//按下关闭按钮之后，发出sig_Close关闭信号
	void sig_Run(bool);
	void sig_Pause(bool);
	void sig_Stop();
private:
	Ui::GuiStatusview ui;
};
