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

class GuiGraphItemVertex;
class GuiGraphVertex;
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
	AlgGraphVertex(AlgGraphNode&parent, QString name)
		:QObject(reinterpret_cast<QObject*>(&parent)), node(parent) {
		setObjectName(name);
	}
	virtual ~AlgGraphVertex() { qDebug() << objectName() << __FUNCTION__; emit sig_Destroyed(&node, this); }

	//������������ʹ�ܣ�isEnable==true�����ȵ������е�assertFunction��У�飬ͨ���󡾵���_Activate()������
	void Activate(QVariant var, bool isActivate = true);
	//���������ڵ㣬����this=>dstVertex����Ҫ���޸�connectedVertexes������this=>dstVertex�ļ����ź�
	void Connect(AlgGraphVertex*dstVertex);
	void Disconnect(AlgGraphVertex*another);
	//���ã���Ҫ���������ʱ״̬��ͼ���к��ı�ģ���Ҫ�����ݼ�����λ��
	virtual void Reset(){}
	//������������ʱ״̬�Ͷ�̬״̬��ָ����ͼʱ��ɱ�ģ���Ҫ��ʹ��λisEnabled�����ӽڵ�connectedVertexes��
	//TODO:�Ƿ����isUnchange�������趨���������Ƿ�ɱ䣿
	virtual void Clear(){}
// 	//�ͷţ��ͷ����г�ԱΪ�գ���Ϊ��ʼ״̬
// 	virtual void Release()
// 	{
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
// 	}

	virtual QStringList Serilize()const { throw "Not Implement!"; }//TODO:�־û�������ڵ���Ϣ�ͽṹ��*�Ƿ񱣴����ݣ���
	virtual void Deserialize(QStringList) { throw "Not Implement!"; }//TODO:�ӳ־û���Ϣ�лָ�
	virtual QString GetGuiAdvice()const { throw "Not Implement!"; }//TODO:�Թ����������GUI���飬���ܲ������������еķ�ʽ

	QVariant data;//����
	QVariant defaultData;//Ĭ��ֵ
	QHash<QString, QVariant> additionInfo;//������Ϣ

	QString description;//����
	QList<std::function<bool(const QVariant&)>> assertFunctions;//����У��

	AlgGraphNode& node;//�����Ľڵ�
	QList<QPointer<AlgGraphVertex>> connectedVertexes;//���ӵ��Ķ˿�

	std::atomic_bool isActivated = false;//�����־
	std::atomic_bool isEnabled = true;//ʹ�ܱ�־

	//GuiGraphVertex*gui = nullptr;//���ӵ�ͼ��
signals:
	void sig_ActivateBegin();//���ʼ
	void sig_Activated(QVariant var, bool is_Activated);//�����źţ�������������һ���ڵ�
	void sig_ActivateEnd();//�����������һ������ɹ���

	void sig_ConnectionAdded(AlgGraphVertex*src, AlgGraphVertex*dst);//���ӽ����ɹ�
	void sig_ConnectionRemoved(AlgGraphVertex*src, AlgGraphVertex*dst);//�����Ƴ��ɹ�
	void sig_Reseted(AlgGraphVertex*);//���óɹ�
	void sig_Cleared(AlgGraphVertex*);//��ճɹ�
	void sig_Released(AlgGraphVertex*);//�ͷųɹ�

	void sig_Destroyed(AlgGraphNode*node,AlgGraphVertex*vertex);
protected:
	//�Զ��弤��֣�
	//ͨ����isActΪtrue���洢���ݡ������ǰVertex����Vertex�᡾����sig_Activated()����һ�����
	//false��������ݡ����ҡ����ᷢ�͡�sig_Activated()��һ������
	virtual void _Activate(QVariant var, bool isAct){}
};
class AlgGraphVertex_Input
	:public AlgGraphVertex
{
	Q_OBJECT
public:
	AlgGraphVertex_Input(AlgGraphNode&parent, QString name) :AlgGraphVertex(parent, name) {}
	virtual ~AlgGraphVertex_Input() {  }

	enum class Behavior :unsigned char
	{
		KEEP = 0,//һֱ���ּ���״̬�����ݣ�Ĭ�ϣ�
		DISPOSABLE = 1,//һ���ԣ�����һ�κ��źź�������ʧ//TODO:������Node�ļ����ź�����Activate(0,false)��ʵ��
					   //BUFFER = 2,//���壬������������//TODO:������ڵ��뷨��ʹ��Buffer Nodeʵ��
	}behavior = Behavior::KEEP;
};
class AlgGraphVertex_Output
	:public AlgGraphVertex
{
	Q_OBJECT
public:
	AlgGraphVertex_Output(AlgGraphNode&parent, QString name) :AlgGraphVertex(parent, name) {}
	virtual ~AlgGraphVertex_Output() { }

	virtual void _Activate(QVariant var, bool isNow)//isNow���Ƿ����̷��͡�����ֻ�����ݲ����͡������������ȣ������ض��Զ����
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
//�㷨�ڵ�
//TODO:�����������㷨���������ⲿ���롢�ⲿ�����������������while���߼����㡢��ʱ��ѭ��������
class AlgGraphNode
	:public QObject
{
	Q_OBJECT
public:
	enum class RunMode
	{
		Thread,//���̷߳�ʽ��Ĭ�ϡ�
		Direct,//��ֱ�ӷ�ʽ���������߳�
		Function,//������ʽ��Node�ڲ�Ƕ����һ��Graph��
	};

	AlgGraphNode(QObject*parent, QThreadPool&pool);
	virtual ~AlgGraphNode();

	virtual void Init();//��ʼ��һЩ�����������ʼ����_isUnchange��Ϊtrue�����ܸ����趨
	virtual void Reset();//��������״̬�������������ʱ���������к�ɱ�Ĳ�����
	virtual void Clear() {}
	virtual void Release();
	virtual QString GetGuiAdvice()const { return "normal"; }

	virtual AlgGraphVertex* AddVertex(QString name, QVariant defaultValue, bool isInput);//���Ĭ�Ͻڵ㣬����������ڵ��sig_Activated()�ź�
	virtual QHash<QString,AlgGraphVertex*> AddVertex(QHash<QString, QVariant>initTbl, bool isInput);
	virtual void RemoveVertex(QString name, bool isInput);
	virtual void RemoveVertex(QStringList names, bool isInput){}
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

	void SetName(QString name);
	void AttachGui(GuiGraphNode*gui) { _gui = gui; }
	void DetachGui() { _gui = nullptr; }//TODO:���GuiGraphNode������İ�ȫ��
	bool isHasGui()const { return _gui != nullptr; }
	const QHash<QString, QPointer<AlgGraphVertex>>& GetVertexes(bool isInput)const { return isInput ? _inputVertex : _outputVertex; }
signals:
	void sig_VertexAdded(const AlgGraphVertex*vtx, bool isInput);
	void sig_VertexRemoved(const AlgGraphVertex*vtx, bool isInput);
	void sig_ConnectionAdded();
	void sig_ConnectionRemoved();

	void sig_ActivatedFinished(AlgGraphNode*node);//�ڵ㱻����
	void sig_RunFinished(AlgGraphNode*node);//���н���
	void sig_OutputFinished(AlgGraphNode*node);//�������

	void sig_Destroyed(AlgGraphNode*node);
protected:
	//���ڵ�������_inputVertex��_outputVertex��ģ�庯����func��lambda���ʽ�����������ε���vtxs��Ԫ�أ���������ڻ��������ɾ��
	//ʾ����_ProcessVertex(_inputVertex,[this](AlgGraphVertex*vertex){vertex->Reset();})
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


	virtual QVariantHash _LoadInput();/*�Զ����ȡ���룬Ĭ��ֱ�ӽ����ݴ�Vertex���и���һ�ݣ��������й��������뱻�޸�*/
	virtual QVariantHash _Run(QVariantHash data);//��Ҫ�����в��֣���������һ���߳������С�
	virtual void _LoadOutput(QVariantHash result);/*�Զ�����������Ĭ��ֱ�ӽ����ݴ���ʱ�����м��ص������*/

	QHash<QString, QPointer<AlgGraphVertex>>_inputVertex;//����ڵ�
	QHash<QString, QPointer<AlgGraphVertex>>_outputVertex;//����ڵ�

	QFutureWatcher<QVariantHash> _result;//�������й۲���
	QThreadPool&_pool;//ʹ�õ��̳߳�
	QReadWriteLock _lock;//�������ܲ�����Ҫ����Ϊ��д���������������̡߳�

	RunMode _mode = RunMode::Thread;//���з�ʽ

	std::atomic_bool _isEnable = true;//ʹ��
	std::atomic_bool _isUnchange = false;//�����ڴ������޸�
	std::atomic_bool _pause = false;//��ͣ��־
	std::atomic_bool _stop = false;//������־

	QPointer<GuiGraphNode> _gui = nullptr;	
};

class AlgGraphNode_Input
	:public AlgGraphNode
{
	Q_OBJECT
public:
	AlgGraphNode_Input(QObject*parent, QThreadPool&pool) :AlgGraphNode(parent, pool) { setObjectName("Input"); }
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
	AlgGraphNode_Output(QObject*parent, QThreadPool&pool) :AlgGraphNode(parent, pool) { setObjectName("Output"); }
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
	QHash<AlgGraphVertex*, int>idxTbl;//��Ӧ˳��
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
	VERTEX_TYPE=65536+0x100,
	ARROW_TYPE=65536+0x200,
	NODE_TYPE=65536+0x400,
};
class GuiGraphItemArrow
	:public QGraphicsLineItem
{
public:
	//GuiGraphItemArrow(const QLineF &line, QGraphicsItem*parent) :QGraphicsLineItem(line, parent) {}
	GuiGraphItemArrow(const GuiGraphItemVertex*src, const GuiGraphItemVertex*dst);
	enum { Type = ARROW_TYPE };
	virtual int type()const { return Type; }

	void updatePosition() { throw "Not Implement"; }
	virtual QRectF boundingRect(void) const { return QGraphicsLineItem::boundingRect(); }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) { QGraphicsLineItem::paint(painter, option, widget); }

	QPointer<const AlgGraphVertex>srcVertex, dstVertex;//ʼĩVertex

};
class GuiGraphItemVertex
	:public QGraphicsItem
{
public:
	GuiGraphItemVertex(QGraphicsItem*parent, const AlgGraphVertex&vertex)
		:QGraphicsItem(parent), _vertex(vertex) {
		setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsScenePositionChanges);
		setAcceptHoverEvents(true);
		setFlag(QGraphicsItem::ItemIsSelectable);
		setZValue(parent->zValue() + 0.1);
	}
	virtual ~GuiGraphItemVertex() { qDebug()<<_vertex.objectName() << __FUNCTION__; }
	enum{Type=VERTEX_TYPE};
	virtual int type()const { return Type; }

	void AddArrow(GuiGraphItemArrow*arrow) { assert(arrow != nullptr);  _arrows.append(QSharedPointer<GuiGraphItemArrow>(arrow)); }
	void RemoveArrow(GuiGraphItemArrow*arrow) { _arrows.removeOne(QSharedPointer<GuiGraphItemArrow>(arrow)); }
// 	void RemoveArrow(AlgGraphVertex*another) 
// 	{ 
// 		int i = 0;
// 		while(i<_arrows.size())
// 			if(_arrows[i]->srcVertex==another||_arrows[i])
// 	}

	virtual QPointF ArrowAttachPosition()const//��ͷ���ӵ�λ�� 
	{ return boundingRect().center(); }

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	const AlgGraphVertex&_vertex;

	int _mouseState = 0;
	QList<QSharedPointer<GuiGraphItemArrow>>_arrows;

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
	GuiGraphItemNode(QRectF area, QGraphicsItem*parent,GuiGraphNode&holder) 
		:QGraphicsRectItem(area, parent),_holder(holder),_title(this)
	{
		setTransformOriginPoint(boundingRect().center());
		setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable);
		//setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable);
		setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
	}
	virtual ~GuiGraphItemNode();//����ʱ���Զ�����
	enum { Type = NODE_TYPE };
	virtual int type()const { return Type; }

	virtual void Refresh();
	void SetTitle(QString name) {
		_title.setPlainText(name);	
		_title.setPos(_title.mapToParent(boundingRect().width() / 2 - _title.boundingRect().width() / 2, 0));
	}

	GuiGraphNode&_holder;
	QGraphicsTextItem _title;
	QHash<const AlgGraphVertex*, GuiGraphItemVertex*> _inputVertexItem;//����ڵ�
	QHash<const AlgGraphVertex*, GuiGraphItemVertex*> _outputVertexItem;//����ڵ�
};
//Node��Gui�Ŀ��������������һ����AlgGraphNode��GuiGraphItemNode֮����ת������
class GuiGraphNode
	:public QObject
{
	Q_OBJECT
public:
	GuiGraphNode(const AlgGraphNode&node,GraphScene&scene);
	virtual ~GuiGraphNode();
	virtual GuiGraphItemNode* InitApperance(QPointF center = QPointF(0, 0), QRectF size = QRectF(0, 0, 100, 100));//����AlgGraphNode����Ϣ��ʼ��GuiGraphItemNode

	virtual QWidget* InitWidget(QWidget*parent);

	GuiGraphItemVertex*AddVertex(const AlgGraphVertex*vtx);
	GuiGraphItemArrow*AddConnection();
	void DetachItem() { _nodeItem = nullptr; }

	virtual QVariant GetData() { throw "NotImplement"; }
	virtual void SetData(QVariant var) { throw "NotImplement"; }

	const AlgGraphNode& _node;//����ӦAlgGraphNode�ĳ����ã�ֻ������д
signals:
	void sig_Destroyed(GuiGraphNode*node);//����ǰ�������ź�
	void sig_ValueChanged();//TODO:����Ҫ�·ŵ���Ӧ����������൱��
protected:	
	//QHash<QString, const AlgGraphVertex*>_inputVertex;//����ڵ�
	//QHash<QString, const AlgGraphVertex*>_outputVertex;//����ڵ�
	//QHash<const AlgGraphVertex*, GuiGraphItemVertex*> _inputVertexItem;//����ڵ�
	//QHash<const AlgGraphVertex*, GuiGraphItemVertex*> _outputVertexItem;//����ڵ�

	GraphScene&_scene;
	GuiGraphItemNode* _nodeItem=nullptr;//�ڳ����л��ƺͽ��������壬������ʱ���Զ�����
	QPointer<QWidget> _panel=nullptr;//���ؼ���TODO:����Ҫ�·ŵ���Ӧ����������൱��
};

// class GuiGraphNode_Input
// 	:public GuiGraphNode
// {
// 	Q_OBJECT
// public:
// 	GuiGraphNode_Input(const AlgGraphNode&node):GuiGraphNode(node){}
// 	virtual ~GuiGraphNode_Input() {}
// 	virtual QWidget* InitWidget(QWidget*parent)
// 	{
// 		_panel = new QPlainTextEdit(parent);
// 		connect(qobject_cast<QPlainTextEdit*>(_panel), &QPlainTextEdit::textChanged, this, &GuiGraphNode::sig_ValueChanged);
// 		return _panel;
// 	}
// 	virtual QVariant GetData() { return (qobject_cast<QPlainTextEdit*>(_panel))->toPlainText(); }
// 	//virtual void SetData(QVariant var) { throw "NotImplement"; }
// };
// class GuiGraphNode_Output
// 	:public GuiGraphNode
// {
// 	Q_OBJECT
// public:
// 	GuiGraphNode_Output(const AlgGraphNode&node) :GuiGraphNode(node) {}
// 	virtual ~GuiGraphNode_Output() {}
// 	virtual QWidget* InitWidget(QWidget*parent)
// 	{
// 		_panel = new QLabel(parent);
// 		return _panel;
// 	}
// 	virtual void SetData(QVariant var) { (qobject_cast<QLabel*>(_panel))->setText(var.toString()); }
// };

class GraphScene
	:public QGraphicsScene
{
	Q_OBJECT
public:
	GraphScene(QObject*parent):QGraphicsScene(parent){}
	virtual ~GraphScene() { if (arrow != nullptr)delete arrow; }
signals:
	void sig_ConnectionAdded(GuiGraphItemVertex*src, GuiGraphItemVertex*dst);
	void sig_RemoveItems(QList<QGraphicsItem*>items);
protected:
//	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;//��Ҫ������
//	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;

	QGraphicsLineItem*arrow = nullptr;
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

	AlgGraphNode& AddNode(AlgGraphNode&node, GuiGraphNode*guiNode = nullptr, QPointF center = QPointF(0, 0));//����Ѵ�����node��������Ӧ��guiNode�����guiNodeΪnullptr�������һ��Ĭ�ϵ�
	GuiGraphNode* AddGuiNode(AlgGraphNode&node,GuiGraphNode*guiNode, QPointF center = QPointF(0, 0));//��node�����ʾ���֣����guiNodeΪnullptr�����Ĭ�ϵ�
	
	void AddNodeAsAdvice(QString advice);//TODO:�������������
	
	void RemoveNode(AlgGraphNode*node);
	void RemoveVertex(AlgGraphVertex*vertex);
	void RemoveConnection(AlgGraphVertex*src, AlgGraphVertex*dst);
	void AddConnection(AlgGraphVertex*srcVertex, AlgGraphVertex*dstVertex);
	void AddConnection(GuiGraphItemVertex*srcVertex, GuiGraphItemVertex*dstVertex);

	void slot_Start(bool b);

private:
	void slot_RemoveItems(QList<QGraphicsItem*>items);

	Ui::TestAnythingClass ui;
	GraphScene _scene;
	QList<AlgGraphNode*>_nodes;

	QGraphicsItem*selectedItem = nullptr;

	QThreadPool _pool;
};

