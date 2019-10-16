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
//Alg��Vertex����Ҫ�������ݴ洢�����ݡ�����Ĺ���
class AlgGraphVertex
	:public QObject
{
	Q_OBJECT
public:
	AlgGraphVertex(AlgGraphNode&parent,QString name)
		:QObject(reinterpret_cast<QObject*>(&parent)), node(parent) {setObjectName(name);}
	virtual ~AlgGraphVertex() {}

	//������������ʹ�ܣ�isEnable==true�����ȵ������е�assertFunction��У�飬ͨ���󡾵���_Activate()������
	void Activate(QVariant var, bool isActivate = true)
	{
		if (isEnabled == true)
		{
			emit sig_ActivateBegin();
			for (auto f : assertFunctions)
				if (f(var) == false)
					throw "AssertFail";//TODO:1.�ĳ�ר�õ�GraphError��2.����Ĭ�ϵ�����ȷ�ϲ���

			_Activate(var, isActivate);
			emit sig_ActivateEnd();
		}
	}
	//���������ڵ㣬����this=>dstVertex����Ҫ���޸�connectedVertexes������this=>dstVertex�ļ����ź�
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
// 		for (auto v : connectedVertexes)//TODO:����������Ҫ����źŲ�
// 			disconnect(v);
		connectedVertexes.clear();		
	}

	virtual QStringList Write()const {throw "Not Implement"; }//TODO:�־û�������ڵ���Ϣ�ͽṹ��*�Ƿ񱣴����ݣ���
	virtual void Read(QStringList){ throw "Not Implement";}//TODO:�ӳ־û���Ϣ�лָ�
	virtual QString GetGuiAdvice()const { return "normal"; }//TODO:�Թ����������GUI���飬���ܲ������������еķ�ʽ

	QVariant data;//����
	QVariant defaultData;//Ĭ��ֵ
	QHash<QString, QVariant> additionInfo;//������Ϣ

	QString description;//����
	QList<std::function<bool(const QVariant&)>> assertFunctions;//����У��

	AlgGraphNode& node;//�����Ľڵ�
	QList<AlgGraphVertex*> connectedVertexes;//���ӵ��Ķ˿�

	std::atomic_bool isActivated = false;//�����־
	std::atomic_bool isEnabled = true;//ʹ�ܱ�־

	GuiGraphItemVertex*gui = nullptr;
signals:
	void sig_ActivateBegin();//���ʼ
	void sig_Activated(QVariant var, bool is_Activated);//�����źţ�������������һ���ڵ�
	void sig_ActivateEnd();//�����������һ������ɹ���

	void sig_ConnectionAdded(AlgGraphVertex*src, AlgGraphVertex*dst);
protected:
	//�Զ��弤��֣�
	//ͨ����isActΪtrue���洢���ݡ������ǰVertex����Vertex�᡾����sig_Activated()����һ�����
	//false��������ݡ����ҡ����ᷢ�͡�sig_Activated()��һ������
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
		KEEP = 0,//һֱ���ּ���״̬�����ݣ�Ĭ�ϣ�
		DISPOSABLE = 1,//һ���ԣ�����һ�κ��źź�������ʧ//TODO:������Node�ļ����ź�����Activate(0,false)��ʵ��
		//BUFFER = 2,//���壬������������//TODO:������ڵ��뷨��ʹ��Buffer Nodeʵ��
	}behavior=Behavior::KEEP;
};
class AlgGraphVertex_Output
	:public AlgGraphVertex
{
	Q_OBJECT
public:
	AlgGraphVertex_Output(AlgGraphNode&parent, QString name):AlgGraphVertex(parent,name){}
	virtual void _Activate(QVariant var, bool isDelay)//isDelay���Ƿ��ӳٲŷ��͡�ֻ�����ݲ����͡������������ȣ������ض��Զ����
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
//�㷨�ڵ�
class AlgGraphNode
	:public QObject
{
	Q_OBJECT
public:
	AlgGraphNode(QObject*parent, QThreadPool&pool) :QObject(parent), _pool(pool) {}
	virtual ~AlgGraphNode() {}

	virtual void Init();
	virtual void Reset();//��������״̬�������������ʱ����
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
	QHash<QString, AlgGraphVertex*>_inputVertex;//����ڵ�
	QHash<QString, AlgGraphVertex*>_outputVertex;//����ڵ�

	QFutureWatcher<void> _result;//�������й۲���
	QThreadPool&_pool;//ʹ�õ��̳߳�
	QReadWriteLock _lock;//�������ܲ�����Ҫ����Ϊ��д���������������̡߳�

	std::atomic_bool _isEnable = true;//ʹ��
	std::atomic_bool _unchange = false;//�����ڴ������޸�
	std::atomic_bool _pause = false;//��ͣ��־
	std::atomic_bool _stop = false;//������־

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
	QHash<AlgGraphVertex*, int>idxTbl;//��Ӧ˳��
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

	const GuiGraphItemVertex*srcVertex = nullptr, *dstVertex = nullptr;//ʼĩVertex
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

	virtual QPointF ArrowAttachPosition()const//��ͷ���ӵ�λ�� 
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

	const AlgGraphNode& _node;//����ӦAlgGraphNode�ĳ����ã�ֻ������д
signals:
	void sig_ValueChanged();//TODO:����Ҫ�·ŵ���Ӧ����������൱��
protected:
	
	QHash<QString, const AlgGraphVertex*>_inputVertex;//����ڵ�
	QHash<QString, const AlgGraphVertex*>_outputVertex;//����ڵ�

	GuiGraphItemNode* _nodeItem=nullptr;//�ڳ����л��ƺͽ���������
	QHash<QString, GuiGraphItemVertex*> _inputVertexItem;//����ڵ�
	QHash<QString, GuiGraphItemVertex*> _outputVertexItem;//����ڵ�
	QWidget* _panel;//���ؼ���TODO:����Ҫ�·ŵ���Ӧ����������൱��
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

	//TODO:һЩ�����õĺ�����ת������
	//VertexInfo&operator=(VertexInfo&&);
	void Reset();
	void Release();
	void Load(QVariant var);
	//virtual bool _AssertFunction(QVariant var);
};

//TODO:�����������㷨���������ⲿ���롢�ⲿ�����������������while���߼����㡢��ʱ��ѭ��������
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
		//TODO:�Ϸ��Լ��
		assert(srcNode._outputVertex.contains(srcVertexName) && dstNode._inputVertex.contains(dstVertexName));
		void (AlgGraphNode::*pActivate)(QVariant, VertexInfo*, bool) = &AlgGraphNode::Activate;//ע������Ҫ����д���������غ���
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
//TODO:��չ����ͬ��Ϊ
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
		border->setTransformOriginPoint(border->boundingRect().center());//������һ���Ǿ��Ǵ�ԭ�㿪ʼ����
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