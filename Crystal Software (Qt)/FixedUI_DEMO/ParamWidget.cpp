#include "stdafx.h"
#include "ParamWidget.h"
#include "GraphError.h"

ParamWidget::ParamWidget(ParamView::ROLE role, QWidget *parent/* = Q_NULLPTR*/)
	: QDockWidget(parent),role(role)
{
	ui.setupUi(this);
	const_cast<ParamView::ROLE>(ui.paramView->_role) = role;
	connect(ui.paramView, &ParamView::sig_ActionTriggered, this, &ParamWidget::sig_ActionTriggered);
}

ParamWidget::~ParamWidget()
{
}

void ParamWidget::AddParam(QString name, QVariant::Type type, QString explaination /*= ""*/, QVariant defaultValue /*= QVariant()*/)
{
	ui.paramView->AddParam(name, type, explaination, defaultValue);
}

void ParamWidget::RemoveParam( QString name)
{
	ui.paramView->RemoveParam(name);
}

void ParamWidget::SetParam(QString name, QVariant value)
{
	ui.paramView->SetParam(name, value);
}

QVariant ParamWidget::GetParam(QString name) const
{
	return ui.paramView->GetParam(name);
}

