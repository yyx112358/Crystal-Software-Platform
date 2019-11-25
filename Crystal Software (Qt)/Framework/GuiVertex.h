#pragma once
#include "global.h"
#include "qgraphicsitem.h"
#include "AlgVertex.h"

class GuiNode;

class GuiVertex :
	public QGraphicsObject,public QEnableSharedFromThis<GuiVertex>
{
public:
	friend class QSharedPointer<GuiVertex>;

	static QSharedPointer<GuiVertex>Create(QSharedPointer<const AlgVertex>vtx, QWeakPointer<const GuiNode>gnode);
	virtual ~GuiVertex();
	enum { Type = GuiType_Vertex };
	virtual int type()const { return Type; }

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	static size_t GetAmount() { return _amount; }
	QWeakPointer<GuiVertex>WeakRef() { return _weakRef; }
	QWeakPointer<const GuiVertex>WeakRef()const { return _weakRef; }
	const QWeakPointer<const GuiNode>guiNode;
	const QWeakPointer<const AlgVertex>algVertex;
protected:
	GuiVertex(QSharedPointer<const AlgVertex>vtx, QWeakPointer<const GuiNode>gnode);

private:
	QWeakPointer<GuiVertex>_weakRef;
	static std::atomic_uint64_t _amount;//类实例总数
};

