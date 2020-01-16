#include "stdafx.h"
#include "GuiNode_Output.h"


QSharedPointer<QWidget> GuiNode_Output::InitWidget(QWidget*parent)
{
//	QSharedPointer<QWidget>box(this);	
	_panel = QSharedPointer<QLabel>::create(parent);
	_panel.objectCast<QLabel>()->setFrameStyle(QFrame::Box | QFrame::Raised);
	return _panel;
}

QVariant GuiNode_Output::GetData()
{
	GRAPH_ASSERT(_panel.isNull() == false);
	return _panel.objectCast<QLabel>()->text();
}

void GuiNode_Output::SetData(QVariant data)
{
	GRAPH_ASSERT(_panel.isNull() == false && data.canConvert<QString>());
	_panel.objectCast<QLabel>()->setText(data.toString());
}