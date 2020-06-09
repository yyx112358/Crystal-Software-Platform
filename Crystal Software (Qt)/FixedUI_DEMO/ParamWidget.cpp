#include "stdafx.h"
#include "ParamWidget.h"
#include "GraphError.h"

ParamWidget::ParamWidget(ParamView::ROLE role, QWidget *parent/* = Q_NULLPTR*/)
	: QDockWidget(parent)
{
	ui.setupUi(this);
	const_cast<ParamView::ROLE>(ui.paramView->_role) = role;
	const_cast<ParamView*>(view) = ui.paramView;
}

ParamWidget::~ParamWidget()
{
}

void ParamWidget::SetName(QString name)
{
	setWindowTitle(name);
	setObjectName(name);
	view->setObjectName(name);
}

