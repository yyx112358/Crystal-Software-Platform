#pragma once

#include "GuiNode.h"

class GuiNode_Constant 
	: public GuiNode
{
	Q_OBJECT
		friend class QSharedPointer<GuiNode_Constant>;
public:
	GuiNode_Constant(QSharedPointer<AlgNode>parent);
	virtual ~GuiNode_Constant() {}

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
protected:
	virtual void _ArrangeLocation();
};
