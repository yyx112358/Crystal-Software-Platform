#include "TestAnything.h"
#include <QPlainTextEdit>
#include <QGraphicsSceneMouseEvent>

#include <QDebug>
#include <opencv.hpp>
#include <functional>
#include <vld.h>
using namespace cv;

TestAnything::TestAnything(QWidget *parent)
	: QMainWindow(parent),_pool(this),_scene(this)
{
	ui.setupUi(this);
	ui.graphicsView->setScene(&_scene);
	connect(ui.pushButton_NodeInput, &QPushButton::clicked, this, [this](bool b)
	{
		auto node = new AlgGraphNode_Input(this, _pool);
		node->Init();
		_nodes.append(node);
		AddGuiNode(node, QPointF(0, 0));
	});
	connect(ui.pushButton_NodeOutput, &QPushButton::clicked, this, [this](bool b)
	{
		auto node = new AlgGraphNode_Output(this, _pool);
		node->Init();
		_nodes.append(node);
		AddGuiNode(node, QPointF(0, 0));
	});
	connect(ui.actionStart, &QAction::triggered, this, &TestAnything::slot_Start);
	void (TestAnything::*pAddConnection)(GuiGraphItemVertex*, GuiGraphItemVertex*) = &TestAnything::AddConnection;//注意这里要这样写来区分重载函数
	connect(&_scene, &GraphScene::sig_ConnectionAdded, this, pAddConnection);

	sizeof(AlgGraphVertex); sizeof(AlgGraphVertex_Input); sizeof(GuiGraphNode);sizeof(AlgGraphNode);

	auto nodein = new AlgGraphNode_Input(this, _pool);
	nodein->Init();
	_nodes.append(nodein);
	AddGuiNode(nodein, QPointF(0, 0));

	auto nodeout = new AlgGraphNode_Output(this, _pool);
	nodeout->Init();
	_nodes.append(nodeout);
	AddGuiNode(nodeout, QPointF(200, 0));

	//AddConnection(nodein->GetVertexes(false).value("out"), nodeout->GetVertexes(true).value("in"));
}

void TestAnything::AddGuiNode(AlgGraphNode*node, QPointF center)
{
	//TODO:后期改为：首先根据AlgGraphNode的GetGuiAdvice()，在工厂类/函数中创建相应GUI，如果没有返回或返回失败，则根据类型（Input/Output等大类）生成默认GUI
	auto guiNode = new GuiGraphNode(*node);
	node->AttachGui(guiNode);
	auto guiItem=guiNode->InitApperance();
	guiItem->setPos(center);
	guiNode->AttachScene(&_scene);
	if(qobject_cast<AlgGraphNode_Input*>(node)!=nullptr)
		ui.groupBox_Input->layout()->addWidget(guiNode->InitWidget(ui.groupBox_Input));
	else if (qobject_cast<AlgGraphNode_Output*>(node) != nullptr)
		ui.groupBox_Output->layout()->addWidget(guiNode->InitWidget(ui.groupBox_Output));
}

void TestAnything::AddConnection(AlgGraphVertex*srcVertex, AlgGraphVertex*dstVertex)
{
	if (srcVertex == nullptr || dstVertex == nullptr || srcVertex->gui == nullptr || dstVertex->gui == nullptr)
		return;
	srcVertex->connectedVertexes.append(dstVertex);
	dstVertex->connectedVertexes.append(srcVertex);
	auto srcItem = srcVertex->gui, dstItem = dstVertex->gui;
	auto arrow = new GuiGraphItemArrow(srcItem, dstItem);
	srcItem->AddArrow(arrow);
	dstItem->AddArrow(arrow);
	_scene.addItem(arrow);
}

void TestAnything::AddConnection(GuiGraphItemVertex*srcVertex, GuiGraphItemVertex*dstVertex)
{
	if (srcVertex == nullptr || dstVertex == nullptr)
		return;
	qDebug() << srcVertex->_vertex.objectName() << dstVertex->_vertex.objectName();
	AddConnection(const_cast<AlgGraphVertex*>(&srcVertex->_vertex), const_cast<AlgGraphVertex*>(&dstVertex->_vertex));
}

void TestAnything::slot_Start(bool b)
{
	qDebug() << __FUNCSIG__;

}

#pragma region AlgGraphNode
void AlgGraphNode::Init()
{

}

void AlgGraphNode::Reset()
{

}

void AlgGraphNode::Release()
{

}

void AlgGraphNode::AddVertex(QString name, QVariant defaultValue, bool isInput)
{

}

void AlgGraphNode::AddVertex(QHash<QString, QVariant>initTbl, bool isInput)
{

}

void AlgGraphNode::RemoveVertex(QString name)
{

}

void AlgGraphNode::RemoveVertex(QStringList names)
{

}

void AlgGraphNode::ConnectVertex(QString vertexName, AlgGraphNode&dstNode, QString dstVertexName)
{

}

void AlgGraphNode::DisconnectVertex(QString vertexName)
{

}

void AlgGraphNode::DisconnectVertex(QString vertexName, AlgGraphNode&dstNode, QString dstVertexName)
{

}

void AlgGraphNode::Write()
{

}

void AlgGraphNode::Read()
{

}

void AlgGraphNode::Activate()
{

}

void AlgGraphNode::Run(/*QMap<QString, QVariant>*/)
{

}

void AlgGraphNode::Output()
{

}

void AlgGraphNode::Pause()
{

}

void AlgGraphNode::Stop()
{

}

#pragma endregion AlgGraphNode
#if 0
TestAnything::TestAnything(QWidget *parent)
	: QMainWindow(parent), pool(this)
{
	ui.setupUi(this);

	connect(ui.actionStart, &QAction::triggered, this, &TestAnything::slot_Start);

	auto scene = new GraphScene(this);
	ui.graphicsView->setScene(&scene->scene);

	nodes["in1"] = new AlgGraphNode(this, pool);
	nodes["in1"]->setObjectName("in1");
	nodes["in1"]->AddVertex("in1", "in1", true);
	nodes["in1"]->AddVertex("out1", "out1", false);
	scene->AddNode(nodes["in1"], 0, 0, 50, 50);

	nodes["in2"] = new AlgGraphNode(this, pool);
	nodes["in2"]->setObjectName("in2");
	nodes["in2"]->AddVertex("in1", "in1", true);
	nodes["in2"]->AddVertex("out1", "out1", false);
	scene->AddNode(nodes["in2"], 0, 100, 50, 50);

	nodes["add1"] = new AlgGraphNode(this, pool);
	nodes["add1"]->setObjectName("add1");
	nodes["add1"]->AddVertex("in1", "in1", true);
	nodes["add1"]->AddVertex("in2", "in2", true);
	nodes["add1"]->AddVertex("out1", "out1", false);
	scene->AddNode(nodes["add1"], 100, 50, 50, 50);

	nodes["out1"] = new AlgGraphNode(this, pool);
	nodes["out1"]->setObjectName("out1");
	nodes["out1"]->AddVertex("in1", "in1", true);
	nodes["out1"]->AddVertex("out1", "out1", false);
	scene->AddNode(nodes["out1"], 200, 50, 50, 50);

	nodes["out2"] = new AlgGraphNode(this, pool);
	nodes["out2"]->setObjectName("out2");
	nodes["out2"]->AddVertex("in1", "in1", true);
	nodes["out2"]->AddVertex("out1", "out1", false);
	scene->AddNode(nodes["out2"], 200, 0, 50, 50);

	ConnectVertex(*nodes["in1"], "out1", *nodes["add1"], "in1");
	ConnectVertex(*nodes["in2"], "out1", *nodes["add1"], "in2");
	ConnectVertex(*nodes["add1"], "out1", *nodes["out1"], "in1");
	ConnectVertex(*nodes["in1"], "out1", *nodes["out2"], "in1");

	connect(ui.plainTextEdit, &QPlainTextEdit::textChanged, this, [this]
	{
		nodes["in1"]->Activate(ui.plainTextEdit->toPlainText(), "in1");
	});
	connect(ui.plainTextEdit_2, &QPlainTextEdit::textChanged, this, [this]
	{
		nodes["in2"]->Activate(ui.plainTextEdit_2->toPlainText(), "in1");
	});
	connect(nodes["out1"], &AlgGraphNode::sig_Output, this, [this](QVariant var)
	{
		ui.label->setText(var.toString());
	});
	connect(nodes["out2"], &AlgGraphNode::sig_Output, this, [this](QVariant var)
	{
		ui.label_2->setText(var.toString());
	});
}


void TestAnything::slot_Start(bool b)
{
	qDebug() << __FUNCSIG__;
	// 	ui.plainTextEdit->setPlainText(ui.plainTextEdit->toPlainText()+__FUNCSIG__"\n");
	// 	//QtConcurrent::run()
	// 
	ui.label->setText("");
	ui.label_2->setText("");
	for (auto node : nodes)
	{
		node->Reset();
		node->Activate();
	}
}

AlgGraphNode::AlgGraphNode(QObject*parent, QThreadPool&pool) :QObject(parent), _pool(pool)
{
	sizeof(VertexInfo);
	connect(&_result, &QFutureWatcher<void>::finished, this, &AlgGraphNode::Output);
	connect(&_result, &QFutureWatcher<void>::finished, this, [this] {emit sig_TaskFinished(this); });
}

AlgGraphNode::~AlgGraphNode()
{
	qDebug() << __FUNCSIG__;
}

void AlgGraphNode::AddVertex(QString name, QVariant defaultValue, bool isInput)
{
	assert(name.isEmpty() == false);
	VertexInfo vtx;
	vtx.defaultValue = defaultValue;
	auto &group = (isInput) ? (_inputVertex) : (_outputVertex);
	group.insert(name, vtx);
	emit sig_VertexAdded(&group[name], isInput);
}

void AlgGraphNode::Reset()
{
	for (auto &v : _inputVertex)
	{
		v.isActivated = false;
		v.isEnabled = true;
		v.param.clear();
	}
	for (auto &v : _outputVertex)
	{
		v.isActivated = false;
		v.isEnabled = true;
		v.param.clear();
	}
}

void AlgGraphNode::Release()
{
	_inputVertex.clear();
	_outputVertex.clear();
}

void AlgGraphNode::Activate(QVariant var, VertexInfo*vtx / *= nullptr* / , bool b / *= true * / )
{
	//TODO:需要加锁
	if (vtx != nullptr)
	{
		if (vtx->isEnabled == false)//禁用顶点，阻塞
			return;
		//TODO:类型判断
		vtx->param = var;
		vtx->isActivated = b;
		emit sig_VertexActivated(b);
	}
	if (_result.isRunning() == false)//运行期间，阻塞输入
	{
		for (auto const &v : _inputVertex)//检查是否全部激活
		{
			if (v.isActivated == false)
				return;
		}
		//TODO:读取输入
		emit sig_NodeActivated(this);
		//TODO:暂停和退出
		_result.setFuture(QtConcurrent::run(&_pool, this, &AlgGraphNode::Run));
		//TODO:暂停和退出
		//emit sig_ResultReady();
	}
}

void AlgGraphNode::Activate(QVariant var / *= QVariant()* / , QString vtxName / *= QString()* / , bool b / *= true * / )
{
	if (vtxName.isEmpty() == false)
		Activate(var, &_inputVertex[vtxName], b);
	else
		Activate(var, nullptr, b);
}

void AlgGraphNode::Run(/ *QMap<QString, QVariant>* / )
{
	_Run();
}

void AlgGraphNode::Output()
{
	//TODO:加锁
	//qDebug() << "====Output====:" << QThread::currentThread();
	for (auto &v : _outputVertex)
	{
		for (auto cv : v.connectedVertexs)
			emit sig_Activate(v.param, cv, true);
		emit sig_Output(v.param, &v);
		v.param.clear();
	}
	emit sig_OutputFinished();
}

void AlgGraphNode::_Run()
{
	qDebug() << "====Start====:" << QThread::currentThread() << QTime::currentTime();
	for (auto &v : _inputVertex)
		qDebug() << "<in>" << objectName() << ':' << v.param;
	QThread::msleep(500);

	QVariant test = __FUNCSIG__;
	_outputVertex["out1"].param = "";
	for (auto const&v : _inputVertex)
		_outputVertex["out1"].param = _outputVertex["out1"].param.toString() + v.param.toString();

	for (auto &v : _outputVertex)
		qDebug() << "<out>" << objectName() << ':' << v.param;
	qDebug() << "End:" << QThread::currentThread() << QTime::currentTime();
}
#endif
#pragma region AlgGraphVertex
void AlgGraphVertex::Activate(QVariant var, bool b /*= true*/)
{

}

void AlgGraphVertex::Connect(AlgGraphVertex*another)
{

}

void AlgGraphVertex::Write()
{

}

void AlgGraphVertex::Read()
{

}
#pragma endregion AlgGraphVertex

GuiGraphNode::GuiGraphNode(const AlgGraphNode&node) :QObject(const_cast<AlgGraphNode*>(&node)), _node(node),
_nodeItem(new GuiGraphItemNode(QRectF(0, 0, 100, 100), nullptr))
{
	setObjectName(node.objectName());
}
//初始化外观
QGraphicsItem* GuiGraphNode::InitApperance()
{
	auto box = _nodeItem->boundingRect();
	QGraphicsTextItem*title = new QGraphicsTextItem(_nodeItem);
	title->setPlainText(objectName());
	title->setPos(title->mapFromParent(box.width() / 2 - title->boundingRect().width() / 2, 0));
	
	int h = title->boundingRect().height();
	for (auto vtx : _node.GetVertexes(true))
	{
		QString name = vtx->objectName();
		GuiGraphItemVertex*pnode = new GuiGraphItemVertex(_nodeItem, *vtx);
		pnode->setPos(pnode->mapFromParent(0, h));
		h += pnode->boundingRect().height();
		pnode->setAcceptHoverEvents(true);
		pnode->setZValue(_nodeItem->zValue() + 0.1);

		vtx->gui = pnode;
		_inputVertex.insert(name, vtx);
		_inputVertexItem.insert(name, pnode);
	}
	for (auto vtx : _node.GetVertexes(false))
	{
		QString name = vtx->objectName();
		GuiGraphItemVertex*pnode = new GuiGraphItemVertex(_nodeItem, *vtx);
		pnode->setPos(pnode->mapFromParent(box.width() - pnode->boundingRect().width(), h));
		h += pnode->boundingRect().height();
		pnode->setAcceptHoverEvents(true);
		pnode->setZValue(_nodeItem->zValue() + 0.1);

		vtx->gui = pnode;
		_inputVertex.insert(name, vtx);
		_inputVertexItem.insert(name, pnode);
	}
	return _nodeItem;
}

QWidget* GuiGraphNode::InitWidget(QWidget*parent)
{
	_panel = new QPlainTextEdit(parent);
	connect(qobject_cast<QPlainTextEdit*>(_panel), &QPlainTextEdit::textChanged, this, &GuiGraphNode::sig_ValueChanged);
	return _panel;
}

void AlgGraphNode_Input::Init()
{
	_outputVertex.insert("out", new AlgGraphVertex_Output(*this, "out"));
}

void AlgGraphNode_Input::Input(QVariant var)
{

}

void AlgGraphNode_Output::Init()
{
	_inputVertex.insert("in", new AlgGraphVertex_Input(*this, "in"));
}

QRectF GuiGraphItemVertex::boundingRect() const
{
	QFontMetrics fm(scene() != nullptr ? (scene()->font()) : (QApplication::font()));
	return QRectF(0, 0, fm.width(_vertex.objectName()), fm.height());
}

void GuiGraphItemVertex::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	if (_mouseState == 1)
		painter->drawRect(boundingRect());
	painter->drawText(0, boundingRect().height(), _vertex.objectName()/*,QTextOption(Qt::AlignmentFlag::AlignCenter)*/);
}


void GraphScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (arrow == nullptr) 
	{	
		auto item = itemAt(event->scenePos(),QTransform());	
		if (item != nullptr && item->type() == GuiGraphItemVertex::Type/*qgraphicsitem_cast<GuiGraphItemVertex*>(item) == nullptr*/)
		{
			auto pos = item->mapToScene(item->boundingRect().center());
			arrow = new GuiGraphItemArrow(QLineF(pos, pos), nullptr);
			addItem(arrow);
		}
	}
	else
	{
		auto srcPos = arrow->line().p1(),dstPos=arrow->line().p2();
		removeItem(arrow);
		delete arrow;
		arrow = nullptr;

		auto dstItem = itemAt(dstPos, QTransform()),srcItem= itemAt(srcPos, QTransform());//必须先删掉arrow，否则获取的是arrow
		if (dstItem != nullptr && dstItem->type() == GuiGraphItemVertex::Type)
		{
			auto srcItem = itemAt(srcPos, QTransform());
			if(srcItem!=nullptr && srcItem->type() == GuiGraphItemVertex::Type)
				emit sig_ConnectionAdded(qgraphicsitem_cast<GuiGraphItemVertex*>(srcItem)
					, qgraphicsitem_cast<GuiGraphItemVertex*>(dstItem));
		}
	}

	QGraphicsScene::mousePressEvent(event);
}
void GraphScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (arrow != nullptr) 
	{
		auto item = itemAt(event->scenePos(), QTransform());
		if (item!=nullptr && item->type() == GuiGraphItemVertex::Type/*qgraphicsitem_cast<GuiGraphItemVertex*>(item) != nullptr*/)
		{
			auto pos = item->mapToScene(item->boundingRect().center());
// 			qDebug()<<event->screenPos()<< item->mapFromScene(QPointF(0,0))<< item->mapFromParent(QPointF(0, 0))
// 				<<item->mapFromScene()
			arrow->setLine(QLineF(arrow->line().p1(), pos));
		}
		else
			arrow->setLine(QLineF(arrow->line().p1(), event->scenePos()));
	}
	QGraphicsScene::mouseMoveEvent(event);
}


void GuiGraphItemArrow::updatePosition()
{
	QPointF p1 = (srcVertex != nullptr) ? (srcVertex->mapToScene(srcVertex->ArrowAttachPosition())) : line().p1(),
		p2 = (dstVertex != nullptr) ? (dstVertex->mapToScene(dstVertex->ArrowAttachPosition())) : line().p1();
	setLine(QLineF(p1, p2));
}
