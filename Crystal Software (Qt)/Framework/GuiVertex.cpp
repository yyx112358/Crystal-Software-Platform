#include "stdafx.h"
#include "GuiVertex.h"
#include "GuiNode.h"
#include "GuiConnection.h"

GuiVertex::GuiVertex(QSharedPointer<const AlgVertex>vtx, QWeakPointer<const GuiNode>gnode)
	:QGraphicsObject(const_cast<GuiNode*>(gnode.data())), algVertex(vtx), guiNode(gnode)
{
	GRAPH_ASSERT(vtx.isNull() == false && gnode.isNull() == false);
	++_amount;
	setObjectName(vtx->objectName());
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsScenePositionChanges);
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
	setAcceptHoverEvents(true);
	if (gnode.isNull() == false)
		setZValue(gnode.lock()->zValue() + 0.1);
}

QVariant GuiVertex::itemChange(GraphicsItemChange change, const QVariant &value)
{	
	if (change == QGraphicsItem::ItemScenePositionHasChanged) 
	{
		emit sig_UpdateConnection();
		for (auto c : _connections)
			c->updatePosition();
	}
	return value;
}

void GuiVertex::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	_hoverState = true;
	QGraphicsItem::hoverEnterEvent(event);
}

void GuiVertex::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	_hoverState = false;
	QGraphicsItem::hoverLeaveEvent(event);
}

QSharedPointer<GuiVertex> GuiVertex::Create(QSharedPointer<const AlgVertex>vtx, QSharedPointer<const GuiNode>gnode)
{
	return QSharedPointer<GuiVertex>::create(vtx, gnode);
}

GuiVertex::~GuiVertex()
{
	qDebug() << __FUNCTION__;
	emit sig_Destroyed(sharedFromThis());
	_connections.clear();
	--_amount;
}

void GuiVertex::AddConnection(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst)
{
	if (src.isNull() == false && src->GetGui() == sharedFromThis())
	{
		auto con = GuiConnection::Create(src, dst);
		//connect(dst->destroyed)
		if (dst.isNull() == false && dst->GetGui().isNull() == false)
		{
			auto dstV = dst->GetGui().lock();
			dstV->_backConnections.append(con);
			connect(dstV.data(), &GuiVertex::sig_UpdateConnection, con.data(), &GuiConnection::updatePosition);
		}
		scene()->addItem(con.data());
		_connections.append(con);
	}
// 	else if (dst.isNull() == false && dst->GetGui() == WeakRef())
// 	{
// 		//connect(this,)
// 	}
// 	else
// 		return;
}

void GuiVertex::RemoveConnection(const AlgVertex*const src, const AlgVertex*const dst)
{
	if (src == algVertex.data())
	{
		QSharedPointer<GuiConnection>pcon;
		for(auto it=_connections.begin();it!=_connections.end();++it)
		{
			if ((*it)->dstAlgVertex.data() == dst)
			{
				pcon = *it;
				_connections.erase(it);
				break;
			}
		}
		
	}
	else if (dst == algVertex.data())
	{
	}
	else
		return;
}

QPointF GuiVertex::ArrowAttachPosition() const
{
	auto bbox = boundingRect();
	switch (algVertex.lock()->type)
	{
	case AlgVertex::VertexType::INPUT:
		return QPointF(-0, bbox.height() / 2);
		break;
	case AlgVertex::VertexType::OUTPUT:
		return QPointF(bbox.width() + 0, bbox.height() / 2);
		break;
	default:
		GRAPH_NOT_IMPLEMENT;
	}
}

QRectF GuiVertex::boundingRect() const
{
	QFontMetrics fm(scene() != nullptr ? (scene()->font()) : (QApplication::font()));
	return QRectF(0, 0, fm.width(objectName()), fm.height());
}

void GuiVertex::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	painter->drawText(0, option->fontMetrics.height(), objectName());
	auto bbox = boundingRect();
	if (isSelected() == true) 
	{
		painter->setBrush(Qt::Dense5Pattern);
	}
	if (_hoverState == true) 
	{
		painter->setPen(QPen(QColor(160, 160, 160)));
		painter->drawRect(bbox);
	}
}

QSharedPointer<GuiVertex> GuiVertex::Clone()const
{
	GRAPH_NOT_IMPLEMENT;
}

std::atomic_uint64_t GuiVertex::_amount;
