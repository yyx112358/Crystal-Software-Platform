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
class AlgGraphNode;

class GuiGraphVertex;
class GuiGraphNode;

class GraphScene;
class GraphController;
class GraphError;
class GraphWarning;

class TestAnything;

class AlgGraphNode
	:public QObject
{
	Q_OBJECT
public:
	AlgGraphNode(QObject*parent, QThreadPool&pool) :QObject(parent), _pool(pool) {}
	virtual ~AlgGraphNode() {}

	void Init();
	void Reset();
	void Release();

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
	void Pause();
	void Stop();

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
	QHash<QString, AlgGraphVertex>_inputVertex;//����ڵ�
	QHash<QString, AlgGraphVertex>_outputVertex;//����ڵ�

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
	AlgGraphNode_Input(QObject*parent, QThreadPool&pool) :AlgGraphNode(parent,pool) {}
};

class AlgGraphNode_Output
	:public AlgGraphNode
{
	Q_OBJECT
public:
	AlgGraphNode_Output(QObject*parent, QThreadPool&pool) :AlgGraphNode(parent, pool) {}
};

class AlgGraphVertex
	:public QObject
{
	Q_OBJECT
public:
	AlgGraphVertex(AlgGraphNode&parent) :QObject(&parent), node(parent) {  }
	virtual ~AlgGraphVertex(){}

	virtual void Activate(QVariant var, bool b = true);
	virtual void Connect(AlgGraphVertex*another);
	virtual void Write();
	virtual void Read();

	QVariant data;//����
	QVariant defaultData;//Ĭ��ֵ
 	QHash<QString, QVariant> additionInfo;//������Ϣ
 
	QString description;//����
	QList<std::function<bool(const QVariant&)>> assertFunctions;//����У��

	AlgGraphNode& node;//�����Ľڵ�
	QList<AlgGraphVertex*> connectedVertexes;//���ӵ��Ķ˿�

	std::atomic_bool is_Activated = false;//�����־
	std::atomic_bool is_Enabled = true;//ʹ�ܱ�־

	GuiGraphVertex*gui = nullptr;
signals:
	void sig_ActivateBegin();
	void sig_ActivateEnd();
};
class AlgGraphVertex_Input
	:public AlgGraphVertex
{
	Q_OBJECT
public:
	enum class Behavior :unsigned char
	{
		KEEP = 0,//һֱ���ּ���״̬�����ݣ�Ĭ�ϣ�
		DISPOSABLE = 1,//һ���ԣ�����һ�κ��źź�������ʧ
		//BUFFER = 2,//���壬������������
	};
};
class AlgGraphVertex_Output
	:public AlgGraphVertex
{
	Q_OBJECT
public:
	virtual void Activate(QVariant var, bool isDelay)//isDelay���Ƿ��ӳٲŷ��ͣ�ֻ�����ݲ����ͣ�
	{
		if (isDelay == true)
			data = var;
		else
		{
			emit sig_ActivateBegin();

			emit sig_Activate(var, true);
			var.clear();

			emit sig_ActivateEnd();
		}
	}
signals:
	void sig_Activate(QVariant var, bool b);
};


class AlgGraphNode_Function
	:public AlgGraphNode
{
	Q_OBJECT
public:

};

class GuiGraphVertex
	:public QGraphicsItem
{
public:

protected:
};
class GuiGraphArrow
	:public QGraphicsItem
{
public:
	virtual QRectF boundingRect(void) const { throw "Not Implement!"; }
	virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) { throw "Not Implement!"; }
protected:
};
class GuiGraphNode
	:public QObject
{
	Q_OBJECT
public:

protected:
	QGraphicsItem*_item;

	QHash<QString, GuiGraphVertex*>_inputVertex;//����ڵ�
	QHash<QString, GuiGraphVertex*>_outputVertex;//����ڵ�
	QHash<QString, QWidget*>_panels;//���ؼ�
};



class TestAnything : public QMainWindow
{
	Q_OBJECT
	friend class GraphController;  
public:
	TestAnything(QWidget *parent = Q_NULLPTR);

	void AddNode(const AlgGraphNode*node, QPointF center);

	void slot_Start(bool b);

	GraphController *controller;
	//QHash<QString,AlgGraphNode*>nodes;	
private:
	Ui::TestAnythingClass ui;
	QGraphicsScene _scene;
	QList<AlgGraphNode>_nodes;
	QList<GuiGraphArrow>_arrows;

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