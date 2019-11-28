#pragma once
#include "qgraphicsitem.h"
#include "GraphSharedClass.h"

class AlgVertex;
class GuiVertex;

//GUI���浱�е�������
class GuiConnection :
	public QGraphicsObject, private QEnableSharedFromThis<GuiConnection>
{
	GRAPH_ENABLE_SHARED(GuiConnection)
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

	static size_t GetAmount() { return _amount; }
	const QWeakPointer<const AlgVertex>srcAlgVertex, dstAlgVertex;
	const QWeakPointer<const GuiVertex>srcGuiVertex, dstGuiVertex;

protected:
	explicit GuiConnection(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst);
	QLineF _line;//��Ӧ���߶Σ�����ʼGuiVertex������ԭ�㣨һ�������Ͻǣ�Ϊԭ�㣩
	QPolygonF _arrowHead;//��ͷ
	//QGraphicsLineItem _lineItem;
private:
	static std::atomic_uint64_t _amount;//��ʵ������
	static double _arrowHeadSize;//��ͷ��С
};

