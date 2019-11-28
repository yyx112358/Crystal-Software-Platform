#include "stdafx.h"
#include "GuiConnection.h"
#include "GuiVertex.h"
#include "AlgVertex.h"

GuiConnection::GuiConnection(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst)
	:QGraphicsObject(/*src->GetGui().data()*/nullptr),srcAlgVertex(src),dstAlgVertex(dst),	
	srcGuiVertex(src->GetGui()),
	dstGuiVertex(dst.isNull() == false && dst->GetGui().isNull() == false ? dst->GetGui() : QWeakPointer<GuiVertex>())
{
	GRAPH_ASSERT(src.isNull() == false && src->GetGui().isNull() == false);
	_amount++;
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
	updatePosition();
}


QSharedPointer<GuiConnection> GuiConnection::Create(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst)
{
	auto con = QSharedPointer<GuiConnection>::create(src, dst);
	con->SetSelfPointer();
	return con;
}

GuiConnection::~GuiConnection()
{
	qDebug()<<((dstGuiVertex.isNull()==false)?(dstGuiVertex.lock()->objectName()):"")<<"-"
		<< ((dstGuiVertex.isNull()==false) ? (dstGuiVertex.lock()->objectName()) : "") << __FUNCTION__;
	--_amount;
}

void GuiConnection::updatePosition()
{
	QPointF p1 = _line.p1(), p2 = _line.p2();
	qreal z = zValue();
	if (srcGuiVertex.isNull() == false) 
	{
		auto srcV = srcGuiVertex.lock();
		p1 = srcV->mapToScene( srcV->ArrowAttachPosition());
		z = std::max(srcV->zValue() + 0.2, z);
	}
	if(dstGuiVertex.isNull() == false)
	{
		auto dstV = dstGuiVertex.lock();
		p2 = dstV->mapToScene(dstV->ArrowAttachPosition());//因为需要以srcGuiVertex起始坐标为原点，所以需要一次变换
		z = std::max(dstV->zValue() + 0.2, z);
	}
	prepareGeometryChange();//【不加这个会残影】
	_line.setP1(p1);
	_line.setP2(p2);
	if (z != zValue())
		setZValue(z);
}

void GuiConnection::SetLine(QLineF l)
{
	_line.setP1(mapFromScene(l.p1()));
	_line.setP2(mapFromScene(l.p2()));
	update();
}

QLineF GuiConnection::Line() const
{
	return QLineF(mapToScene(_line.p1()),mapToScene(_line.p2()));
}

QRectF GuiConnection::boundingRect() const
{
	qreal extra = (3/*pen().width()*/ + _arrowHeadSize) / 2.0;

	return QRectF(_line.p1(), QSizeF(_line.x2() - _line.x1(), _line.y2() - _line.y1()))
		.normalized().adjusted(-extra, -extra, extra, extra);
// 	return QRectF(std::min(_line.x1(), _line.x2()) - 1, std::min(_line.y1(), _line.y2() - 1),
// 		std::abs(_line.dx()) + 2, std::abs(_line.dy()) + 2);
}

void GuiConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	//画箭头
	double angle = std::atan2(-_line.dy(), _line.dx());	
	qreal arrowSize = _arrowHeadSize;
	QPointF arrowP1 = _line.p2() - QPointF(sin(angle + M_PI / 3) * arrowSize,
		cos(angle + M_PI / 3) * arrowSize);
	QPointF arrowP2 = _line.p2() - QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
		cos(angle + M_PI - M_PI / 3) * arrowSize);
	_arrowHead.clear();
	_arrowHead << _line.p2() << arrowP1 << arrowP2;
	painter->setBrush(Qt::BrushStyle::SolidPattern);
	painter->drawPolygon(_arrowHead);
	//画线
	if (isSelected())
		painter->setPen(Qt::DashLine);
	painter->drawLine(_line);
}
QPainterPath GuiConnection::shape() const
{
 	QPainterPath path;
 	path.moveTo(_line.x1() - 0.5, _line.y1() - 0.5);
 	path.lineTo(_line.x2() + 0.5, _line.y2() + 0.5);
 	path.lineTo(_line.x2() - 0.5, _line.y1() - 0.5);
 	path.lineTo(_line.x1() + 0.5, _line.y1() + 0.5);
 	path.lineTo(_line.x1() - 0.5, _line.y1() - 0.5);
 	path.moveTo(_line.p1());
 	path.lineTo(_line.p2());
	path.addPolygon(_arrowHead);
 	//path.addRect(boundingRect());
 	return path;
// 	auto s = QGraphicsLineItem(_line).shape();
// 	s.addPolygon(_arrowHead);
// 	return s;
}

std::atomic_uint64_t GuiConnection::_amount;

double GuiConnection::_arrowHeadSize = 10;
