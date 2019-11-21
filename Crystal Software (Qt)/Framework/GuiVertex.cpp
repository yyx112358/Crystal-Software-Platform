#include "stdafx.h"
#include "GuiVertex.h"
#include "GuiNode.h"

GuiVertex::GuiVertex(QWeakPointer<const AlgVertex>vtx, QWeakPointer<const GuiNode>gnode)
	:QGraphicsObject(const_cast<GuiNode*>(gnode.data())), algVertex(vtx), guiNode(gnode)
{
	++_amount;
	setObjectName(vtx.lock()->objectName());
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsScenePositionChanges);
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
}

GuiVertex::~GuiVertex()
{
	--_amount;
}

QRectF GuiVertex::boundingRect() const
{
	QFontMetrics fm(scene() != nullptr ? (scene()->font()) : (QApplication::font()));
	return QRectF(0, 0, fm.width(objectName()), fm.height());
}

void GuiVertex::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	if (isSelected() == true)
		painter->setPen(Qt::DashLine);
	painter->drawRect(boundingRect());
	painter->drawText(0, option->fontMetrics.height(), objectName());
}

std::atomic_uint64_t GuiVertex::_amount;
