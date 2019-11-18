#include "stdafx.h"
#include "GuiVertex.h"


GuiVertex::GuiVertex(QWeakPointer<const AlgVertex>vtx, QGraphicsItem*parent)
	:QGraphicsObject(parent),algVertex(vtx)
{
	++_amount;
	setObjectName(vtx.lock()->objectName());
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
	painter->drawRect(boundingRect());
	painter->drawText(0, option->fontMetrics.height(), objectName());
}

std::atomic_uint64_t GuiVertex::_amount;
