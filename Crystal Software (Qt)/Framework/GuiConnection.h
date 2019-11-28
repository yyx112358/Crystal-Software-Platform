#pragma once
#include "qgraphicsitem.h"
#include "GraphSharedClass.h"

class AlgVertex;
class GuiVertex;

//GUI界面当中的连接线
class GuiConnection :
	public QGraphicsObject, private QEnableSharedFromThis<GuiConnection>
{
	GRAPH_ENABLE_SHARED(GuiConnection)
public:
	static QSharedPointer<GuiConnection>Create(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst);//创建对象（注意dst不能为空）
	virtual ~GuiConnection();

	enum {	Type = GuiType_Connection };
	virtual int type()const { return Type; }

	void updatePosition();

	void SetLine(QLineF l);//设定scene坐标
	QLineF Line()const;//返回scene坐标
	virtual QPainterPath shape() const override;
	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	static size_t GetAmount() { return _amount; }
	const QWeakPointer<const AlgVertex>srcAlgVertex, dstAlgVertex;
	const QWeakPointer<const GuiVertex>srcGuiVertex, dstGuiVertex;

protected:
	explicit GuiConnection(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst);
	QLineF _line;//对应的线段（以起始GuiVertex的坐标原点（一般是左上角）为原点）
	QPolygonF _arrowHead;//箭头
	//QGraphicsLineItem _lineItem;
private:
	static std::atomic_uint64_t _amount;//类实例总数
	static double _arrowHeadSize;//箭头大小
};

