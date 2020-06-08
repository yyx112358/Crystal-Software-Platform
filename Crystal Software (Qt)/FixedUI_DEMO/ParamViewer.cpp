#include "stdafx.h"
#include "ParamViewer.h"
#include "GraphError.h"

ParamViewer::ParamViewer(QWidget *parent)
	: QDockWidget(parent)
{
	ui.setupUi(this);
}

ParamViewer::~ParamViewer()
{
}

void ParamViewer::AddParam(ParamView::ROLE role, QString name, QVariant::Type type, QString explaination /*= ""*/, QVariant defaultValue /*= QVariant()*/)
{
	_Role2View(role).AddParam(name, type, explaination, defaultValue);
}

void ParamViewer::RemoveParam(ParamView::ROLE role, QString name)
{
	_Role2View(role).RemoveParam(name);
}

void ParamViewer::SetParam(ParamView::ROLE role, QString name, QVariant value)
{
	_Role2View(role).SetParam(name, value);
}

QVariant ParamViewer::GetParam(ParamView::ROLE role, QString name) const
{
	return _Role2View(role).GetParam(name);
}

ParamView& ParamViewer::_Role2View(ParamView::ROLE role)
{
	switch (role)
	{
	case ParamView::INPUT:
		return *ui.paramView_Input;
	case ParamView::OUTPUT:
		return *ui.paramView_Output;
	case ParamView::PARAMETER:
		return *ui.paramView_Param;
	default:
		GRAPH_ASSERT(role == ParamView::INPUT || role == ParamView::OUTPUT || role == ParamView::PARAMETER);
	}
}

const ParamView& ParamViewer::_Role2View(ParamView::ROLE role) const
{
	switch (role)
	{
	case ParamView::INPUT:
		return *ui.paramView_Input;
	case ParamView::OUTPUT:
		return *ui.paramView_Output;
	case ParamView::PARAMETER:
		return *ui.paramView_Param;
	default:
		GRAPH_ASSERT(role == ParamView::INPUT || role == ParamView::OUTPUT || role == ParamView::PARAMETER);
	}
}
