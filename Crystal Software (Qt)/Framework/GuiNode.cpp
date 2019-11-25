#include "stdafx.h"
#include "AlgNode.h"
#include "GuiNode.h"
#include "GuiVertex.h"
#include <algorithm>

GuiNode::GuiNode(QSharedPointer<AlgNode>parent)
	:algnode(parent)
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


bool GuiNode::RemoveFromParent()
{
	GRAPH_NOT_IMPLEMENT;
}

GuiNode::~GuiNode()
{
	qDebug() << objectName() << __FUNCTION__;
	_panel.clear();
	--_amount;
}

void GuiNode::InitApperance(QPointF center)
{
	if (_weakRef.isNull() == true && sharedFromThis().isNull() == false)
		_weakRef = sharedFromThis(); 
	_inputVertex.clear();
	_outputVertex.clear();

	setPos(center);
	auto anode = algnode.lock();
	for (auto avtx : anode->GetVertexes(AlgVertex::VertexType::INPUT))
		AddVertex(avtx.constCast<AlgVertex>());
	for (auto avtx : anode->GetVertexes(AlgVertex::VertexType::OUTPUT))
		AddVertex(avtx.constCast<AlgVertex>());
	_ArrangeLocation();
}

QWeakPointer<GuiVertex> GuiNode::AddVertex(QSharedPointer<AlgVertex>vtx)
{
	QSharedPointer<GuiVertex>gvtx = GuiVertex::Create(vtx, sharedFromThis());
	vtx->AttachGui(gvtx);
	_Vertexes(vtx->type).append(gvtx);
	return gvtx;
}

void GuiNode::RemoveVertex(QSharedPointer<const AlgVertex>vtx)
{
	_Vertexes(vtx->type).removeOne(vtx->GetGui());
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

void GuiNode::update()
{

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
			emit sig_SendActionToController(sharedFromThis(), result->text(),result->isChecked());
		else if (alg.contains(result))
			emit sig_SendActionToAlg(result->text(),result->isChecked());
	}
}

QList<QSharedPointer<GuiVertex>>& GuiNode::_Vertexes(AlgVertex::VertexType type)
{
	switch (type)
	{
	case AlgVertex::VertexType::INPUT:return _inputVertex;
	case AlgVertex::VertexType::OUTPUT:return _outputVertex;
	default:GRAPH_NOT_IMPLEMENT; break;
	}
}
const QList<QSharedPointer<GuiVertex>>& GuiNode::_Vertexes(AlgVertex::VertexType type) const
{
	switch (type)
	{
	case AlgVertex::VertexType::INPUT:return _inputVertex;
	case AlgVertex::VertexType::OUTPUT:return _outputVertex;
	default:GRAPH_NOT_IMPLEMENT; break;
	}
}




void GuiNode::_SortVertexesByName(AlgVertex::VertexType type)
{
	auto &vtxList = _Vertexes(type);
	std::sort(vtxList.begin(), vtxList.end(), [](const QSharedPointer<GuiVertex>&a, const QSharedPointer<GuiVertex>&b)
	{
		return a->objectName() < b->objectName();
	});
}

void GuiNode::_ArrangeLocation()
{
	QFontMetrics fm((scene() != nullptr) ? (scene()->font()) : QApplication::font());
	for (auto x=0, y = fm.height(), i = 0; i < _inputVertex.size();y+=_inputVertex[i]->boundingRect().height(), i++)
		_inputVertex[i]->setPos(x, y);
	for (auto y = fm.height(), i = 0; i < _outputVertex.size(); y += _outputVertex[i]->boundingRect().height(), i++)
		_outputVertex[i]->setPos(boundingRect().width() - _outputVertex[i]->boundingRect().width(), y);
}

std::atomic_uint64_t GuiNode::_amount;
