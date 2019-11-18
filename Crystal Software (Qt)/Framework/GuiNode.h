#pragma once
#include "global.h"
#include "qgraphicsitem.h"
#include <atomic>
//#include "AlgNode.h"
#include "GuiVertex.h"

class AlgNode;
class AlgVertex;
class GuiVertex;

class GuiNode :
	public QGraphicsObject
{
	Q_OBJECT
public:
	struct FactoryInfo
	{
		QString key;//用于唯一标识的key
		QString title;//显示用的标题
		QString description;//描述
		std::function<QSharedPointer<GuiNode>(AlgNode&)>defaultConstructor;//默认构造函数

		FactoryInfo() {}
		FactoryInfo(QString key, std::function<QSharedPointer<GuiNode>(AlgNode&)>defaultConstructor, QString description = QString())
			:key(key), title(key), description(description), defaultConstructor(defaultConstructor)
		{}
	};
	enum { Type = GuiType_Node };
	virtual int type()const { return Type; }

	friend class Interface_Factory;
	friend class QSharedPointer<GuiNode>;

	virtual ~GuiNode();

	virtual void InitApperance(QPointF center);//根据调用时候的状态生成（全量更新）
	virtual QWeakPointer<GuiVertex>AddVertex(QSharedPointer<const AlgVertex>vtx);//添加（增量更新）
	virtual QSharedPointer<QWidget> InitWidget(QWidget*parent) { GRAPH_NOT_IMPLEMENT; }

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
	virtual void update();//
	virtual void refresh(){}

	QWeakPointer<GuiNode>WeakRef()const { return _weakRef; }
	static size_t GetAmount() { return _amount; }
	const QWeakPointer<const AlgNode> algnode;//对应的AlgNode
signals:
	void sig_SendActionToAlg(QString action, bool isChecked);
	void sig_SendActionToController(QWeakPointer<GuiNode>wp, QString action, bool isChecked);
protected:
 	QList<QSharedPointer<GuiVertex>>&_Vertexes(AlgVertex::VertexType type);
 	const QList<QSharedPointer<GuiVertex>>&_Vertexes(AlgVertex::VertexType type)const;
	void _SortVertexesByName(AlgVertex::VertexType type);
	void _ArrangeLocation();

	QList<QSharedPointer<GuiVertex>>_inputVertex;
	QList<QSharedPointer<GuiVertex>>_outputVertex;
	QSharedPointer<QWidget>_panel;

	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
	GuiNode(AlgNode&parent);
	void SetWeakRef(QWeakPointer<GuiNode>wp) { _weakRef = wp; }
	QWeakPointer<GuiNode>_weakRef;//自身的weakRef，用于

	static std::atomic_uint64_t _amount;//类实例总数
};

