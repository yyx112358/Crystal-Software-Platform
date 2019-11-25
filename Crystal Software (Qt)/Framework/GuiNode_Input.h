#pragma once
#include "GuiNode.h"
class GuiNode_Input :
	public GuiNode
{
	Q_OBJECT
public:
	GuiNode_Input(QSharedPointer<AlgNode>parent):GuiNode(parent){}
	virtual ~GuiNode_Input(){}

	virtual QSharedPointer<QWidget> InitWidget(QWidget*parent) override;


	virtual QVariant GetData() override;
	virtual void SetData(QVariant data) override;

};

