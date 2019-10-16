/*!
 * \file TestAnything.h
 * \date 2019/10/08 11:10
 *
 * \author Yyx112358
 * Contact: user@company.com
 *
 * \brief 
 *
 * TODO: long description
 *
 * \note
*/
#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TestAnything.h"
#include <QtConcurrent>
#include <QVariant>
#include <QDebug>
#include <QTime>
#include <QGraphicsItem>
#include <atomic>

class AlgGraphVertex;
class AlgGraphVertex_Input;
class AlgGraphVertex_Input;
class AlgGraphNode;

class GuiGraphItemVertex;
class GuiGraphNode;

class GraphScene;
class GraphController;
class GraphError;
class GraphWarning;

class TestAnything;


#pragma region AlgGraphVertex
//Alg的Vertex，主要负责数据存储、传递、激活的工作
class AlgGraphVertex
	:public QObject
{
	Q_OBJECT
public:
	AlgGraphVertex(AlgGraphNode&parent,QString name)
		:QObject(reinterpret_cast<QObject*>(&parent)), node(parent) {setObjectName(name);}
	virtual ~AlgGraphVertex() {}

	//《激活函数》如果使能（isEnable==true）首先调用所有的assertFunction做校验，通过后【调用_Activate()函数】
	void Activate(QVariant var, bool isActivate = true)
	{
		if (isEnabled == true)
		{
			emit sig_ActivateBegin();
			for (auto f : assertFunctions)
				if (f(var) == false)
					throw "AssertFail";//TODO:1.改成专用的GraphError；2.加入默认的类型确认部分

			_Activate(var, isActivate);
			emit sig_ActivateEnd();
		}
	}
	//连接两个节点，方向this=>dstVertex，主要是修改connectedVertexes并连接this=>dstVertex的激活信号
	void Connect(AlgGraphVertex*dstVertex)
	{
		assert(dstVertex != nullptr);
		connectedVertexes.append(dstVertex);
		dstVertex->connectedVertexes.append(this);
		emit sig_ConnectionAdded(this, dstVertex);
	}
	virtual void Reset()
	{
		data.clear();
		isActivated = false;
		isEnabled = true;
	}
	virtual void Release()
	{
		Reset();
		defaultData.clear();
// 		for (auto v : connectedVertexes)//TODO:控制器中需要清除信号槽
// 			disconnect(v);
		connectedVertexes.clear();		
	}

	virtual QStringList Write()const {throw "Not Implement"; }//TODO:持久化，保存节点信息和结构（*是否保存数据？）
	virtual void Read(QStringList){ throw "Not Implement";}//TODO:从持久化信息中恢复
	virtual QString GetGuiAdvice()const { return "normal"; }//TODO:对工厂类给出的GUI建议，可能采用类似命令行的方式

	QVariant data;//数据
	QVariant defaultData;//默认值
	QHash<QString, QVariant> additionInfo;//附加信息

	QString description;//描述
	QList<std::function<bool(const QVariant&)>> assertFunctions;//输入校验

	AlgGraphNode& node;//从属的节点
	QList<AlgGraphVertex*> connectedVertexes;//连接到的端口

	std::atomic_bool isActivated = false;//激活标志
	std::atomic_bool isEnabled = true;//使能标志

	GuiGraphItemVertex*gui = nullptr;
signals:
	void sig_ActivateBegin();//激活开始
	void sig_Activated(QVariant var, bool is_Activated);//激活信号，可用来激活下一个节点
	void sig_ActivateEnd();//激活结束【不一定激活成功】

	void sig_ConnectionAdded(AlgGraphVertex*src, AlgGraphVertex*dst);
protected:
	//自定义激活部分，
	//通过后，isAct为true【存储数据】，激活当前Vertex，该Vertex会【发送sig_Activated()】进一步激活；
	//false则【清空数据】，且【不会发送】sig_Activated()进一步激活
	virtual void _Activate(QVariant var, bool isAct)
	{
		if (isAct == true)
		{
			data = var;
			isActivated = true;
			emit sig_Activated(var, true);
		}
		else
		{
			data.clear();
			isActivated = false;
		}
	}
};
class AlgGraphVertex_Input
	:public AlgGraphVertex
{
	Q_OBJECT
public:
	AlgGraphVertex_Input(AlgGraphNode&parent, QString name) :AlgGraphVertex(parent, name) {}

	enum class Behavior :unsigned char
	{
		KEEP = 0,//一直保持激活状态和数据（默认）
		DISPOSABLE = 1,//一次性，激活一次后信号和数据消失//TODO:可以用Node的激活信号连接Activate(0,false)来实现
		//BUFFER = 2,//缓冲，缓冲输入数据//TODO:这个现在的想法是使用Buffer Node实现
	}behavior=Behavior::KEEP;
};
class AlgGraphVertex_Output
	:public AlgGraphVertex
{
	Q_OBJECT
public:
	AlgGraphVertex_Output(AlgGraphNode&parent, QString name):AlgGraphVertex(parent,name){}
	virtual void _Activate(QVariant var, bool isDelay)//isDelay，是否延迟才发送【只存数据不发送】。与基类中相比，激活后必定自动清空
	{
		if (isDelay == true)
			data = var;
		else
		{
			data = var;
			emit sig_Activated(var, true);
			data.clear();
		}
	}
};
#pragma endregion

#pragma region AlgGraphNode
//算法节点
class AlgGraphNode
	:public QObject
{
	Q_OBJECT
public:
	AlgGraphNode(QObject*parent, QThreadPool&pool) :QObject(parent), _pool(pool) {}
	virtual ~AlgGraphNode() {}

	virtual void Init();
	virtual void Reset();//重置运行状态，清除所有运行时参数
	virtual void Release();
	virtual QString GetGuiAdvice()const { return "normal"; }

	virtual void AddVertex(QString name, QVariant defaultValue, bool isInput);
	virtual void AddVertex(QHash<QString, QVariant>initTbl, bool isInput);
	virtual void RemoveVertex(QString name);
	virtual void RemoveVertex(QStringList names);
	virtual void ConnectVertex(QString vertexName, AlgGraphNode&dstNode, QString dstVertexName);
	virtual void DisconnectVertex(QString vertexName);
	virtual void DisconnectVertex(QString vertexName, AlgGraphNode&dstNode, QString dstVertexName);

	virtual void Write();
	virtual void Read();

	void Activate();
	void Run(/*QMap<QString, QVariant>*/);
	void Output();
	void Pause(bool isPause);
	void Stop(bool isStop);

	void AttachGui(GuiGraphNode*gui) { _gui = gui; }
	const QHash<QString, AlgGraphVertex*>& GetVertexes(bool isInput)const { return isInput ? _inputVertex : _outputVertex; }
signals:
	void sig_VertexAdded(const AlgGraphVertex*vtx, bool isInput);
	void sig_VertexRemoved(const AlgGraphVertex*vtx, bool isInput);
	void sig_ConnectionAdded();
	void sig_ConnectionRemoved();

	void sig_VertexActivated(bool);
	void sig_NodeActivated();
	void sig_RunFinished();
	void sig_OutputFinished();
protected:
	QHash<QString, AlgGraphVertex*>_inputVertex;//输入节点
	QHash<QString, AlgGraphVertex*>_outputVertex;//输出节点

	QFutureWatcher<void> _result;//程序运行观测器
	QThreadPool&_pool;//使用的线程池
	QReadWriteLock _lock;//锁【可能并不需要，因为读写参数都发生在主线程】

	std::atomic_bool _isEnable = true;//使能
	std::atomic_bool _unchange = false;//不可在创建后修改
	std::atomic_bool _pause = false;//暂停标志
	std::atomic_bool _stop = false;//结束标志

	GuiGraphNode* _gui = nullptr;
};

class AlgGraphNode_Input
	:public AlgGraphNode
{
	Q_OBJECT
public:
	AlgGraphNode_Input(QObject*parent, QThreadPool&pool) :AlgGraphNode(parent, pool) { setObjectName("Input"); }

	virtual void Init() override;
	virtual void Input(QVariant var);
};
class AlgGraphNode_Output
	:public AlgGraphNode
{
	Q_OBJECT
public:
	AlgGraphNode_Output(QObject*parent, QThreadPool&pool) :AlgGraphNode(parent, pool) { setObjectName("Output"); }

	virtual void Init() override;
};
class AlgGraphNode_Add
	:public AlgGraphNode
{
	Q_OBJECT
public:
	AlgGraphNode_Add(QObject*parent, QThreadPool&pool) :AlgGraphNode(parent, pool) { setObjectName("Add"); }
	QHash<AlgGraphVertex*, int>idxTbl;//对应顺序
	virtual void Init() override;
};
class AlgGraphNode_Function
	:public AlgGraphNode
{
	Q_OBJECT
public:

};

#pragma endregion


enum
{
	VERTEX_TYPE=65536+0x100,
	ARROW_TYPE=65536+0x200,
	NODE_TYPE=65536+0x400,
};
class GuiGraphItemArrow
	:public QGraphicsLineItem
{
public:
	GuiGraphItemArrow(const QLineF &line, QGraphicsItem*parent) :QGraphicsLineItem(line, parent) {}
	GuiGraphItemArrow(const GuiGraphItemVertex*src, const GuiGraphItemVertex*dst)
		:QGraphicsLineItem(nullptr), srcVertex(src), dstVertex(dst)
	{
		updatePosition();
	}
	enum { Type = ARROW_TYPE };
	virtual int type()const { return Type; }

	void updatePosition();
	virtual QRectF boundingRect(void) const { return QGraphicsLineItem::boundingRect(); }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) { QGraphicsLineItem::paint(painter, option, widget); }

	const GuiGraphItemVertex*srcVertex = nullptr, *dstVertex = nullptr;//始末Vertex
protected:

};
class GuiGraphItemVertex
	:public QGraphicsItem
{
public:
	GuiGraphItemVertex(QGraphicsItem*parent, const AlgGraphVertex&vertex)
		:QGraphicsItem(parent), _vertex(vertex) {
		setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsScenePositionChanges);
	}
	enum{Type=VERTEX_TYPE};
	virtual int type()const { return Type; }

	void AddArrow(GuiGraphItemArrow*arrow) { assert(arrow != nullptr);  _arrows.append(arrow); }

	virtual QPointF ArrowAttachPosition()const//箭头连接点位置 
	{ return boundingRect().center(); }

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	const AlgGraphVertex&_vertex;
protected:

	int _mouseState = 0;
	QList<GuiGraphItemArrow*>_arrows;

	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value)
	{
		if (change == QGraphicsItem::ItemScenePositionHasChanged) {
			for (auto a : _arrows)	
				a->updatePosition();
		}
		return value;
	}
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override { _mouseState = 1; QGraphicsItem::hoverEnterEvent(event); }
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override { _mouseState = 0; QGraphicsItem::hoverLeaveEvent(event); }
// 	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override{ _mouseState = 1; QGraphicsItem::mousePressEvent(event); }
// 	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override { 	_mouseState = 0; QGraphicsItem::mouseReleaseEvent(event); }

};

class GuiGraphItemNode
	:public QGraphicsRectItem
{
public:
	GuiGraphItemNode(QRectF area, QGraphicsItem*parent) :QGraphicsRectItem(area, parent) 
	{
		setTransformOriginPoint(boundingRect().center());
		setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable);
		setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable);
	}

	enum { Type = NODE_TYPE };
	virtual int type()const { return Type; }
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value)
	{
		if (change == QGraphicsItem::ItemPositionChange) {

		}
		return value;
	}

protected:
};
class GuiGraphNode
	:public QObject
{
	Q_OBJECT
public:
	GuiGraphNode(const AlgGraphNode&node);
	QGraphicsItem* InitApperance();
	virtual QWidget* InitWidget(QWidget*parent);
	GuiGraphItemVertex*AddVertex(const AlgGraphVertex*vtx);
	GuiGraphItemArrow*AddConnection();
	QGraphicsScene*AttachScene(QGraphicsScene*scene) { scene->addItem(_nodeItem); return scene; }

	virtual QVariant GetData() { throw "NotImplement"; }
	virtual void SetData(QVariant var) { throw "NotImplement"; }

	const AlgGraphNode& _node;//对相应AlgGraphNode的常引用，只读不可写
signals:
	void sig_ValueChanged();//TODO:后面要下放到相应输入输出子类当中
protected:
	
	QHash<QString, const AlgGraphVertex*>_inputVertex;//输入节点
	QHash<QString, const AlgGraphVertex*>_outputVertex;//输入节点

	GuiGraphItemNode* _nodeItem=nullptr;//在场景中绘制和交互的物体
	QHash<QString, GuiGraphItemVertex*> _inputVertexItem;//输入节点
	QHash<QString, GuiGraphItemVertex*> _outputVertexItem;//输出节点
	QWidget* _panel;//面板控件，TODO:后面要下放到相应输入输出子类当中
};
class GraphScene
	:public QGraphicsScene
{
	Q_OBJECT
public:
	GraphScene(QObject*parent):QGraphicsScene(parent){}

signals:
	void sig_ConnectionAdded(GuiGraphItemVertex*src, GuiGraphItemVertex*dst);
protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

	GuiGraphItemArrow*arrow = nullptr;
};

class TestAnything : public QMainWindow
{
	Q_OBJECT
public:
	TestAnything(QWidget *parent = Q_NULLPTR);

	void AddNode(QString NodeName);
	void AddGuiNode(AlgGraphNode*node, QPointF center = QPointF(0, 0));
	void AddConnection(AlgGraphVertex*srcVertex, AlgGraphVertex*dstVertex);
	void AddConnection(GuiGraphItemVertex*srcVertex, GuiGraphItemVertex*dstVertex);

	void slot_Start(bool b);

private:
	Ui::TestAnythingClass ui;
	GraphScene _scene;
	QList<AlgGraphNode*>_nodes;
	QList<GuiGraphItemArrow*>_arrows;

	QThreadPool _pool;
};


#if 0
//TODO:InputVertex,OutputVertex,TriggerVertex,EnableVertex
class VertexInfo
{
public:
	QVariant param;
	QVariant defaultValue;
	
	QMap<QString, QVariant>additionInfo;
	//QString name;
	//QString description;
	std::function<bool(const VertexInfo&)>*assertFunction;

	QList<VertexInfo*>connectedVertexs;

	bool isActivated = false;
	bool isEnabled = true;

	//TODO:一些方便用的函数和转移语义
	//VertexInfo&operator=(VertexInfo&&);
	void Reset();
	void Release();
	void Load(QVariant var);
	//virtual bool _AssertFunction(QVariant var);
};

//TODO:派生：基本算法、常量、外部输入、外部输出、函数、条件、while、逻辑运算、延时、循环、缓冲
class AlgGraphNode
	:public QObject//, public Interface_Alg
{
	Q_OBJECT

public:

	AlgGraphNode(QObject*parent, QThreadPool&pool);
	virtual ~AlgGraphNode();

	void AddVertex(QString name,QVariant defaultValue,bool isInput);
	void AddVertex(QHash<QString, QVariant>initTbl, bool isInput);
	void RemoveVertex(QString name);
	void RemoveVertex(QStringList names);
	friend void ConnectVertex(AlgGraphNode&srcNode, QString srcVertexName, AlgGraphNode&dstNode, QString dstVertexName)
	{
		//TODO:合法性检查
		assert(srcNode._outputVertex.contains(srcVertexName) && dstNode._inputVertex.contains(dstVertexName));
		void (AlgGraphNode::*pActivate)(QVariant, VertexInfo*, bool) = &AlgGraphNode::Activate;//注意这里要这样写来区分重载函数
		connect(&srcNode, &AlgGraphNode::sig_Activate, &dstNode, pActivate);
		VertexInfo&srcV = srcNode._outputVertex[srcVertexName], &dstV = dstNode._inputVertex[dstVertexName];
		srcV.connectedVertexs.append(&dstV);
		dstV.connectedVertexs.append(&srcV);
	}

	void Reset();
	void Release();	

	void Activate(QVariant var = QVariant(), QString vtxName = QString(), bool b = true);
	void Activate(QVariant var,VertexInfo*vtx = nullptr, bool b = true);
	void Run(/ *QMap<QString,QVariant>* /);
	void Output();

	void Pause();
	void Stop();

	QStringList GetVertexNames()const;
	const VertexInfo* GetVertexInfo()const;
	QVariant GetVertexParam(QString vtxName, bool isInput)const { return isInput ? _inputVertex[vtxName].param : _outputVertex[vtxName].param; }

	//static AlgGraphNode* Create();
signals:
	void sig_VertexAdded(const VertexInfo*vtx,bool isInput);
	void sig_ConnectionAdded();

	void sig_Activate(QVariant var = QVariant(), VertexInfo*vtx = nullptr, bool b = true);
	void sig_VertexActivated(bool);
	void sig_NodeActivated(const AlgGraphNode*node);
	void sig_TaskFinished(const AlgGraphNode*node);
	void sig_Output(QVariant var = QVariant(), VertexInfo*srcVertex = nullptr/ *, bool b = true* /);
	void sig_OutputFinished();

	void sig_ReportProgress(float);
	void sig_ResultReady();
protected:
	virtual void _LoadInput(){}
	virtual void _Run();
	virtual void _LoadOutput(){}

	//bool _enable = true;
	
	QFutureWatcher<void>_result;
	QHash<QString, VertexInfo>_inputVertex;
	QHash<QString, VertexInfo>_outputVertex;

	QThreadPool&_pool;
};
//TODO:扩展：不同行为
class GuiGraphNode
	:public QObject
{	
	Q_OBJECT
public:
	GuiGraphNode(const AlgGraphNode*algnode,qreal x,qreal y,qreal width,qreal height,QObject*parent)
		:QObject(parent),guiItems(nullptr),_algnode(algnode)
	{
		connect(_algnode, &AlgGraphNode::destroyed, this, &QObject::deleteLater);
		
		Init(QRectF(x,y,width,height));
		
	}
	virtual ~GuiGraphNode()
	{
		qDebug() << __FUNCSIG__;
	}
	virtual void Init(QRectF area)
	{
		guiItems = new QGraphicsItemGroup(nullptr);
		guiItems->setFlag(QGraphicsItem::ItemIsMovable);
		border = new QGraphicsRectItem(area, guiItems);
		border->setTransformOriginPoint(border->boundingRect().center());//不加这一句那就是从原点开始缩放
		guiItems->addToGroup(border);
		title = new QGraphicsTextItem(_algnode->objectName(), guiItems);
		title->setPos(border->boundingRect().center());
		title->setTransformOriginPoint(border->boundingRect().center());
		guiItems->addToGroup(title);	
		qDebug() << guiItems->pos() << border->pos() << title->pos();
		
		connect(_algnode, &AlgGraphNode::sig_NodeActivated, this, [this]
		{
			border->setScale(1.25);
			title->setPos(border->boundingRect().center());
		});
		connect(_algnode, &AlgGraphNode::sig_TaskFinished, this, [this]
		{
			border->setScale(1/1.25);
			title->setPos(border->boundingRect().center());
		});
		connect(_algnode, &AlgGraphNode::sig_Output, this, [this](QVariant var)
		{
			title->setPlainText(var.toString());
		});
	}
	QGraphicsItemGroup *guiItems;
	QGraphicsItem*border;
	QGraphicsTextItem*title;
	QList<QGraphicsItem*>inputs, outputs;

	const AlgGraphNode*_algnode;
};

class GraphScene
	:public QObject
{
	Q_OBJECT
public:
	GraphScene(QObject*parent)
		:QObject(parent)
	{}

	void AddNode(const AlgGraphNode*node, qreal x, qreal y, qreal width, qreal height)
	{
		nodes[node] = new GuiGraphNode(node, x, y, width, height,this);
		connect(node, &QObject::destroyed, this, [this](QObject*obj)
		{
			nodes.remove(qobject_cast<const AlgGraphNode*> (obj));
		});
		scene.addItem(nodes[node]->guiItems);
	}
	void AddConnection(const AlgGraphNode*srcNode,const QString srcVertexName,
		const AlgGraphNode*dstNode, const QString dstVertexName);

	QHash<const AlgGraphNode*, GuiGraphNode*>nodes;
	QGraphicsScene scene;
};
#endif