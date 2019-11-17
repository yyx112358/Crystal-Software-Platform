#include "stdafx.h"
#include "GuiNode.h"


GuiNode::GuiNode(AlgNode&parent)
	:algnode(parent.WeakRef())
{
	++_amount;
	auto node = algnode.lock();
	setObjectName(node->objectName());
	connect(node.data(), &AlgNode::objectNameChanged, this, &GuiNode::setObjectName);
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

void GuiNode::InitApperance(QPointF center)
{

}

QRectF GuiNode::boundingRect() const
{
	return QRectF(0, 0, 100, 100);
}

void GuiNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	//边框
	//auto oldpen = painter->pen();
	if (isSelected() == true)
		painter->setPen(Qt::DashLine);
	//painter->setBackgroundMode(Qt::OpaqueMode);
	QPainterPath path;
	path.addRoundRect(boundingRect(), 25);
	painter->fillPath(path, Qt::white);
	painter->drawRoundRect(boundingRect());
	//painter->setPen(oldpen);	
	//标题
	painter->drawText(boundingRect().width() / 2 - option->fontMetrics.width(objectName())/2
		, option->fontMetrics.height(), objectName());
}
void GuiNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	//static QSharedDataPointer<QString>defaultMenu;//TODO:之后可以用这种隐式共享指针来实现默认菜单。保证默认情况下共用一个默认表，而发生复制时候则自动复制
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
