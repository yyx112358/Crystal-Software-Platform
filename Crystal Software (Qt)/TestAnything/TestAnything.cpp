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
		auto guiNode = new GuiGraphNode_Input(*node);
		AddNode(*node, guiNode, QPointF(-100, 0));
		connect(guiNode, &GuiGraphNode::sig_ValueChanged, node, &AlgGraphNode_Input::Activate);
	});
	connect(ui.pushButton_NodeOutput, &QPushButton::clicked, this, [this](bool b)
	{
		auto node = new AlgGraphNode_Output(this, _pool);
		AddNode(*node, new GuiGraphNode_Output(*node), QPointF(100, 0));	});
	connect(ui.pushButton_NodeAdd, &QPushButton::clicked, this, [this](bool b)
	{
		auto node = new AlgGraphNode_Add(this, _pool);
		AddNode(*node, nullptr, QPointF(0, 0));
	});

	connect(ui.actionStart, &QAction::triggered, this, &TestAnything::slot_Start);
	void (TestAnything::*pAddConnection)(GuiGraphItemVertex*, GuiGraphItemVertex*) = &TestAnything::AddConnection;//注意这里要这样写来区分重载函数
	connect(&_scene, &GraphScene::sig_ConnectionAdded, this, pAddConnection);
	connect(&_scene, &GraphScene::sig_RemoveItems, this, [this](QList<QGraphicsItem*>items)
	{
		for (auto item : items)
		{
			switch (item->type())
			{
			case GuiGraphItemNode::Type:
				RemoveNode(const_cast<AlgGraphNode*> (&((qgraphicsitem_cast<GuiGraphItemNode*>(item))->_holder._node)));
			default:
				break;
			}
		}
	});

	sizeof(AlgGraphVertex); sizeof(AlgGraphVertex_Input); sizeof(GuiGraphNode);sizeof(AlgGraphNode);
}

AlgGraphNode& TestAnything::AddNode(AlgGraphNode&node, GuiGraphNode*guiNode /*= nullptr*/, QPointF center /*= QPointF(0, 0)*/)
{
	node.Init();
	_nodes.append(&node);
	//TODO:删除节点及其附属
	connect(&node, &AlgGraphNode::destroyed, this, [this] 
	{
		qDebug() << sender()->objectName() << __FUNCTION__;
		if (_nodes.size() > 0)
			_nodes.removeAll(qobject_cast<AlgGraphNode*>(sender()));
	});
	AddGuiNode(node, guiNode, center);
	return node;
}

GuiGraphNode* TestAnything::AddGuiNode(AlgGraphNode&node, GuiGraphNode*guiNode /*= nullptr*/, QPointF center /*= QPointF(0, 0)*/)
{
	//TODO:后期改为：首先根据AlgGraphNode的GetGuiAdvice()，在工厂类/函数中创建相应GUI，如果没有返回或返回失败，则根据类型（Input/Output等大类）生成默认GUI
	if (guiNode == nullptr)
		guiNode = new GuiGraphNode(node);
	node.AttachGui(guiNode);
	//添加item
	auto guiItem=guiNode->InitApperance();
	guiItem->setPos(center);
	guiNode->AttachScene(&_scene);
	//如果是输入输出类型，则添加控件
	if(qobject_cast<AlgGraphNode_Input*>(&node)!=nullptr)
		ui.groupBox_Input->layout()->addWidget(guiNode->InitWidget(ui.groupBox_Input));
	else if (qobject_cast<AlgGraphNode_Output*>(&node) != nullptr)
		ui.groupBox_Output->layout()->addWidget(guiNode->InitWidget(ui.groupBox_Output));
	
	return guiNode;
}

void TestAnything::RemoveNode(AlgGraphNode*node)
{
	assert(_nodes.contains(node));
	//_scene.removeItem(node->_gui->_nodeItem);
	_nodes.removeAll(node);
	//node->deleteLater();
	delete node;
}

void TestAnything::AddConnection(AlgGraphVertex*srcVertex, AlgGraphVertex*dstVertex)
{
	assert(srcVertex != dstVertex //不允许指向同一个
		&& srcVertex->connectedVertexes.contains(dstVertex) == false//不允许重复的连接
		&& dstVertex->connectedVertexes.contains(srcVertex) == false);
	if (srcVertex == nullptr || dstVertex == nullptr || srcVertex->gui == nullptr || dstVertex->gui == nullptr
		|| srcVertex == dstVertex //不允许指向同一个
		|| srcVertex->connectedVertexes.contains(dstVertex) == true//不允许重复的连接
		|| dstVertex->connectedVertexes.contains(srcVertex) == true)
		return;
	srcVertex->Connect(dstVertex);
	connect(srcVertex, &AlgGraphVertex::sig_Activated, dstVertex, &AlgGraphVertex::Activate);

	auto srcItem = srcVertex->gui, dstItem = dstVertex->gui;
	auto arrow = new GuiGraphItemArrow(srcItem, dstItem);
	arrow->setZValue(srcItem->zValue() - 0.2);
	srcItem->AddArrow(arrow);
	dstItem->AddArrow(arrow);
	_scene.addItem(arrow);	
}

void TestAnything::AddConnection(GuiGraphItemVertex*srcVertex, GuiGraphItemVertex*dstVertex)
{
	if (srcVertex == nullptr || dstVertex == nullptr)
		return;
	qDebug() << __FUNCTION__ << srcVertex->_vertex.objectName() << dstVertex->_vertex.objectName();
	AddConnection(const_cast<AlgGraphVertex*>(&srcVertex->_vertex), const_cast<AlgGraphVertex*>(&dstVertex->_vertex));
}

void TestAnything::slot_Start(bool b)
{
	qDebug() << __FUNCSIG__;
	for (auto n : _nodes)
		n->Reset();
	for (auto n : _nodes)
		n->Activate();
}

void TestAnything::slot_RemoveItems(QList<QGraphicsItem*>items)
{

}

#pragma region AlgGraphNode

AlgGraphNode::AlgGraphNode(QObject*parent, QThreadPool&pool) :QObject(parent), _pool(pool)
{
	connect(&_result, &QFutureWatcher<QVariantHash>::finished, this, [this] {emit sig_RunFinished(this); });
	connect(&_result, &QFutureWatcher<QVariantHash>::finished, this, &AlgGraphNode::Output);
}

void AlgGraphNode::Init()
{
	AddVertex("in", "in", true);
	AddVertex("out", "out", false);
}
void AlgGraphNode_Input::Init()
{
	AddVertex("out", "out", false);
}
void AlgGraphNode_Output::Init()
{
	AddVertex("in", "in", true);
}
void AlgGraphNode_Add::Init()
{
	AddVertex("in1", "in", true);
	AddVertex("in2", "in", true);
	AddVertex("out", "out", false);
}

QVariantHash AlgGraphNode_Input::_Run(QVariantHash data)
{
	assert(_gui.isNull() == false);
	QVariantHash result;
	result["out"] = _gui->GetData();
	return result;
}
QVariantHash AlgGraphNode_Output::_Run(QVariantHash data)
{
	assert(_gui.isNull() == false);
	for (auto d : data)
		_gui->SetData(d);
	return data;
}
QVariantHash AlgGraphNode_Add::_Run(QVariantHash data)
{
	QVariantHash result;
	QString s;
	for (auto d : data)
		s += d.toString();
	result["out"] = s;
	return result;
}

void AlgGraphNode::Reset()
{
	for (auto it = _inputVertex.begin(); it != _inputVertex.end();)
	{
		if (it->isNull() == false)
		{
			(*it)->Reset();
			++it;
		}
		else
		{
			qDebug() << __FUNCTION__ << it.key() << "Not exist";
			it = _inputVertex.erase(it);
		}
	}
	for (auto it = _outputVertex.begin(); it != _outputVertex.end();)
	{
		if (it->isNull() == false)
		{
			(*it)->Reset();
			++it;
		}
		else
		{
			qDebug() << __FUNCTION__ << it.key() << "Not exist";
			it = _outputVertex.erase(it);
		}
	}
	_pause = false;
	_stop = false;
	
	//_lock.unlock();//TODO:考虑加锁
}
void AlgGraphNode::Release()
{
	Reset();
	for (auto v : _inputVertex)
		v->Release();
	for (auto v : _outputVertex)
		v->Release();
	_isEnable = true;
}
AlgGraphVertex* AlgGraphNode::AddVertex(QString name, QVariant defaultValue, bool isInput)
{
	AlgGraphVertex* pv;
	assert(_inputVertex.contains(name) == false && _outputVertex.contains(name) == false);
	QString newName = name;//TODO:需要判定重名，或者自动添加尾注（例如in_1,in_2）
	if (isInput == true)
	{
		pv = new AlgGraphVertex_Input(*this, newName);
		pv->defaultData = defaultValue;
		_inputVertex.insert(newName, pv);
		connect(pv, &AlgGraphVertex::sig_Activated, this, &AlgGraphNode::Activate);
	}
	else
	{
		pv = new AlgGraphVertex_Output(*this, newName);
		pv->defaultData = defaultValue;
		_outputVertex.insert(newName, pv);
	}
	return pv;
}

QHash<QString, AlgGraphVertex*> AlgGraphNode::AddVertex(QHash<QString, QVariant>initTbl, bool isInput)
{
	QHash<QString, AlgGraphVertex*> result;
	for (auto it = initTbl.begin(); it != initTbl.end(); ++it)
		result[it.key()] = AddVertex(it.key(), it.value(), isInput);
	return result;
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
	qDebug() << objectName() << __FUNCTION__;
	if (_result.isRunning() == false)//运行期间，阻塞输入
	{
		for (auto it = _inputVertex.begin(); it != _inputVertex.end();)//检查是否全部激活
		{
			if (it->isNull() == false)
			{
				if ((*it)->isActivated == false)
					return;
				++it;
			}
			else
			{
				qDebug() << __FUNCTION__ << it.key() << "Not exist";
				it = _inputVertex.erase(it);
			}
		}
		Run();
	}
}

void AlgGraphNode::Run()
{
	//TODO:加锁
	qDebug() << objectName() << __FUNCTION__;
	auto data = _LoadInput();
	emit sig_ActivatedFinished(this);
	//TODO:暂停和退出
	switch (_mode)
	{
	case AlgGraphNode::RunMode::Thread:
		_result.setFuture(QtConcurrent::run(&_pool, this, &AlgGraphNode::_Run,data));
		break;
	case AlgGraphNode::RunMode::Direct:
		_LoadOutput(_Run(data));
		emit sig_RunFinished(this);
		Output();
		break;
	case AlgGraphNode::RunMode::Function:
		throw "Not Implement";
		break;
	default:
		break;
	}
	//TODO:暂停和退出
	//emit sig_ResultReady();
}

void AlgGraphNode::Output()
{
	//TODO:加锁
	//qDebug() << "====Output====:" << QThread::currentThread();
	if (_mode != RunMode::Direct)
		_LoadOutput((_result.future().resultCount() > 0) ? (_result.result()) : (QVariantHash()));
	//_result.setFuture(QFuture<QVariantHash>());
	for (auto it = _outputVertex.begin(); it != _outputVertex.end();)
	{
		if (it->isNull() == false)
		{
			(*it)->Activate((*it)->data, true);
			++it;
		}
		else
		{
			qDebug() << __FUNCTION__ << it.key() << "Not exist";
			it = _outputVertex.erase(it);
		}
	}
	emit sig_OutputFinished(this);
}

void AlgGraphNode::Pause(bool isPause)
{

}

void AlgGraphNode::Stop(bool isStop)
{

}

QVariantHash AlgGraphNode::_LoadInput()
{
	QHash<QString, QVariant>result;
	for (auto it = _inputVertex.begin(); it != _inputVertex.end(); ++it)
		result.insert(it.key(), it.value()->data);
	return result;
}

QVariantHash AlgGraphNode::_Run(QVariantHash data)
{
	return data;
}

void AlgGraphNode::_LoadOutput(QVariantHash result)
{
	for (auto it = result.begin(); it != result.end(); ++it)
	{
		auto vtx = qobject_cast<AlgGraphVertex_Output*>(_outputVertex.value(it.key()));
		if (vtx != nullptr)
			vtx->data = it.value();
		//else GraphWarning();//TODO:警告
	}
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
#pragma region GuiGraphNode
GuiGraphNode::GuiGraphNode(const AlgGraphNode&node) :QObject(const_cast<AlgGraphNode*>(&node)), _node(node),
_nodeItem(new GuiGraphItemNode(QRectF(0, 0, 100, 100), nullptr,*this))
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
		_inputVertexItem.insert(vtx, pnode);
	}
	h = title->boundingRect().height();
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
		_inputVertexItem.insert(vtx, pnode);
	}
	return _nodeItem;
}
#pragma endregion GuiGraphNode




QRectF GuiGraphItemVertex::boundingRect() const
{
	QFontMetrics fm(scene() != nullptr ? (scene()->font()) : (QApplication::font()));
	return QRectF(0, 0, fm.width(_vertex.objectName()), fm.height());
}

void GuiGraphItemVertex::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	painter->drawText(0, boundingRect().height(), _vertex.objectName()/*,QTextOption(Qt::AlignmentFlag::AlignCenter)*/);
	if (_mouseState == 1)
		painter->drawRect(boundingRect());
	else
	{
		painter->setPen(QPen(QColor(200, 200, 200)));
		//painter->setBrush(QBrush(QColor(100, 100, 100), Qt::BrushStyle::SolidPattern));
		painter->drawRect(boundingRect());
	}
}


void GraphScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if(event->button()==Qt::LeftButton)
	{
		if (arrow == nullptr)
		{
			auto item = itemAt(event->scenePos(), QTransform());
			if (/*item != nullptr && item->type() == GuiGraphItemVertex::Type*/qgraphicsitem_cast<GuiGraphItemVertex*>(item) != nullptr)
			{
				auto pos = item->mapToScene(item->boundingRect().center());
				arrow = new QGraphicsLineItem(QLineF(pos, pos), nullptr);
				addItem(arrow);
			}
		}
		else
		{
			auto srcPos = arrow->line().p1(), dstPos = arrow->line().p2();
			removeItem(arrow);
			delete arrow;
			arrow = nullptr;

			auto dstItem = itemAt(dstPos, QTransform()), srcItem = itemAt(srcPos, QTransform());//必须先删掉arrow，否则获取的是arrow
			if (qgraphicsitem_cast<GuiGraphItemVertex*>(dstItem) != nullptr)
			{
				auto srcItem = itemAt(srcPos, QTransform());
				if (srcItem != nullptr && srcItem->type() == GuiGraphItemVertex::Type)
					emit sig_ConnectionAdded(qgraphicsitem_cast<GuiGraphItemVertex*>(srcItem)
						, qgraphicsitem_cast<GuiGraphItemVertex*>(dstItem));
			}
		}
	}

	QGraphicsScene::mousePressEvent(event);
}
void GraphScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (arrow != nullptr) 
	{
		auto item = itemAt(event->scenePos(), QTransform());
		if (qgraphicsitem_cast<GuiGraphItemVertex*>(item) != nullptr)
		{
			auto pos = item->mapToScene(item->boundingRect().center());
			arrow->setLine(QLineF(arrow->line().p1(), pos));
		}
		else
			arrow->setLine(QLineF(arrow->line().p1(), event->scenePos()));
	}
	QGraphicsScene::mouseMoveEvent(event);
}


void GraphScene::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Delete)
	{
		auto selects = this->selectedItems();
		emit sig_RemoveItems(selects);
	}
}

void GuiGraphItemArrow::updatePosition()
{
	QPointF p1 = (srcVertex.isNull()==false) ? (srcVertex->mapToScene(srcVertex->ArrowAttachPosition())) : line().p1(),
		p2 = (dstVertex.isNull()==false) ? (dstVertex->mapToScene(dstVertex->ArrowAttachPosition())) : line().p1();
	setLine(QLineF(p1, p2));
}

void GraphView::wheelEvent(QWheelEvent *event)
{
	QPoint scrollAmount = event->angleDelta();
	float factor = (scrollAmount.y() > 0) ? (1.259921049894873164) : (1 / 1.259921049894873164);
	scale(factor, factor);
}

