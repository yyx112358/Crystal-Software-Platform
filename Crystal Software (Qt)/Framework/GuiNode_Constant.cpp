#include "stdafx.h"
#include "GuiNode_Constant.h"

#include "AlgNode_Constant.h"
#include "CustomTypes.h"

GuiNode_Constant::GuiNode_Constant(QSharedPointer<AlgNode>parent) 
	:GuiNode(parent)
{

}

QRectF GuiNode_Constant::boundingRect() const
{
	auto pn = algnode.lock().objectCast<const AlgNode_Constant>();
	if (pn != nullptr)
	{
		QFontMetrics fm(scene() != nullptr ? (scene()->font()) : (QApplication::font()));
		QString str = QVariant2Description(pn->GetData(), 10);
		if (str != _outputVertex.front()->objectName())
			_outputVertex.front()->setObjectName(str);
		return _outputVertex.front()->boundingRect();
	}
	else
		return GuiNode::boundingRect();
}

void GuiNode_Constant::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	painter->drawRect(boundingRect());

}

void GuiNode_Constant::_ArrangeLocation()
{
	GRAPH_ASSERT(_outputVertex.size() == 1);
	_outputVertex.front()->setPos(0, 0);
	_outputVertex.front()->setZValue(zValue() - 0.1);
}
