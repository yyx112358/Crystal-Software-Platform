#pragma once
#include "global.h"
#include "qgraphicsitem.h"
#include <atomic>
#include "GuiVertex.h"

class AlgNode;
class AlgVertex;
class GuiVertex;

class GuiNode :
	public QGraphicsObject, public QEnableSharedFromThis<GuiNode>
{
	Q_OBJECT
	GRAPH_SHARED_BASE_QOBJECT(GuiNode)
public:
	struct FactoryInfo
	{
		QString key;//用于唯一标识的key
		QString title;//显示用的标题
		QString description;//描述
		std::function<QSharedPointer<GuiNode>(QSharedPointer<AlgNode>)>defaultConstructor;//默认构造函数

		FactoryInfo() {}
		FactoryInfo(QString key, std::function<QSharedPointer<GuiNode>(QSharedPointer<AlgNode>)>defaultConstructor, QString description = QString())
			:key(key), title(key), description(description), defaultConstructor(defaultConstructor)
		{}
	};
	enum { Type = GuiType_Node };
	virtual int type()const { return Type; }

	bool RemoveFromParent();
	virtual ~GuiNode();

	virtual void InitApperance(QPointF center);//根据调用时候的状态生成（全量更新）
	virtual QWeakPointer<GuiVertex>AddVertex(QSharedPointer<AlgVertex>vtx);//添加（增量更新）
	virtual void RemoveVertex(QSharedPointer<const AlgVertex>vtx);
	virtual QSharedPointer<QWidget> InitWidget(QWidget*parent) { return nullptr; }

	virtual QVariant GetData() { GRAPH_NOT_IMPLEMENT; }
	virtual void SetData(QVariant data) { GRAPH_NOT_IMPLEMENT; }

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
	//virtual void update();//
	virtual void refresh(){}

	//QWeakPointer<GuiNode>WeakRef()const { return _weakRef; }
	const QWeakPointer<const AlgNode> algnode;//对应的AlgNode
signals:
	void sig_SendActionToAlg(QString action, bool isChecked);
	void sig_SendActionToController(QSharedPointer<GuiNode>node, QString action, bool isChecked);
protected:
	GuiNode(QSharedPointer<AlgNode>parent);

 	QList<QSharedPointer<GuiVertex>>&_Vertexes(AlgVertex::VertexType type);
 	const QList<QSharedPointer<GuiVertex>>&_Vertexes(AlgVertex::VertexType type)const;
	virtual void _SortVertexesByName(AlgVertex::VertexType type);
	virtual void _ArrangeLocation();

	QList<QSharedPointer<GuiVertex>>_inputVertex;
	QList<QSharedPointer<GuiVertex>>_outputVertex;
	QSharedPointer<QWidget>_panel;

	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
};

