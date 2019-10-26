#include "TestAnything.h"
#include <QPlainTextEdit>
#include <QGraphicsSceneMouseEvent>

#include <qDebug>
#include <opencv.hpp>
#include <functional>
#include <vld.h>
using namespace cv;

#pragma region TestAnything
TestAnything::TestAnything(QWidget *parent)
	: QMainWindow(parent), _pool(this), _scene(this)
{
	ui.setupUi(this);
	ui.graphicsView->setScene(&_scene);
	connect(ui.pushButton_NodeInput, &QPushButton::clicked, this, [this](bool b)
	{
		auto node = new AlgGraphNode_Input(this, _pool);
		auto gui = new GuiGraphController_Input(*node, _scene);
		AddNode(*node, gui, QPointF(-100, 0));
		ui.groupBox_Input->layout()->addWidget(gui->InitWidget(ui.groupBox_Input));
	});
	connect(ui.pushButton_NodeOutput, &QPushButton::clicked, this, [this](bool b)
	{
		auto node = new AlgGraphNode_Output(this, _pool);
		auto gui = new GuiGraphController_Output(*node, _scene);
		AddNode(*node, gui, QPointF(100, 0));
		ui.groupBox_Output->layout()->addWidget(gui->InitWidget(ui.groupBox_Output));
	});
	connect(ui.pushButton_NodeAdd, &QPushButton::clicked, this, [this](bool b)
	{
		auto node = new AlgGraphNode_Add(this, _pool);
		auto gui = new GuiGraphController(*node, _scene);
		AddNode(*node, gui, QPointF(0, 0));
	});
	connect(ui.pushButton_5, &QPushButton::clicked, this, [this](bool b)
	{
		//_scene.clear();
	});
	connect(ui.pushButton_7, &QPushButton::clicked, this, [this](bool b)
	{
		if (_nodes.size() > 0)
		{
			auto node = _nodes[0];
			auto vtxs = node->GetVertexes(true);
			if (vtxs.size() > 0)
				delete vtxs[vtxs.keys()[0]];
		}
	});

	connect(ui.actionStart, &QAction::triggered, this, &TestAnything::slot_Start);
	bool (TestAnything::*pAddConnection)(GuiGraphItemVertex*, GuiGraphItemVertex*) = &TestAnything::AddConnection;//注意这里要这样写来区分重载函数
	connect(&_scene, &GraphScene::sig_ConnectionAdded, this, pAddConnection);
	connect(&_scene, &GraphScene::sig_RemoveItems, this, &TestAnything::slot_RemoveItems);
	_monitorTimerId = startTimer(100, Qt::TimerType::CoarseTimer);
}

TestAnything::~TestAnything()
{
	auto tmp = _nodes;//【改成临时变量，避免删除node时候，node自动解离_nodes从而改变_nodes】
	for (auto n : tmp)
		delete n;
	_nodes.clear();
}

AlgGraphNode& TestAnything::AddNode(AlgGraphNode&node, GuiGraphController*guiNode /*= nullptr*/, QPointF center /*= QPointF(0, 0)*/)
{
	//AlgGraphNode初始化
	assert(_nodes.contains(&node) == false);//避免重复加入
	node.Init();
	//Gui初始化
	if (guiNode != nullptr)
		AddGuiNode(node, guiNode, center);
	//添加到控制器监视中
	_nodes.append(&node);
	connect(&node, &AlgGraphNode::sig_Destroyed, this, [this](AlgGraphNode*node)//删除后自动从_nodes中删除
	{
		_nodes.removeOne(node);
	}, Qt::DirectConnection);//注意不能连接QObject::destroyed信号，因为发出该信号时候派生类已经被析构了
	//无名称则自动命名
	if (node.objectName().isEmpty() == true)
		node.setObjectName(node.metaObject()->className() + QString::number(_nodes.size()));

	return node;
}

GuiGraphController* TestAnything::AddGuiNode(AlgGraphNode&node, GuiGraphController*guiNode, QPointF center /*= QPointF(0, 0)*/)
{
	node.AttachGui(guiNode);
	connect(guiNode, &GuiGraphController::sig_Destroyed, &node, &AlgGraphNode::DetachGui, Qt::DirectConnection);//GUI自动解离
	auto guiItem = guiNode->InitApperance(center);

	return guiNode;
}

bool TestAnything::RemoveNode(AlgGraphNode*node)
{
	_nodes.removeOne(node);
	delete node;//TODO:加入删除失败的异常处理
	return true;
}

bool TestAnything::AddConnection(GuiGraphItemVertex*srcVertex, GuiGraphItemVertex*dstVertex)
{
	return AddConnection(const_cast<AlgGraphVertex*>(&srcVertex->_vertex),
		const_cast<AlgGraphVertex*>(&dstVertex->_vertex));
}

bool TestAnything::AddConnection(AlgGraphVertex*srcVertex, AlgGraphVertex*dstVertex)
{
	bool b = srcVertex != dstVertex //不允许指向同一个
		&& srcVertex->connectedVertexes.contains(dstVertex) == false//不允许重复的连接
		&& dstVertex->connectedVertexes.contains(srcVertex) == false;
	assert(b);	if (!b)	return false;

	qDebug() << srcVertex->node.objectName()+':'+ srcVertex->objectName() 
		<< dstVertex->node.objectName() + ':' + dstVertex->objectName() << __FUNCTION__;
	srcVertex->Connect(dstVertex);
	connect(srcVertex, &AlgGraphVertex::sig_Activated, dstVertex, &AlgGraphVertex::Activate);
	connect(srcVertex, &AlgGraphVertex::sig_ConnectionRemoved, this, [this](AlgGraphVertex*src, AlgGraphVertex*dst)
	{	
		if (src->gui != nullptr&&dst->gui != nullptr)
		{
			for (auto a : src->gui->_arrows)
			{
				if (a->dstItemVertex == dst->gui)
				{
					delete a;
					return;
				}
			}
			throw "Can't Find";//TODO:改成相应Error
		}
	}, Qt::UniqueConnection);//【注意这里务必加入UniqueConnection避免重复发送】
	auto srcItem = srcVertex->gui, dstItem = dstVertex->gui;
	b = srcItem != nullptr&&dstItem != nullptr;
	assert(b);	if (!b)	return true;//注意这里仍然返回true因为连接已成功，只是后面可以添加一个warning来提示
	auto arrow = new GuiGraphItemArrow(srcItem, dstItem);
	arrow->setZValue(srcItem->zValue() - 0.2);
	srcItem->_arrows.append(arrow);
	dstItem->_arrows.append(arrow);
	_scene.addItem(arrow);
	
	return true;
}

void TestAnything::RemoveConnection(AlgGraphVertex*src, AlgGraphVertex*dst)
{
	src->Disconnect(dst);
}

void TestAnything::slot_Start(bool b)
{
	for (auto n : _nodes)
	{
		n->Reset();
		n->Activate();
	}
}

void TestAnything::slot_RemoveItems(QList<QGraphicsItem*>items)
{
	for (auto item : items)
	{
		switch (item->type())
		{
		case GuiGraphItemNode::Type:
			RemoveNode(const_cast<AlgGraphNode*> (&((qgraphicsitem_cast<GuiGraphItemNode*>(item))->controller._node)));
			break;
		case GuiGraphItemVertex::Type:
			delete const_cast<AlgGraphVertex*> (&((qgraphicsitem_cast<GuiGraphItemVertex*>(item))->_vertex));
			break;
		case GuiGraphItemArrow::Type:
		{
			auto arrow = qgraphicsitem_cast<GuiGraphItemArrow*>(item);
			RemoveConnection(const_cast<AlgGraphVertex*>(&arrow->srcItemVertex->_vertex),
				const_cast<AlgGraphVertex*>(&arrow->dstItemVertex->_vertex));
			break;
		}
		default:
			break;
		}
	}
}

void TestAnything::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _monitorTimerId)//用来监视避免内存泄露的
	{
		ui.lcdNumber_AlgNode->display(static_cast<int> (AlgGraphNode::GetAmount()));
		ui.lcdNumber_AlgVertex->display(static_cast<int> (AlgGraphVertex::GetAmount()));
		ui.lcdNumber_GuiController->display(static_cast<int> (GuiGraphController::GetAmount()));
		ui.lcdNumber_GuiItemNode->display(static_cast<int> (GuiGraphItemNode::GetAmount()));
		ui.lcdNumber_GuiItemVertex->display(static_cast<int> (GuiGraphItemVertex::GetAmount()));
		ui.lcdNumber_GuiItemArrow->display(static_cast<int> (GuiGraphItemArrow::GetAmount()));
	}
}
#pragma endregion TestAnything

#pragma region AlgGraphVertex
size_t AlgGraphVertex::_amount = 0;

void AlgGraphVertex::Activate(QVariant var, bool isActivate /*= true*/)
{
	if (isEnabled == true)
	{
		qDebug() << node.objectName() + ':' + objectName() << __FUNCTION__;
		emit sig_ActivateBegin();
		for (auto f : assertFunctions)
			if (f(var) == false)
				throw "AssertFail";//TODO:1.改成专用的GraphError；2.加入默认的类型确认部分

		_Activate(var, isActivate);
		emit sig_ActivateEnd();
	}
}
#pragma endregion AlgGraphVertex

#pragma region AlgGraphNode
AlgGraphNode::AlgGraphNode(QObject*parent, QThreadPool&pool)
	:QObject(parent), _pool(pool)
{
	connect(&_result, &QFutureWatcher<QVariantHash>::finished, this, [this] {emit sig_RunFinished(this); });
	connect(&_result, &QFutureWatcher<QVariantHash>::finished, this, &AlgGraphNode::Output);
	_amount++;
}
AlgGraphNode::~AlgGraphNode()
{
	qDebug() << objectName() << __FUNCTION__;

	emit sig_Destroyed(this);//主要从控制器中解离
	for (auto v : _inputVertex.values())//【必须先复制一份，不然删除时候会改变QHash】
		delete v;
	_inputVertex.clear();
	for (auto v : _outputVertex.values())
		delete v;
	_outputVertex.clear();
	if (_gui != nullptr)
		delete _gui;
	_amount--;
}

void AlgGraphNode::Reset()
{
	for (auto v : _inputVertex)
		v->Reset();
	for (auto v : _outputVertex)
		v->Reset();
	_pause = false;
	_stop = false;

	//_lock.unlock();//TODO:考虑加锁
}
void AlgGraphNode::Release()
{
	throw __FUNCTION__"Not Implement!";
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

AlgGraphVertex* AlgGraphNode::AddVertex(QString name, QVariant defaultValue, bool isInput)
{
	qDebug() << objectName() + ':' + name << isInput << __FUNCTION__;
	AlgGraphVertex* pv;
	assert(_inputVertex.contains(name) == false && _outputVertex.contains(name) == false);
	QString newName = name;//TODO:需要判定重名，或者自动添加尾注（例如in_1,in_2）
	if (isInput == true)
	{
		pv = new AlgGraphVertex_Input(*this, newName);
		_inputVertex.insert(newName, pv);
		connect(pv, &AlgGraphVertex::sig_Activated, this, &AlgGraphNode::Activate);
		connect(pv, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			_inputVertex.remove(vertex->objectName());
		}, Qt::DirectConnection);
		connect(pv, &AlgGraphVertex::objectNameChanged, this, [this](QString newname)
		{
			AlgGraphVertex*vtx = qobject_cast<AlgGraphVertex*>(sender());
			QString oldname = _inputVertex.key(vtx);
			_inputVertex.remove(oldname);
			_inputVertex.insert(newname, vtx);
		}, Qt::DirectConnection);
	}
	else
	{
		pv = new AlgGraphVertex_Output(*this, newName);
		_outputVertex.insert(newName, pv);
		connect(pv, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			_outputVertex.remove(vertex->objectName());
		}, Qt::DirectConnection);
		connect(pv, &AlgGraphVertex::objectNameChanged, this, [this](QString newname)
		{
			AlgGraphVertex*vtx = qobject_cast<AlgGraphVertex*>(sender());
			QString oldname = _outputVertex.key(vtx);
			_outputVertex.remove(oldname);
			_outputVertex.insert(newname, vtx);
		}, Qt::DirectConnection);
	}
	pv->defaultData = defaultValue;

	return pv;
}
QHash<QString, AlgGraphVertex*> AlgGraphNode::AddVertex(QHash<QString, QVariant>initTbl, bool isInput)
{
	throw __FUNCTION__"Not Implement!";
}
void AlgGraphNode::RemoveVertex(QString name, bool isInput)
{
	auto &vtxs = isInput ? _inputVertex : _outputVertex;
	//vtxs.remove(name);//TODO:名称出错时候的处理
	delete vtxs.value(name);
}
void AlgGraphNode::RemoveVertex(QStringList names, bool isInput)
{
	auto &vtxs = isInput ? _inputVertex : _outputVertex;
	for(auto name:names)
		delete vtxs.value(name);
}

void AlgGraphNode::ConnectVertex(QString vertexName, AlgGraphNode&dstNode, QString dstVertexName)
{
	throw __FUNCTION__"Not Implement!";
}
void AlgGraphNode::DisconnectVertex(QString vertexName)
{
	throw __FUNCTION__"Not Implement!";
}
void AlgGraphNode::DisconnectVertex(QString vertexName, AlgGraphNode&dstNode, QString dstVertexName)
{
	throw __FUNCTION__"Not Implement!";
}

void AlgGraphNode::Write()
{
	throw __FUNCTION__"Not Implement!";
}
void AlgGraphNode::Read()
{
	throw __FUNCTION__"Not Implement!";
}

void AlgGraphNode::Activate()
{
	qDebug() << objectName() << __FUNCTION__;
	if (_result.isRunning() == false)//运行期间，阻塞输入
	{
		for (auto v:_inputVertex)//检查是否全部激活
			if (v->isActivated == false)
				return;
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
		_result.setFuture(QtConcurrent::run(&_pool, this, &AlgGraphNode::_Run, data));
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
	for (auto v:_outputVertex)
		v->Activate(v->data, true);
	emit sig_OutputFinished(this);
}

void AlgGraphNode::Pause(bool isPause)
{
	_pause = isPause;
}
void AlgGraphNode::Stop(bool isStop)
{
	_stop = isStop;
}

QVariantHash AlgGraphNode::_LoadInput()
{
	QHash<QString, QVariant>result;
	for (auto it = _inputVertex.begin(); it != _inputVertex.end(); ++it)
		result.insert(it.key(), it.value()->data);
	return result;
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

size_t AlgGraphNode::_amount = 0;

QVariantHash AlgGraphNode::_Run(QVariantHash data)
{
	QThread::msleep(500);
	return data;
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
#pragma endregion

#pragma region GuiGraphItem
GuiGraphItemVertex::GuiGraphItemVertex(GuiGraphItemNode&parent, const AlgGraphVertex&vertex, bool isInput) :QGraphicsItem(&parent), _nodeItem(parent), _vertex(vertex), _isInput(isInput)
{
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsScenePositionChanges);
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
	_amount++;
}

GuiGraphItemVertex::~GuiGraphItemVertex()
{
	qDebug() << _vertex.objectName() << __FUNCTION__;
	const_cast<AlgGraphVertex&>(_vertex).gui = nullptr;//TODO:后面改成Detach
	if (_isInput == true)
		const_cast<GuiGraphItemNode&>(_nodeItem).inputItemVertex.remove(&_vertex);
	else
		const_cast<GuiGraphItemNode&>(_nodeItem).outputItemVertex.remove(&_vertex);
	auto tmpArrow = _arrows;
	for (auto a : tmpArrow)
		delete a;
	_arrows.clear();
	_amount--;
}


size_t GuiGraphItemVertex::_amount = 0;

#pragma endregion GuiGraphItem

#pragma region GuiGraphController
GuiGraphController::GuiGraphController(const AlgGraphNode&node, GraphScene&scene)
	:QObject(const_cast<AlgGraphNode*>(&node)), _node(node), _scene(scene)
{
	setObjectName(_node.objectName());
	connect(&node, &AlgGraphNode::objectNameChanged, [this](const QString name)
	{
		setObjectName(name);
		if (_nodeItem != nullptr)
			_nodeItem->Refresh();
	});
	_amount++;
}

GuiGraphController::~GuiGraphController()
{
	qDebug() << _node.objectName() << __FUNCTION__;
	emit sig_Destroyed(this);
	if (_nodeItem != nullptr)
	{
		_scene.removeItem(_nodeItem);
		delete _nodeItem;
	}
	if (_panel.isNull() == false)
		delete _panel;
	//const_cast<AlgGraphNode&>(_node).DetachGui();
	_amount--;
}
QWidget* GuiGraphController::InitWidget(QWidget*parent)
{
	_panel = new QPlainTextEdit(parent);
	//connect(qobject_cast<QPlainTextEdit*>(_panel), &QPlainTextEdit::textChanged, this, &GuiGraphNode::sig_ValueChanged);
	// 		connect(this, &GuiGraphNode::destroyed, this, [this] 
	// 		{
	// 			_nodeItem->scene()->removeItem(_nodeItem); 
	// 			delete _nodeItem;
	// 		});
	return _panel;
}

GuiGraphItemVertex* GuiGraphController::AddVertex(const AlgGraphVertex*vtx, const bool isInput)
{
	assert(_nodeItem != nullptr);
	if (_nodeItem == nullptr)
		return nullptr;
	auto vtxItem = new GuiGraphItemVertex(*_nodeItem, *vtx, isInput);
	const_cast<AlgGraphVertex*>(vtx)->gui = vtxItem;
	if(isInput==true)
	{
		connect(vtx, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			if (_nodeItem != nullptr)
			{
				_scene.removeItem(_nodeItem->inputItemVertex.take(vertex));
				_nodeItem->Refresh();
			}
		});
		_nodeItem->inputItemVertex.insert(vtx, vtxItem);
	}
	else
	{
		connect(vtx, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			if (_nodeItem != nullptr)
			{
				_scene.removeItem(_nodeItem->outputItemVertex.take(vertex));
				_nodeItem->Refresh();
			}
		});
		_nodeItem->outputItemVertex.insert(vtx, vtxItem);
	}
	return vtxItem;
}

size_t GuiGraphController::_amount = 0;

GuiGraphItemNode* GuiGraphController::InitApperance(QPointF center /*= QPointF(0, 0)*/, QRectF size /*= QRectF(0, 0, 100, 100)*/)
{
	if (_nodeItem != nullptr)
	{
		delete _nodeItem;
		_nodeItem = nullptr;
	}
	_nodeItem = new GuiGraphItemNode(size, nullptr, *this);
	_nodeItem->setPos(center);
	
	for (auto vtx : _node.GetVertexes(true))
		AddVertex(vtx, true);
	for (auto vtx : _node.GetVertexes(false))
		AddVertex(vtx, false);

	_nodeItem->Refresh();
	_scene.addItem(_nodeItem);
	return _nodeItem;
}

GuiGraphItemNode::~GuiGraphItemNode()
{
	qDebug() << title.toPlainText() << __FUNCTION__;
	controller.DetachItem();
	for (auto v : inputItemVertex.values())
		delete v;
	inputItemVertex.clear();
	for (auto v : outputItemVertex.values())
		delete v;
	outputItemVertex.clear();
	_amount--;
}

void GuiGraphItemNode::Refresh()
{
	auto box = boundingRect();
	//标题
	title.setPlainText(controller.objectName());
	title.setPos(box.width() / 2 - title.boundingRect().width() / 2, 0);

	int h = title.boundingRect().height();
	for (auto vtxItem : inputItemVertex)
	{
		vtxItem->setPos(0, h);
		h += vtxItem->boundingRect().height();
	}
	h = title.boundingRect().height();
	for (auto vtxItem : outputItemVertex)
	{
		vtxItem->setPos(boundingRect().width() - vtxItem->boundingRect().width(), h);
		h += vtxItem->boundingRect().height();
	}
}

size_t GuiGraphItemNode::_amount=0;

#pragma endregion GuiGraphItem

#pragma region GraphScene
void GraphScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (arrow == nullptr)
		{
			auto item = itemAt(event->scenePos(), QTransform());
			auto vtxItem = qgraphicsitem_cast<GuiGraphItemVertex*>(item);
			if (vtxItem != nullptr)
			{
				auto pos = item->mapToScene(item->boundingRect().center());
				arrow = new GuiGraphItemArrow(vtxItem, nullptr);
				arrow->setLine(QLineF(arrow->line().p1(), pos));
				connect(&vtxItem->_vertex, &AlgGraphVertex::sig_Destroyed, [this]
				{
					if (arrow != nullptr)
						delete arrow;
					arrow = nullptr;
				});
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
#pragma endregion GraphScene

#pragma region GuiGraphItemArrow
GuiGraphItemArrow::GuiGraphItemArrow(const GuiGraphItemVertex*src, const GuiGraphItemVertex*dst)
	:QGraphicsLineItem(nullptr),srcItemVertex(src),dstItemVertex(dst)
{
	setFlag(QGraphicsItem::ItemIsSelectable);
	updatePosition();
	++_amount;
}

GuiGraphItemArrow::~GuiGraphItemArrow()
{
	qDebug() << __FUNCTION__;
	if (srcItemVertex != nullptr) 
	{
		const_cast<GuiGraphItemVertex*>(srcItemVertex)->_arrows.removeOne(this);
		srcItemVertex = nullptr;
	}
	if (dstItemVertex != nullptr)
	{
		const_cast<GuiGraphItemVertex*>(dstItemVertex)->_arrows.removeOne(this);
		dstItemVertex = nullptr;
	}
	--_amount;
}

void GuiGraphItemArrow::updatePosition()
{
	QPointF p1 = (srcItemVertex != nullptr) ? (srcItemVertex->mapToScene(srcItemVertex->ArrowAttachPosition())) : line().p1(),
		p2 = (dstItemVertex != nullptr) ? (dstItemVertex->mapToScene(dstItemVertex->ArrowAttachPosition())) : line().p1();

	setLine(QLineF(p1, p2));
}
size_t GuiGraphItemArrow::_amount = 0;
void GuiGraphItemArrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	QGraphicsLineItem::paint(painter, option, widget);
}
#pragma endregion GuiGraphItemArrow



