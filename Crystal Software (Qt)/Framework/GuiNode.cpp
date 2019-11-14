#include "stdafx.h"
#include "GuiNode.h"


GuiNode::GuiNode(AlgNode&parent)
	:algnode(parent)
{
	++_amount;
	setObjectName(algnode.objectName());
	connect(&algnode, &AlgNode::objectNameChanged, this, &GuiNode::setObjectName);
	setTransformOriginPoint(boundingRect().center());//以中心为原点
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable);//可移动
	//setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable);
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);//可选中
}


GuiNode::~GuiNode()
{
	qDebug() << objectName() << __FUNCTION__;
	--_amount;
}

QRectF GuiNode::boundingRect() const
{
	return QRectF(0, 0, 100, 100);
}

void GuiNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	QFontMetrics fm(painter->font());
	painter->drawText(boundingRect().width() / 2 - fm.width(algnode.objectName())/2, fm.height(), algnode.objectName());
	painter->drawRect(QRectF(0, 0, 100, 100));
}

std::atomic_uint64_t GuiNode::_amount;
