#pragma once
#include "qgraphicsitem.h"

class AlgVertex;
class GuiVertex;

//GUI���浱�е�������
class GuiConnection :
	public QGraphicsObject, public QEnableSharedFromThis<GuiConnection>
{
	GRAPH_SHARED_BASE_QOBJECT(GuiConnection)
public:
	static QSharedPointer<GuiConnection>Create(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst);//��������ע��dst����Ϊ�գ�
	virtual ~GuiConnection();

	enum {	Type = GuiType_Connection };
	virtual int type()const { return Type; }

	void updatePosition();

	void SetLine(QLineF l);//�趨scene����
	QLineF Line()const;//����scene����
	virtual QPainterPath shape() const override;
	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	const QWeakPointer<const AlgVertex>srcAlgVertex, dstAlgVertex;
	const QWeakPointer<const GuiVertex>srcGuiVertex, dstGuiVertex;

protected:
	explicit GuiConnection(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst);
	QLineF _line;//��Ӧ���߶Σ�����ʼGuiVertex������ԭ�㣨һ�������Ͻǣ�Ϊԭ�㣩
	QPolygonF _arrowHead;//��ͷ
	//QGraphicsLineItem _lineItem;
private:
	static double _arrowHeadSize;//��ͷ��С
};

