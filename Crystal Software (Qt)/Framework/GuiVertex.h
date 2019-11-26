#pragma once
#include "global.h"
#include "qgraphicsitem.h"
#include "AlgVertex.h"
#include "GraphSharedClass.h"

class GuiNode;

class GuiVertex :
	public QGraphicsObject, private QEnableSharedFromThis<GuiVertex>
{
	GRAPH_ENABLE_SHARED(GuiVertex)
public:
	friend class QSharedPointer<GuiVertex>;

	static QSharedPointer<GuiVertex>Create(QSharedPointer<const AlgVertex>vtx, QWeakPointer<const GuiNode>gnode);
	virtual ~GuiVertex();
	enum { Type = GuiType_Vertex };
	virtual int type()const { return Type; }

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	static size_t GetAmount() { return _amount; }
	const QWeakPointer<const GuiNode>guiNode;
	const QWeakPointer<const AlgVertex>algVertex;
protected:
	GuiVertex(QSharedPointer<const AlgVertex>vtx, QWeakPointer<const GuiNode>gnode);

private:
	static std::atomic_uint64_t _amount;//类实例总数
};

