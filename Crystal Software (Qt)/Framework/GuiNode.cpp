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

void GuiNode::InitApperance(QPointF center)
{

}

QRectF GuiNode::boundingRect() const
{
	return QRectF(0, 0, 100, 100);
}

void GuiNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	//�߿�
	//auto oldpen = painter->pen();
	if (isSelected() == true)
		painter->setPen(Qt::DashLine);
	//painter->setBackgroundMode(Qt::OpaqueMode);
	QPainterPath path;
	path.addRoundRect(boundingRect(), 25);
	painter->fillPath(path, Qt::white);
	painter->drawRoundRect(boundingRect());
	//painter->setPen(oldpen);	
	//����
	painter->drawText(boundingRect().width() / 2 - option->fontMetrics.width(objectName())/2
		, option->fontMetrics.height(), objectName());
}
void GuiNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	//static QSharedDataPointer<QString>defaultMenu;//TODO:֮�������������ʽ����ָ����ʵ��Ĭ�ϲ˵�����֤Ĭ������¹���һ��Ĭ�ϱ�����������ʱ�����Զ�����
	//QSharedDataPointer<QString>Menu=defaultMenu;
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
			if (result->text() == "bring to front") 
			{
				float zvalue = zValue();
				for (auto item : collidingItems())
					if (item->zValue() >= zvalue && item->type() == GuiNode::Type)
						zvalue = item->zValue() + 0.1;
				setZValue(zvalue);
			}
		}
		else if (ctrler.contains(result))
			emit sig_SendActionToController(WeakRef(), result->text(),result->isChecked());
		else if (alg.contains(result))
			emit sig_SendActionToAlg(result->text(),result->isChecked());
	}
}

std::atomic_uint64_t GuiNode::_amount;
