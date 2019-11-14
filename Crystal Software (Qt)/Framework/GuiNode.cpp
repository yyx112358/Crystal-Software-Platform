#include "stdafx.h"
#include "GuiNode.h"


GuiNode::GuiNode(AlgNode&parent)
	:algnode(parent.WeakRef())
{
	++_amount;
	auto node = algnode.lock();
	setObjectName(node->objectName());
	connect(node.data(), &AlgNode::objectNameChanged, this, &GuiNode::setObjectName);
	setTransformOriginPoint(boundingRect().center());//������Ϊԭ��
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable);//���ƶ�
	//setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable);
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);//��ѡ��
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
	//�߿�
	auto oldpen = painter->pen();
	if (isSelected() == true)
		painter->setPen(Qt::DashLine);
	painter->drawRect(QRectF(0, 0, 100, 100));
	painter->setPen(oldpen);
	//����
	painter->drawText(boundingRect().width() / 2 - option->fontMetrics.width(objectName())/2
		, option->fontMetrics.height(), objectName());
}
void GuiNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	auto pos= event->screenPos();
	QMenu menu;
	QList<QAction*> insitu, ctrler, alg;
	ctrler.append(menu.addAction("delete"));
	insitu.append(menu.addAction("bring to front"));
	auto result = menu.exec(pos);
	if (result != nullptr)
	{
		if (insitu.contains(result))
		{

		}
		else if (ctrler.contains(result))
			emit sig_SendActionToController(WeakRef(), result->text());
		else if (alg.contains(result))
			emit sig_SendActionToAlg(WeakRef(), result->text());
	}
}

std::atomic_uint64_t GuiNode::_amount;
