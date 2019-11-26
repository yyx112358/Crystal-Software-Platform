#pragma once

#include "GuiNode.h"

class GuiNode_Output 
	: public GuiNode
{
	Q_OBJECT
		friend class QSharedPointer<GuiNode_Output>;
public:
	GuiNode_Output(QSharedPointer<AlgNode>parent):GuiNode(parent){}
	virtual ~GuiNode_Output(){}

	virtual QSharedPointer<QWidget> InitWidget(QWidget*parent) override;


	virtual QVariant GetData() override;
	virtual void SetData(QVariant data) override;
};
