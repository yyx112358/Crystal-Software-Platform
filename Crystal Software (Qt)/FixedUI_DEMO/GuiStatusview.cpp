#include "stdafx.h"
#include "GuiStatusview.h"

GuiStatusview::GuiStatusview(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.pushButton_close, &QPushButton::clicked, this, &GuiStatusview::sig_Close);
	connect(ui.pushButton_run, &QPushButton::clicked, this, &GuiStatusview::sig_Run);
	connect(ui.pushButton_pause, &QPushButton::clicked, this, &GuiStatusview::sig_Pause);
}

GuiStatusview::~GuiStatusview()
{
}

void GuiStatusview::slot_Reset()
{

}

// void GuiStatusview::slot_ShowState(State_E sta)
// {
// 	ui.label->setText(State_Str[static_cast<int>(sta)]);
// 	if(sta==State_E::stop_end)
// 	{
// 		ui.pushButton_run->setChecked(false);
// 		ui.pushButton_pause->setChecked(false);
// 	}
// }

void GuiStatusview::slot_SetProgress(float progress)
{
	if (progress > 1)
		progress = 1;
	if (progress < 0)
		progress = 0;
	ui.progressBar->setValue(ui.progressBar->maximum()*progress);
}
