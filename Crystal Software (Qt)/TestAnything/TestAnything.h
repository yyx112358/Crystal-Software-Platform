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
#include <QLabel>
#include <QPlainTextEdit>

class AlgGraphVertex;
class AlgGraphVertex_Input;
class AlgGraphNode;

class GuiGraphItemArrow;
class GuiGraphItemNode;
class GuiGraphItemVertex;
class GuiGraphController;

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
	AlgGraphVertex(AlgGraphNode&parent, QString name)
		:QObject(reinterpret_cast<QObject*>(&parent)), node(parent) {
		setObjectName(name);
		_amount++;
	}

	virtual ~AlgGraphVertex()
	{
		qDebug() << objectName() << __FUNCTION__;
		emit sig_Destroyed(&node, this);
		Clear();
		if (gui != nullptr)
		{
			delete gui;
			gui = nullptr;
		}
		_amount--;
	}

	//《激活函数》如果使能（isEnable==true）首先调用所有的assertFunction做校验，通过后【调用_Activate()函数】
	void Activate(QVariant var, bool isActivate = true);
	//连接两个节点，方向this=>dstVertex，主要是修改connectedVertexes并连接this=>dstVertex的激活信号
	void Connect(AlgGraphVertex*dstVertex)
	{
		assert(dstVertex != nullptr);
		connectedVertexes.append(dstVertex);
		dstVertex->connectedVertexes.append(this);
		emit sig_ConnectionAdded(this, dstVertex);
	}
	void Disconnect(AlgGraphVertex*another)
	{
		if (connectedVertexes.removeAll(another) > 0)//如果确实连接了another
		{
			another->connectedVertexes.removeAll(this);
			if (disconnect(another) == true)//清除连接
				emit sig_ConnectionRemoved(this, another);
			/*else */if (another->disconnect(this) == true)
				emit sig_ConnectionRemoved(another, this);
		}
	}
	//重置，主要是清除运行时状态（图运行后会改变的，主要是数据及激活位）
	virtual void Reset()
	{
		data.clear();
		isActivated = false;
		emit sig_Reseted(this);
	}
	//清除，清除运行时状态和动态状态（指调节图时候可变的，主要是使能位isEnabled和连接节点connectedVertexes）
	//TODO:是否加入isUnchange参数，设定其它参数是否可变？
	virtual void Clear()
	{
		Reset();
		isEnabled = true;
		while (connectedVertexes.size() > 0)//清除connectedVertexes
		{
			if (connectedVertexes.back().isNull() == false)//如果不是无效节点
				Disconnect(connectedVertexes.back());//Disconnect()会清除本节点
			else
				connectedVertexes.pop_back();
		}
		emit sig_Cleared(this);
	}
	//释放，释放所有成员为空，成为初始状态
	virtual void Release()
	{
// 		Clear();
// 		defaultData.clear();
// 		additionInfo.clear();
// 		assertFunctions.clear();
// 
// 		disconnect();
// 		connectedVertexes.clear();
// 		if (gui != nullptr)
// 			delete gui;
// 
// 		emit sig_Released(this);
	}

	virtual QStringList Write()const { throw "Not Implement"; }//TODO:持久化，保存节点信息和结构（*是否保存数据？）
	virtual void Read(QStringList) { throw "Not Implement"; }//TODO:从持久化信息中恢复
	virtual QString GetGuiAdvice()const { return "normal"; }//TODO:对工厂类给出的GUI建议，可能采用类似命令行的方式
	static size_t GetAmount() { return _amount; }

	QVariant data;//数据
	QVariant defaultData;//默认值
	QHash<QString, QVariant> additionInfo;//附加信息

	QString description;//描述
	QList<std::function<bool(const QVariant&)>> assertFunctions;//输入校验

	AlgGraphNode& node;//从属的节点
	QList<QPointer<AlgGraphVertex>> connectedVertexes;//连接到的端口

	std::atomic_bool isActivated = false;//激活标志
	std::atomic_bool isEnabled = true;//使能标志

	GuiGraphItemVertex*gui = nullptr;//连接的图形//TODO:改成Node当中使用Attach和Detach函数并做检验的形式
signals:
	void sig_ActivateBegin();//激活开始
	void sig_Activated(QVariant var, bool is_Activated);//激活信号，可用来激活下一个节点
	void sig_ActivateEnd();//激活结束【不一定激活成功】

	void sig_ConnectionAdded(AlgGraphVertex*src, AlgGraphVertex*dst);//连接建立成功
	void sig_ConnectionRemoved(AlgGraphVertex*src, AlgGraphVertex*dst);//连接移除成功
	void sig_Reseted(AlgGraphVertex*);//重置成功
	void sig_Cleared(AlgGraphVertex*);//清空成功
	void sig_Released(AlgGraphVertex*);//释放成功

	void sig_Destroyed(AlgGraphNode*node, AlgGraphVertex*vertex);//删除成功
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
	static size_t _amount;
};
class AlgGraphVertex_Input
	:public AlgGraphVertex
{
	Q_OBJECT
public:
	AlgGraphVertex_Input(AlgGraphNode&parent, QString name) :AlgGraphVertex(parent, name) {}
	virtual ~AlgGraphVertex_Input() {  }

	//virtual void Init();
	enum class Behavior :unsigned char
	{
		KEEP = 0,//一直保持激活状态和数据（默认）
		DISPOSABLE = 1,//一次性，激活一次后信号和数据消失//TODO:可以用Node的激活信号连接Activate(0,false)来实现
					   //BUFFER = 2,//缓冲，缓冲输入数据//TODO:这个现在的想法是使用Buffer Node实现
	}behavior = Behavior::KEEP;
};
class AlgGraphVertex_Output
	:public AlgGraphVertex
{
	Q_OBJECT
public:
	AlgGraphVertex_Output(AlgGraphNode&parent, QString name) :AlgGraphVertex(parent, name) {}
	virtual ~AlgGraphVertex_Output() { Release(); }

	virtual void _Activate(QVariant var, bool isNow)//isNow，是否立刻发送【否则只存数据不发送】。与基类中相比，激活后必定自动清空
	{
		if (isNow == false)
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
//TODO:派生：基本算法、常量、外部输入、外部输出、函数、条件、while、逻辑运算、延时、循环、缓冲
class AlgGraphNode
	:public QObject
{
	Q_OBJECT
public:
	enum class RunMode
	{
		Thread,//多线程方式【默认】
		Direct,//简单直接方式，不开多线程
		Function,//函数方式（Node内部嵌套另一个Graph）
	};

	AlgGraphNode(QObject*parent, QThreadPool&pool);
	virtual ~AlgGraphNode();

	virtual void Init();//初始化一些参数，如果初始化后将_isUnchange设为true，则不能更改设定
	virtual void Reset();//重置运行状态，清除所有运行时参数（运行后可变的参数）
	virtual void Clear() {}
	virtual void Release();
	virtual QString GetGuiAdvice()const { return "normal"; }

	virtual AlgGraphVertex* AddVertex(QString name, QVariant defaultValue, bool isInput);//添加默认节点，并连接输入节点的sig_Activated()信号
	virtual QHash<QString, AlgGraphVertex*> AddVertex(QHash<QString, QVariant>initTbl, bool isInput);
	virtual void RemoveVertex(QString name,bool isInput);
	virtual void RemoveVertex(QStringList names, bool isInput);
	virtual void ConnectVertex(QString vertexName, AlgGraphNode&dstNode, QString dstVertexName);
	virtual void DisconnectVertex(QString vertexName);
	virtual void DisconnectVertex(QString vertexName, AlgGraphNode&dstNode, QString dstVertexName);

	virtual void Write();
	virtual void Read();

	void Activate();
	void Run();
	void Output();
	void Pause(bool isPause);
	void Stop(bool isStop);

	void AttachGui(GuiGraphController*gui) { assert(_gui == nullptr); _gui = gui; }
	void DetachGui() { assert(_gui != nullptr); _gui = nullptr; }//TODO:检查GuiGraphNode析构后的安全性
	bool isHasGui()const { return _gui != nullptr; }
	const QHash<QString, AlgGraphVertex*>& GetVertexes(bool isInput)const { return isInput ? _inputVertex : _outputVertex; }

	static size_t GetAmount() { return _amount; }
signals:
	void sig_VertexAdded(const AlgGraphVertex*vtx, bool isInput);
	void sig_VertexRemoved(const AlgGraphVertex*vtx, bool isInput);
	void sig_ConnectionAdded();
	void sig_ConnectionRemoved();

	void sig_ActivatedFinished(AlgGraphNode*node);//节点被激活
	void sig_RunFinished(AlgGraphNode*node);//运行结束
	void sig_OutputFinished(AlgGraphNode*node);//输出结束

	void sig_Destroyed(AlgGraphNode*node);
protected:
	//用于迭代处理_inputVertex和_outputVertex的模板函数，func是lambda表达式。函数将依次迭代vtxs中元素，如果不存在会输出错误并删除
	//示例：_ProcessVertex(_inputVertex,[this](AlgGraphVertex*vertex){vertex->Reset();})
	template<typename F>
	bool _ProcessVertex(QHash<QString, QPointer<AlgGraphVertex>>&vtxs, F const&func)
	{
		for (auto it = vtxs.begin(); it != vtxs.end();)
		{
			if (it->isNull() == false)
			{
				func(*it);
				++it;
			}
			else
			{
				qDebug() << __FUNCTION__ << it.key() << "Not exist";
				it = vtxs.erase(it);
			}
		}
	}
	virtual QVariantHash _LoadInput();/*自定义读取输入，默认直接将数据从Vertex当中复制一份，以免运行过程中输入被修改*/
	virtual QVariantHash _Run(QVariantHash data);//主要的运行部分，【将在另一个线程中运行】
	virtual void _LoadOutput(QVariantHash result);/*自定义加载输出，默认直接将数据从临时数据中加载到输出中*/

	QHash<QString, AlgGraphVertex*>_inputVertex;//输入节点
	QHash<QString, AlgGraphVertex*>_outputVertex;//输出节点

	QFutureWatcher<QVariantHash> _result;//程序运行观测器
	QThreadPool&_pool;//使用的线程池
	QReadWriteLock _lock;//锁【可能并不需要，因为读写参数都发生在主线程】

	RunMode _mode = RunMode::Thread;//运行方式

	std::atomic_bool _isEnable = true;//使能
	std::atomic_bool _isUnchange = false;//不可在创建后修改
	std::atomic_bool _pause = false;//暂停标志
	std::atomic_bool _stop = false;//结束标志

	QPointer<GuiGraphController> _gui = nullptr;
	static size_t _amount;
};

class AlgGraphNode_Input
	:public AlgGraphNode
{
	Q_OBJECT
public:
	AlgGraphNode_Input(QObject*parent, QThreadPool&pool) :AlgGraphNode(parent, pool) {  }
	virtual ~AlgGraphNode_Input() {}

	virtual void Init() override;
protected:
	virtual QVariantHash _Run(QVariantHash data) override;

};
class AlgGraphNode_Output
	:public AlgGraphNode
{
	Q_OBJECT
public:
	AlgGraphNode_Output(QObject*parent, QThreadPool&pool) :AlgGraphNode(parent, pool) {  }
	virtual ~AlgGraphNode_Output() {}

	virtual void Init() override;
protected:
	virtual QVariantHash _Run(QVariantHash data) override;

};
class AlgGraphNode_Add
	:public AlgGraphNode
{
	Q_OBJECT
public:
	AlgGraphNode_Add(QObject*parent, QThreadPool&pool) :AlgGraphNode(parent, pool) { setObjectName("Add"); }
	QHash<AlgGraphVertex*, int>idxTbl;//对应顺序
	virtual void Init() override;
protected:
	virtual QVariantHash _Run(QVariantHash data) override;

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
	VERTEX_TYPE = 65536 + 0x100,
	ARROW_TYPE = 65536 + 0x200,
	NODE_TYPE = 65536 + 0x400,
};
class GuiGraphItemArrow
	:public QGraphicsLineItem
{
public:
	//GuiGraphItemArrow(const QLineF &line, QGraphicsItem*parent) :QGraphicsLineItem(line, parent) {}
	GuiGraphItemArrow(const GuiGraphItemVertex*src, const GuiGraphItemVertex*dst);
	virtual ~GuiGraphItemArrow();
	enum { Type = ARROW_TYPE };
	virtual int type()const { return Type; }
	static size_t GetAmount() { return _amount; }

	void updatePosition();
	virtual QRectF boundingRect(void) const { return QGraphicsLineItem::boundingRect(); }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

	const GuiGraphItemVertex*srcItemVertex, *dstItemVertex;//始末Vertex
protected:
	static size_t _amount;
};
class GuiGraphItemVertex
	:public QGraphicsItem
{
public:
	GuiGraphItemVertex(GuiGraphItemNode&parent, const AlgGraphVertex&vertex,bool isInput);
	virtual ~GuiGraphItemVertex();
	enum { Type = VERTEX_TYPE };
	virtual int type()const { return Type; }
	static size_t GetAmount() { return _amount; }

	virtual QPointF ArrowAttachPosition()const//箭头连接点位置 
	{
		return boundingRect().center();
	}

	virtual QRectF boundingRect() const override
	{
		QFontMetrics fm(scene() != nullptr ? (scene()->font()) : (QApplication::font()));
		return QRectF(0, 0, fm.width(_vertex.objectName()), fm.height());
	}
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override
	{
		painter->drawText(0, boundingRect().height(), _vertex.objectName()/*,QTextOption(Qt::AlignmentFlag::AlignCenter)*/);
		if (isSelected() == true)
			painter->setBrush(Qt::Dense5Pattern);
		if (_mouseState == 1)
			painter->drawRect(boundingRect());
		else
		{
			painter->setPen(QPen(QColor(200, 200, 200)));
			//painter->setBrush(QBrush(QColor(100, 100, 100), Qt::BrushStyle::SolidPattern));
			painter->drawRect(boundingRect());
		}
	}

	const bool _isInput;
	char _mouseState = 0;

	const AlgGraphVertex&_vertex;
	const GuiGraphItemNode&_nodeItem;
	QList<GuiGraphItemArrow*>_arrows; 
	static size_t _amount;

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

};

class GuiGraphItemNode
	:public QGraphicsRectItem
{
public:
	GuiGraphItemNode(QRectF area, QGraphicsItem*parent, GuiGraphController&holder)
		:QGraphicsRectItem(area, parent), controller(holder), title(this)
	{
		setTransformOriginPoint(boundingRect().center());
		setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable);
		//setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable);
		setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
		_amount++;
	}
	virtual ~GuiGraphItemNode();//析构时候自动解离
	enum { Type = NODE_TYPE };
	virtual int type()const { return Type; }
	static size_t GetAmount() { return _amount; }

	virtual void Refresh();

	GuiGraphController&controller;
	QGraphicsTextItem title;
	QHash<const AlgGraphVertex*, GuiGraphItemVertex*>inputItemVertex;
	QHash<const AlgGraphVertex*, GuiGraphItemVertex*>outputItemVertex;
	static size_t _amount;
};
//Node在Gui的控制器，更多的起到一个在AlgGraphNode和GuiGraphItemNode之间中转的作用
class GuiGraphController
	:public QObject
{
	Q_OBJECT
public:
	GuiGraphController(const AlgGraphNode&node, GraphScene&scene);
	virtual ~GuiGraphController();

	virtual GuiGraphItemNode* InitApperance(QPointF center = QPointF(0, 0), QRectF size = QRectF(0, 0, 100, 100));//根据AlgGraphNode的信息初始化GuiGraphItemNode
	virtual QWidget* InitWidget(QWidget*parent);

	GuiGraphItemVertex*AddVertex(const AlgGraphVertex*vtx, const bool isInput);
	GuiGraphItemArrow*AddConnection();
	void DetachItem() { assert(_nodeItem != nullptr); _nodeItem = nullptr; }

	virtual QVariant GetData() { throw "NotImplement"; }
	virtual void SetData(QVariant var) { throw "NotImplement"; }
	static size_t GetAmount() { return _amount; }

	const AlgGraphNode& _node;//对相应AlgGraphNode的常引用，只读不可写
signals:
	void sig_Destroyed(GuiGraphController*node);//析构前发出的信号
	void sig_ValueChanged();//TODO:后面要下放到相应输入输出子类当中
protected:

	GraphScene&_scene;
	GuiGraphItemNode* _nodeItem = nullptr;//在场景中绘制和交互的物体，其析构时会自动解离
	QPointer<QWidget> _panel = nullptr;//面板控件，TODO:后面要下放到相应输入输出子类当中
	static size_t _amount;
};

class GuiGraphController_Input
	:public GuiGraphController
{
	Q_OBJECT
public:
	GuiGraphController_Input(const AlgGraphNode&node, GraphScene&scene) :GuiGraphController(node, scene) {}
	virtual ~GuiGraphController_Input() {}
	virtual QWidget* InitWidget(QWidget*parent)
	{
		_panel = new QPlainTextEdit(parent);
		connect(qobject_cast<QPlainTextEdit*>(_panel), &QPlainTextEdit::textChanged, this, &GuiGraphController::sig_ValueChanged);
		return _panel;
	}
	virtual QVariant GetData() { return (qobject_cast<QPlainTextEdit*>(_panel))->toPlainText(); }
	//virtual void SetData(QVariant var) { throw "NotImplement"; }
};
class GuiGraphController_Output
	:public GuiGraphController
{
	Q_OBJECT
public:
	GuiGraphController_Output(const AlgGraphNode&node, GraphScene&scene) :GuiGraphController(node, scene) {}
	virtual ~GuiGraphController_Output() {}
	virtual QWidget* InitWidget(QWidget*parent)
	{
		_panel = new QLabel(parent);
		return _panel;
	}
	virtual void SetData(QVariant var) { (qobject_cast<QLabel*>(_panel))->setText(var.toString()); }
};

class GraphScene
	:public QGraphicsScene
{
	Q_OBJECT
public:
	GraphScene(QObject*parent) :QGraphicsScene(parent) {}
	virtual ~GraphScene() { if (arrow != nullptr)delete arrow; }
signals:
	void sig_ConnectionAdded(GuiGraphItemVertex*src, GuiGraphItemVertex*dst);
	void sig_RemoveItems(QList<QGraphicsItem*>items);
protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;//主要处理画线
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;

	GuiGraphItemArrow*arrow = nullptr;
};

class GraphView
	:public QGraphicsView
{
	Q_OBJECT
public:
	GraphView(QWidget*parent) :QGraphicsView(parent) {}
	virtual ~GraphView() {}
protected:
	virtual void wheelEvent(QWheelEvent *event) override;
	//virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

};

class TestAnything : public QMainWindow
{
	Q_OBJECT
public:
	TestAnything(QWidget *parent = Q_NULLPTR);
	~TestAnything();

	AlgGraphNode& AddNode(AlgGraphNode&node, GuiGraphController*guiNode = nullptr, QPointF center = QPointF(0, 0));//添加已创建的node并配置相应的guiNode，如果guiNode为nullptr，则添加一个默认的
	GuiGraphController* AddGuiNode(AlgGraphNode&node, GuiGraphController*guiNode, QPointF center = QPointF(0, 0));//给node添加显示部分，如果guiNode为nullptr则添加默认的
	bool AddConnection(AlgGraphNode&srcNode, QString srcNodeName, bool srcIsInput, AlgGraphNode&dstNode, QString dstNodeName, bool dstIsInput);
	bool AddConnection(AlgGraphVertex*srcVertex, AlgGraphVertex*dstVertex);//添加连接并显示
	bool AddConnection(GuiGraphItemVertex*srcVertex, GuiGraphItemVertex*dstVertex);//在GUI上添加连接（实际调用AddConnection(AlgGraphVertex*, AlgGraphVertex*)）

	void AddNodeAsAdvice(QString advice);//TODO:根据命令行添加

	bool RemoveNode(AlgGraphNode*node);
	bool RemoveVertex(AlgGraphVertex*vertex);
	void RemoveConnection(AlgGraphVertex*src, AlgGraphVertex*dst);


	void slot_Start(bool b);

private:
	void slot_RemoveItems(QList<QGraphicsItem*>items);

	Ui::TestAnythingClass ui;
	GraphScene _scene;
	QList<AlgGraphNode*>_nodes;

	QGraphicsItem*selectedItem = nullptr;
	int _monitorTimerId = 0;

	QThreadPool _pool;
protected:
	virtual void timerEvent(QTimerEvent *event) override;

};

