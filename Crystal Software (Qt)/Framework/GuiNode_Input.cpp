#include "stdafx.h"
#include "GuiNode_Input.h"


QSharedPointer<QWidget> GuiNode_Input::InitWidget(QWidget*parent)
{
	_panel=QSharedPointer<QPlainTextEdit>::create(parent);
	return _panel;
}

QVariant GuiNode_Input::GetData()
{
	GRAPH_ASSERT(_panel.isNull() == false);
	return _panel.objectCast<QPlainTextEdit>()->toPlainText();
}

void GuiNode_Input::SetData(QVariant data)
{
	GRAPH_ASSERT(_panel.isNull() == false && data.canConvert<QString>());
	_panel.objectCast<QPlainTextEdit>()->setPlainText(data.toString());
}
