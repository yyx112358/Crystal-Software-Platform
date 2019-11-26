#pragma once
#include "global.h"
#include "qgraphicsitem.h"
#include <atomic>
#include "GraphSharedClass.h"
#include "GuiVertex.h"

class AlgNode;
class AlgVertex;
class GuiVertex;

class GuiNode :
	public QGraphicsObject, protected QEnableSharedFromThis<GuiNode>
{
	Q_OBJECT
	Q_DISABLE_COPY(GuiNode)
	GRAPH_ENABLE_SHARED(GuiNode)
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
	virtual void update();//
	virtual void refresh(){}

	//QWeakPointer<GuiNode>WeakRef()const { return _weakRef; }
	static size_t GetAmount() { return _amount; }
	const QWeakPointer<const AlgNode> algnode;//对应的AlgNode
signals:
	void sig_SendActionToAlg(QString action, bool isChecked);
	void sig_SendActionToController(QSharedPointer<GuiNode>node, QString action, bool isChecked);
protected:
	GuiNode(QSharedPointer<AlgNode>parent);

 	QList<QSharedPointer<GuiVertex>>&_Vertexes(AlgVertex::VertexType type);
 	const QList<QSharedPointer<GuiVertex>>&_Vertexes(AlgVertex::VertexType type)const;
	void _SortVertexesByName(AlgVertex::VertexType type);
	void _ArrangeLocation();

	QList<QSharedPointer<GuiVertex>>_inputVertex;
	QList<QSharedPointer<GuiVertex>>_outputVertex;
	QSharedPointer<QWidget>_panel;

	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
	static std::atomic_uint64_t _amount;//类实例总数
};

