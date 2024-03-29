﻿#include "TestAnything.h"
#include <QPlainTextEdit>
#include <QGraphicsSceneMouseEvent>

#include <qDebug>
#include <opencv.hpp>
#include <functional>
#include <vld.h>
#include <QMessageBox>
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
		connect(gui, &GuiGraphController::sig_ValueChanged, node, &AlgGraphNode::Activate);
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
	connect(ui.pushButton_NodeBuffer, &QPushButton::clicked, this, [this](bool b)
	{
		auto node = new AlgGraphNode_Buffer(this, _pool);
		auto gui = new GuiGraphController(*node, _scene);
		AddNode(*node, gui, QPointF(0, 0));
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
	connect(ui.actionStop, &QAction::triggered, this, &TestAnything::slot_Stop);
	bool (TestAnything::*pAddConnection)(GuiGraphItemVertex&, GuiGraphItemVertex&) = &TestAnything::AddConnection;//注意这里要这样写来区分重载函数
	connect(&_scene, &GraphScene::sig_ConnectionAdded, this, pAddConnection);
	connect(&_scene, &GraphScene::sig_RemoveItems, this, &TestAnything::slot_RemoveItems);
	connect(&_scene, &GraphScene::sig_ActionTriggered, this, &TestAnything::slot_ActionProcessor);
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

bool TestAnything::AddConnection(GuiGraphItemVertex&srcItemVertex, GuiGraphItemVertex&dstItemVertex)
{
 	return AddConnection(const_cast<AlgGraphVertex&>(srcItemVertex._vertex),
 		const_cast<AlgGraphVertex&>(dstItemVertex._vertex));
}

bool TestAnything::AddConnection(AlgGraphVertex&srcVertex, AlgGraphVertex&dstVertex)
{
	try
	{
		//连接Alg Vertex
		//bool b = srcVertex.node.ConnectVertex(srcVertex.objectName(), srcVertex.vertexType,
		//	dstVertex.node, dstVertex.objectName(), dstVertex.vertexType);
		//assert(b);
		GRAPH_ASSERT(srcVertex.node.ConnectVertex(srcVertex.objectName(), srcVertex.vertexType,
				dstVertex.node, dstVertex.objectName(), dstVertex.vertexType));
		//连接Gui Vertex
		auto srcItem = srcVertex.gui, dstItem = dstVertex.gui;
		//b = srcItem != nullptr && dstItem != nullptr;
		//assert(b);	
		GRAPH_ASSERT(srcItem != nullptr && dstItem != nullptr);
		//if (!b)	return true;//注意这里仍然返回true因为连接已成功，只是后面可以添加一个warning来提示
		auto arrow = new GuiGraphItemArrow(srcItem, dstItem);
		arrow->setZValue(srcItem->zValue() - 0.2);
		srcItem->_arrows.append(arrow);
		dstItem->_arrows.append(arrow);
		_scene.addItem(arrow);

		return true;
	}
	catch (GraphError&e)
	{
		QMessageBox::information(this, "连接错误", e.msg);
		return false;
	}
}

void TestAnything::RemoveConnection(AlgGraphVertex*src, AlgGraphVertex*dst)
{
	src->Disconnect(dst);
}

void TestAnything::slot_Start(bool b)
{
	//ui.actionStart->setEnabled(false);
	for (auto n : _nodes)
	{
		n->Reset();
		n->Activate();
	}
}

void TestAnything::slot_Stop(bool b)
{
	for (auto n : _nodes)
		n->Stop(b);
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

void TestAnything::slot_ActionProcessor(QGraphicsItem*item, QAction*action)
{
	static QStringList easyFeatures = { "Delete" };//可以在这里处理的简单工作，TODO:之后应该会改成宏定义并放在其它地方
	if (easyFeatures.contains(action->objectName()) == true)
	{
		if (action->objectName() == "Delete")
			slot_RemoveItems({ item });
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
		ui.lcdNumber_Running->display(static_cast<int> (AlgGraphNode::GetRunningAmount()));
	}
}
#pragma endregion TestAnything

#pragma region AlgGraphVertex
size_t AlgGraphVertex::_amount = 0;

AlgGraphVertex::AlgGraphVertex(AlgGraphNode&parent, QString name, VertexType vertexType)
	:QObject(&parent), node(parent), vertexType(vertexType)
{
	setObjectName(name);
	_amount++;
}

AlgGraphVertex::~AlgGraphVertex()
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

void AlgGraphVertex::Activate(QVariant var, bool isAct /*= true*/)
{
	if (isEnabled == true)
	{
		qDebug() << node.objectName() + ':' + objectName() << __FUNCTION__;
		emit sig_ActivateBegin();
		for (auto f : inputAssertFunctions)
			if (f(this,var) == false)
				throw "AssertFail";//TODO:1.改成专用的GraphError；2.加入默认的类型确认部分

		if (isAct == true)
			data = var;
		else
			data.clear();
		isActivated = isAct;
		emit sig_Activated(var, isAct);
		emit sig_ActivateEnd();
	}
}

#pragma endregion AlgGraphVertex

#pragma region AlgGraphNode
AlgGraphNode::AlgGraphNode(QObject*parent, QThreadPool&pool)
	:QObject(parent), _pool(pool)
{
	sizeof(QObject);
	connect(&_result, &QFutureWatcher<QVariantHash>::finished, this, [this] {emit sig_RunFinished(this); });
	connect(&_result, &QFutureWatcher<QVariantHash>::finished, this, &AlgGraphNode::Output);
	_amount++;
}
AlgGraphNode::~AlgGraphNode()
{
	qDebug() << objectName() << __FUNCTION__;
	blockSignals(true);
	_result.waitForFinished();//TODO:停止线程
	blockSignals(false);
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
	blockSignals(true);
	_result.waitForFinished();//TODO:停止线程
	blockSignals(false);
	for (auto v : _inputVertex)
		v->Reset();
	for (auto v : _outputVertex)
		v->Reset();
	_pause = false;
	_stop = false;
	_isRunning = false;
	
	//_lock.unlock();//TODO:考虑加锁
}
void AlgGraphNode::Release()
{
	throw __FUNCTION__"Not Implement!";
}

void AlgGraphNode::Init()
{
	AddVertex("in", "in", AlgGraphVertex::VertexType::INPUT);
	AddVertex("out", "out", AlgGraphVertex::VertexType::OUTPUT);
}
void AlgGraphNode_Input::Init()
{
	AddVertex("out", "out", AlgGraphVertex::VertexType::OUTPUT);
}
void AlgGraphNode_Output::Init()
{
	AddVertex("in", "in", AlgGraphVertex::VertexType::INPUT);
}
void AlgGraphNode_Add::Init()
{
	AddVertex("in1", "in", AlgGraphVertex::VertexType::INPUT);
	AddVertex("in2", "in", AlgGraphVertex::VertexType::INPUT);
	AddVertex("out", "out", AlgGraphVertex::VertexType::OUTPUT);
}
void AlgGraphNode_Buffer::Init()
{
	AddVertex("in", "in", AlgGraphVertex::VertexType::INPUT);
	AddVertex("out", "out", AlgGraphVertex::VertexType::OUTPUT);
}

//特殊点：输出端口最多连接一个；下一个Node运行完毕后再向其激活；下一个Vertex激活后清空其输入
bool AlgGraphNode_Buffer::ConnectVertex(QString vertexName, AlgGraphVertex::VertexType vertexType, AlgGraphNode&dstNode, QString dstVertexName, AlgGraphVertex::VertexType dstVertexType)
{
	if (vertexType == AlgGraphVertex::VertexType::OUTPUT)
		assert(_outputVertex.value(vertexName)->nextVertexes.size() == 0
		&& _outputVertex.value(vertexName)->prevVertexes.size() == 0);//输出端口最多连接一个
	if (AlgGraphNode::ConnectVertex(vertexName, vertexType, dstNode, dstVertexName, dstVertexType) == false)
		return false;
	connect(&_outputVertex.value(vertexName)->nextVertexes[0]->node, &AlgGraphNode::sig_ActivateReady,
		[this] { _isActivateByNext = true; Activate(); });//【下一个节点运行完毕后再向其激活】
	connect(&_outputVertex.value(vertexName)->nextVertexes[0]->node, &AlgGraphNode::sig_Activated,
		_outputVertex.value(vertexName),&AlgGraphVertex::Deactivate);//下一个节点激活后清空其输入
	return true;
}


AlgGraphVertex* AlgGraphNode::AddVertex(QString name, QVariant defaultValue, AlgGraphVertex::VertexType vertexType)
{
	qDebug() << objectName() + ':' + name << vertexType << __FUNCTION__;
	AlgGraphVertex* pv;
	assert(_isUnchange == false);
	assert(_inputVertex.contains(name) == false && _outputVertex.contains(name) == false);
	QString newName = name;//TODO:需要判定重名，或者自动添加尾注（例如in_1,in_2）
	if (vertexType == AlgGraphVertex::VertexType::INPUT)
	{
		pv = new AlgGraphVertex(*this, newName,AlgGraphVertex::VertexType::INPUT);
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
	else if (vertexType == AlgGraphVertex::VertexType::OUTPUT)
	{
		pv = new AlgGraphVertex(*this, newName,AlgGraphVertex::VertexType::OUTPUT);
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
QHash<QString, AlgGraphVertex*> AlgGraphNode::AddVertex(QHash<QString, QVariant>initTbl, AlgGraphVertex::VertexType vertexType)
{
	throw __FUNCTION__"Not Implement!";
}
void AlgGraphNode::RemoveVertex(QString name, AlgGraphVertex::VertexType vertexType)
{
	auto &vtxs = _GetVertexes(vertexType);
	//vtxs.remove(name);//TODO:名称出错时候的处理
	delete _GetVertexes(vertexType).value(name);
}
void AlgGraphNode::RemoveVertex(QStringList names, AlgGraphVertex::VertexType vertexType)
{
	auto &vtxs = _GetVertexes(vertexType);
	for(auto name:names)
		delete vtxs.value(name);
}

bool AlgGraphNode::ConnectVertex(QString vertexName, AlgGraphVertex::VertexType vertexType, AlgGraphNode&dstNode, QString dstVertexName, AlgGraphVertex::VertexType dstVertexType)
{
	auto srcVertex = _GetVertexes(vertexType).value(vertexName);
	auto dstVertex = dstNode._GetVertexes(dstVertexType).value(dstVertexName);
	 
// 	bool b = srcVertex != dstVertex //不允许指向同一个
// 		&& srcVertex != nullptr && dstVertex != nullptr //非空
// 		&& srcVertex->nextVertexes.contains(dstVertex) == false
// 		&& srcVertex->prevVertexes.contains(dstVertex) == false
// 		&& dstVertex->nextVertexes.contains(srcVertex) == false
// 		&& dstVertex->prevVertexes.contains(srcVertex) == false;//不允许重复和反向的连接
// 	assert(b);
// 	if (!b)	return false;
	GRAPH_ASSERT(srcVertex != dstVertex //不允许指向同一个
		&& srcVertex != nullptr && dstVertex != nullptr //非空
		&& srcVertex->nextVertexes.contains(dstVertex) == false
		&& srcVertex->prevVertexes.contains(dstVertex) == false
		&& dstVertex->nextVertexes.contains(srcVertex) == false
		&& dstVertex->prevVertexes.contains(srcVertex) == false);

	qDebug() << srcVertex->node.objectName() + ':' + srcVertex->objectName()
		<< dstVertex->node.objectName() + ':' + dstVertex->objectName() << __FUNCTION__;
	srcVertex->Connect(dstVertex);
	connect(srcVertex, &AlgGraphVertex::sig_Activated, dstVertex, &AlgGraphVertex::Activate, Qt::QueuedConnection);//必须改成QueuedConnection，否则是直连，相当于会递归调用进行深度优先遍历
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
			//throw "Can't Find";//TODO:改成相应Error【去掉这行是否无影响，有待观察】
		}
	}, Qt::UniqueConnection);//【注意这里务必加入UniqueConnection避免重复发送】

	return true;
}
void AlgGraphNode::DisconnectVertex(QString vertexName, AlgGraphVertex::VertexType vertexType)
{
	throw __FUNCTION__"Not Implement!";
}
void AlgGraphNode::DisconnectVertex(QString vertexName, AlgGraphVertex::VertexType vertexType,
	AlgGraphNode&dstNode, QString dstVertexName, AlgGraphVertex::VertexType dstVertexType)
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
	if (_isRunning == false && _stop == false)//运行期间，阻塞输入
	{
		for (auto v : _inputVertex)//检查是否全部激活
			if (v->isActivated == false)
				return;
		_isRunning = true;
		_runningAmount++;
		Run();
	}
}
void AlgGraphNode::Run()
{
	//TODO:加锁
	qDebug() << objectName() << __FUNCTION__;
	auto data = _LoadInput();
	emit sig_Activated(this);
	//TODO:暂停和退出
	switch (_mode)
	{
	case AlgGraphNode::RunMode::Thread:
		_result.setFuture(QtConcurrent::run(&_pool, this, &AlgGraphNode::_Run, data));
		break;
	case AlgGraphNode::RunMode::Direct: 
		try
		{
			_LoadOutput(_Run(data));
		}
		catch(GraphError&e)
		{
			qDebug() << e.msg;
			Stop(true);
		}
		catch (QException&e)
		{
			qDebug() << e.what();
			Stop(true);
		}
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
	{
		try
		{
			_result.waitForFinished();
			_LoadOutput((_result.future().resultCount() > 0) ? (_result.result()) : (QVariantHash()));
		}
		catch (GraphError&e)
		{
			qDebug() << e.msg;
			Stop(true);
		}
		catch (QException&e)
		{
			qDebug() << e.what();
			Stop(true);
		}
	}
	//_result.setFuture(QFuture<QVariantHash>());
	//_result.();
	if (_stop == false) 
	{
		for (auto v : _outputVertex)
		{
			if (v->data.isNull() == false)
			{
				v->Activate(v->data, true);
				v->Reset();
			}
		}
		emit sig_OutputFinished(this);
	}
	_runningAmount--;
	_isRunning = false;
	emit sig_ActivateReady(this);
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
		auto vtx = _outputVertex.value(it.key());
		if (vtx != nullptr)
			vtx->data = it.value();
		//else GraphWarning();//TODO:警告
	}
}

std::atomic_uint64_t AlgGraphNode::_amount = 0;

std::atomic_uint64_t AlgGraphNode::_runningAmount = 0;

QVariantHash AlgGraphNode::_Run(QVariantHash data)
{
	QThread::msleep(500);
	return data;
}

QVariantHash AlgGraphNode_Input::_Run(QVariantHash data)
{
	//QThread::msleep(500);
	assert(_gui.isNull() == false);
	QVariantHash result;
	result["out"] = _gui->GetData();
	return result;
}
QVariantHash AlgGraphNode_Output::_Run(QVariantHash data)
{
	QThread::msleep(500);
	assert(_gui.isNull() == false);
	for (auto d : data)
		_gui->SetData(d);
	return data;
}
QVariantHash AlgGraphNode_Add::_Run(QVariantHash data)
{
	QThread::msleep(500);
	auto keys = data.keys();
	keys.sort();
	QVariantHash result;
	QString s;
	for (auto k : keys)
		s += data.value(k).toString();
	result["out"] = s;
	return result;
}
QVariantHash AlgGraphNode_Buffer::_Run(QVariantHash data)
{
	//QThread::msleep(500);
	//qDebug() << objectName() << __FUNCTION__;
	QVariantHash result;
	if (_isActivateByNext == false) //是由inputVertex所Activate的
	{
		if (_isFlatArray == true && data.value("in").canConvert(QVariant::List) == true)
		{
			for (auto v:data.value("in").toList())
				_qdata.push_back(v);
		}
		else
			_qdata.push_back(data.value("in"));
	}
	if (_qdata.size() > 0 && _outputVertex.value("out")->nextVertexes[0]->node.isRunning() == false) 
	{
		result.insert("out", _qdata.takeFirst());
	}
	_isActivateByNext = false;
	return result;
}
#pragma endregion

#pragma region GuiGraphItem
GuiGraphItemVertex::GuiGraphItemVertex(GuiGraphItemNode&parent, const AlgGraphVertex&vertex) 
	:QGraphicsItem(&parent), _nodeItem(parent), _vertex(vertex)
{
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsScenePositionChanges);
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
	_amount++;
}

GuiGraphItemVertex::~GuiGraphItemVertex()
{
	qDebug() << _vertex.objectName() << __FUNCTION__;
	const_cast<AlgGraphVertex&>(_vertex).gui = nullptr;//TODO:后面改成Detach
	if (_vertex.vertexType== AlgGraphVertex::VertexType::INPUT)
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
	connect(&node, &AlgGraphNode::sig_Activated, [this] {_nodeItem->update(); });
	connect(&node, &AlgGraphNode::sig_ActivateReady, [this] {_nodeItem->update(); });
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

GuiGraphItemVertex* GuiGraphController::AddVertex(const AlgGraphVertex&vtx, const bool isInput)
{
	assert(_nodeItem != nullptr);
	if (_nodeItem == nullptr)
		return nullptr;
	auto vtxItem = new GuiGraphItemVertex(*_nodeItem, vtx);
	const_cast<AlgGraphVertex*>(&vtx)->gui = vtxItem;
	if(isInput==true)
	{
		connect(&vtx, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			if (_nodeItem != nullptr)
			{
				_scene.removeItem(_nodeItem->inputItemVertex.take(vertex));
				_nodeItem->Refresh();
			}
		});
		_nodeItem->inputItemVertex.insert(&vtx, vtxItem);
	}
	else
	{
		connect(&vtx, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			if (_nodeItem != nullptr)
			{
				_scene.removeItem(_nodeItem->outputItemVertex.take(vertex));
				_nodeItem->Refresh();
			}
		});
		_nodeItem->outputItemVertex.insert(&vtx, vtxItem);
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
		AddVertex(*vtx, true);
	for (auto vtx : _node.GetVertexes(false))
		AddVertex(*vtx, false);

	_nodeItem->Refresh();
	_scene.addItem(_nodeItem);
	return _nodeItem;
}

GuiGraphItemNode::~GuiGraphItemNode()
{
	qDebug() << title.toPlainText() << __FUNCTION__;
	const_cast<GuiGraphController&>(controller).DetachItem();
	for (auto v : inputItemVertex.values())
		delete v;
	inputItemVertex.clear();
	for (auto v : outputItemVertex.values())
		delete v;
	outputItemVertex.clear();
	_amount--;
}

QSharedPointer<QMenu> GuiGraphItemNode::GetDefaultMenu()
{

	static auto m = QSharedPointer<QMenu>::create();
	if (m->actions().size() == 0) 
	{
		m->addAction("Delete")->setObjectName("A`Delete");
		m->addAction("AddInput")->setObjectName("A`AddInput");
		m->addAction("AddOutput")->setObjectName("A`AddOutput");
		m->addAction("AddInput")->setObjectName("A`AddInput");
		m->addAction("AddOutput")->setObjectName("A`AddOutput");
	}
	return m;
}

void GuiGraphItemNode::Refresh()
{
	auto box = boundingRect();
	//标题
	title.setPlainText(controller.objectName());
	title.setPos(box.width() / 2 - title.boundingRect().width() / 2, 0);

	int h = title.boundingRect().height();
	QMap<QString, GuiGraphItemVertex*>sortVtxs;//按名称排序
	for (auto vtxItem : inputItemVertex)
		sortVtxs.insert(vtxItem->_vertex.objectName(), vtxItem);
	for (auto vtxItem : sortVtxs)
	{
		vtxItem->setPos(0, h);
		h += vtxItem->boundingRect().height();
	}

	h = title.boundingRect().height();
	sortVtxs.clear();//按名称排序
	for (auto vtxItem : outputItemVertex)
		sortVtxs.insert(vtxItem->_vertex.objectName(), vtxItem);
	for (auto vtxItem : sortVtxs)
	{
		vtxItem->setPos(boundingRect().width() - vtxItem->boundingRect().width(), h);
		h += vtxItem->boundingRect().height();
	}
}

size_t GuiGraphItemNode::_amount=0;

void GuiGraphItemNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
{
	if (controller._node.isRunning() == true)
		painter->setPen(QColor(255, 0, 0));
	else
		painter->setPen(QColor(0, 0, 0));
	painter->drawRect(25, 25, 50, 50);
	QGraphicsRectItem::paint(painter, option, widget);
}

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
					emit sig_ConnectionAdded(*qgraphicsitem_cast<GuiGraphItemVertex*>(srcItem)
						, *qgraphicsitem_cast<GuiGraphItemVertex*>(dstItem));
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



void GraphScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	auto pos = event->screenPos();
	auto item = itemAt(event->scenePos(), QTransform());
	if (item != nullptr)
	{
		QAction*result = nullptr;
		switch (item->type())
		{
		case GuiGraphItemNode::Type:
		{
			auto n = qgraphicsitem_cast<GuiGraphItemNode*>(item);
			result = n->GetMenu()->exec(pos);
			break;
		}
		default:
			break;
		}
		if (result != nullptr)
		{
			qDebug() << result->objectName() << result->text() << __FUNCTION__;
			//if (0)
			//	sendEvent(item, nullptr);
			//else
			emit sig_ActionTriggered(item, result);
		}
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




