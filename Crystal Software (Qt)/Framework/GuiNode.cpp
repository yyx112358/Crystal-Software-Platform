#include "stdafx.h"
#include "AlgNode.h"
#include "GuiNode.h"
#include "GuiVertex.h"
#include <algorithm>

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
	_panel.clear();
	--_amount;
}

void GuiNode::InitApperance(QPointF center)
{
	_inputVertex.clear();
	_outputVertex.clear();

	setPos(center);
	auto anode = algnode.lock();
	for (auto avtx : anode->GetVertexes(AlgVertex::VertexType::INPUT))
		AddVertex(avtx);
	for (auto avtx : anode->GetVertexes(AlgVertex::VertexType::OUTPUT))
		AddVertex(avtx);
	_ArrangeLocation();
}

QWeakPointer<GuiVertex> GuiNode::AddVertex(QSharedPointer<const AlgVertex>vtx)
{
	QSharedPointer<GuiVertex>gvtx = QSharedPointer<GuiVertex>::create(vtx, this);
	_Vertexes(vtx->type).append(gvtx);
	return gvtx;
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

void GuiNode::update()
{

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
