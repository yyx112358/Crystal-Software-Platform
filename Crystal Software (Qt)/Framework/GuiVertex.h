#pragma once
#include "global.h"
#include "qgraphicsitem.h"
#include "AlgVertex.h"
#include "GraphSharedClass.h"

class GuiNode;
class GuiConnection;

class GuiVertex :
	public QGraphicsObject, private QEnableSharedFromThis<GuiVertex>
{
	Q_OBJECT
		Q_DISABLE_COPY(GuiVertex)
		GRAPH_ENABLE_SHARED(GuiVertex)
public:
	friend class QSharedPointer<GuiVertex>;

	static QSharedPointer<GuiVertex>Create(QSharedPointer<const AlgVertex>vtx, QSharedPointer<const GuiNode>gnode);
	virtual ~GuiVertex();
	enum { Type = GuiType_Vertex };
	virtual int type()const { return Type; }

	//void AddConnection()
	void AddConnection(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst);
	void RemoveConnection(const AlgVertex*const src, const AlgVertex*const dst);

	virtual QPointF ArrowAttachPosition()const;//箭头连接点位置 
	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	static size_t GetAmount() { return _amount; }
	const QWeakPointer<const GuiNode>guiNode;
	const QWeakPointer<const AlgVertex>algVertex;

signals:
	void sig_UpdateConnection();

	void sig_Destroyed(QWeakPointer<GuiVertex>wp);//删除信号
protected:
	GuiVertex(QSharedPointer<const AlgVertex>vtx, QWeakPointer<const GuiNode>gnode);

	QList<QSharedPointer<GuiConnection>>_connections;
	QList<QWeakPointer<GuiConnection>>_backConnections;

	bool _hoverState = false;

	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;


	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
	static std::atomic_uint64_t _amount;//类实例总数
};

