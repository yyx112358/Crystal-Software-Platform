#include "stdafx.h"
#include "GuiNode_Constant.h"

#include "AlgNode_Constant.h"
#include "CustomTypes.h"
#include <algorithm>

GuiNode_Constant::GuiNode_Constant(QSharedPointer<AlgNode>parent) 
	:GuiNode(parent)
{

}

QRectF GuiNode_Constant::boundingRect() const
{
	auto pn = algnode.lock().objectCast<const AlgNode_Constant>();
	if (pn != nullptr/* && _outputVertex.empty() == false*/)
	{
		QFontMetrics fm(scene() != nullptr ? (scene()->font()) : (QApplication::font()));
		QString str = QVariant2Description(pn->GetData(), 10);
		return QRectF(0, 0, std::max(fm.width(str) + 2, fm.width("    ") + 2), fm.height() + 2);
// 		if (str != _outputVertex.front()->objectName())
// 			_outputVertex.front()->setObjectName(str);
//		return _outputVertex.front()->boundingRect();
	}
	else
		return GuiNode::boundingRect();
}

void GuiNode_Constant::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	auto pn = algnode.lock().objectCast<const AlgNode_Constant>();
	if (pn.isNull() == true)
		return;
	QPen pen;
	if (isSelected() == true)
		pen.setStyle(Qt::DashLine);
	if (algnode.lock()->IsRunning() == true)
		pen.setColor(QColor(255, 0, 0));
	else
		pen.setColor(QColor(0, 0, 0));
	painter->setPen(pen);
	QRectF box = boundingRect();
	painter->drawRect(box);
	painter->drawText(1, box.height() - 2, QVariant2Description(pn->GetData(), 10));
}

void GuiNode_Constant::_ArrangeLocation()
{
	GRAPH_ASSERT(_outputVertex.size() == 1);
	_outputVertex.front()->setPos(boundingRect().width() - 12, 0);
	_outputVertex.front()->setZValue(zValue() - 0.1);
}
