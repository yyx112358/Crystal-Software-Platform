#include "TestAnything.h"
#include <QPlainTextEdit>
#include <QGraphicsSceneMouseEvent>

#include <qDebug>
#include <opencv.hpp>
#include <functional>
#include <vld.h>
using namespace cv;

TestAnything::TestAnything(QWidget *parent)
	: QMainWindow(parent), _pool(this), _scene(this)
{
	ui.setupUi(this);
	ui.graphicsView->setScene(&_scene);
	connect(ui.pushButton_NodeInput, &QPushButton::clicked, this, [this](bool b)
	{
		auto node = new AlgGraphNode(this, _pool);
		AddNode(*node, new GuiGraphController(*node, _scene), QPointF(-100, 0));
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
	connect(ui.pushButton_5, &QPushButton::clicked, this, [this](bool b)
	{
		_scene.clear();
	});
	connect(ui.actionStart, &QAction::triggered, this, &TestAnything::slot_Start);
	void (TestAnything::*pAddConnection)(GuiGraphItemVertex*, GuiGraphItemVertex*) = &TestAnything::AddConnection;//注意这里要这样写来区分重载函数
	connect(&_scene, &GraphScene::sig_ConnectionAdded, this, pAddConnection);
	connect(&_scene, &GraphScene::sig_RemoveItems, this, &TestAnything::slot_RemoveItems);
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
	connect(guiNode, &GuiGraphController::sig_Destroyed, &node, &AlgGraphNode::DetachGui, Qt::DirectConnection);//GUI�Զ����
	auto guiItem = guiNode->InitApperance(center);
	return guiNode;
}

void TestAnything::RemoveNode(AlgGraphNode*node)
{
	_nodes.removeOne(node);
	delete node;
}

void TestAnything::AddConnection(GuiGraphItemVertex*srcVertex, GuiGraphItemVertex*dstVertex)
{
	AddConnection(const_cast<AlgGraphVertex*>(&srcVertex->_vertex),
		const_cast<AlgGraphVertex*>(&dstVertex->_vertex));
}

void TestAnything::AddConnection(AlgGraphVertex*srcVertex, AlgGraphVertex*dstVertex)
{
	bool b = srcVertex != dstVertex //不允许指向同一个
		&& srcVertex->connectedVertexes.contains(dstVertex) == false//不允许重复的连接
		&& dstVertex->connectedVertexes.contains(srcVertex) == false;
	assert(b);	if (!b)	return;

	srcVertex->Connect(dstVertex);
	connect(srcVertex, &AlgGraphVertex::sig_Activated, dstVertex, &AlgGraphVertex::Activate);

	auto srcItem = srcVertex->gui, dstItem = dstVertex->gui;
	b = srcItem != nullptr&&dstItem != nullptr;
	assert(b);	if (!b)	return;
	connect(srcVertex, &AlgGraphVertex::sig_ConnectionRemoved, [](AlgGraphVertex*src, AlgGraphVertex*dst)
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
			throw "Can't Find";
		}
	});
	auto arrow = new GuiGraphItemArrow(srcItem, dstItem);
	arrow->setZValue(srcItem->zValue() - 0.2);
	srcItem->_arrows.append(arrow);
	dstItem->_arrows.append(arrow);
	_scene.addItem(arrow);
}

void TestAnything::slot_Start(bool b)
{

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
		default:
			break;
		}
	}
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
}
#pragma region AlgGraphNode
AlgGraphNode::AlgGraphNode(QObject*parent, QThreadPool&pool)
	:QObject(parent), _pool(pool)
{
	connect(&_result, &QFutureWatcher<QVariantHash>::finished, this, [this] {emit sig_RunFinished(this); });
	connect(&_result, &QFutureWatcher<QVariantHash>::finished, this, &AlgGraphNode::Output);
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
}

void AlgGraphNode::Init()
{
	AddVertex("in1", "in1", true);
	AddVertex("in2", "in2", true);
	AddVertex("out1", "out1", false);
	AddVertex("out2", "out2", false);
}
void AlgGraphNode::Reset()
{

}
void AlgGraphNode::Release()
{

}

AlgGraphVertex* AlgGraphNode::AddVertex(QString name, QVariant defaultValue, bool isInput)
{
	AlgGraphVertex* pv;
	assert(_inputVertex.contains(name) == false && _outputVertex.contains(name) == false);
	QString newName = name;//TODO:需要判定重名，或者自动添加尾注（例如in_1,in_2）
	if (isInput == true)
	{
		pv = new AlgGraphVertex/*_Input*/(*this, newName);
		_inputVertex.insert(newName, pv);
		connect(pv, &AlgGraphVertex::sig_Activated, this, &AlgGraphNode::Activate);
		connect(pv, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			_inputVertex.remove(vertex->objectName());
		}, Qt::DirectConnection);
	}
	else
	{
		pv = new AlgGraphVertex/*_Output*/(*this, newName);
		_outputVertex.insert(newName, pv);
		connect(pv, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			_outputVertex.remove(vertex->objectName());
		}, Qt::DirectConnection);
	}
	pv->defaultData = defaultValue;

	return pv;
}
QHash<QString, AlgGraphVertex*> AlgGraphNode::AddVertex(QHash<QString, QVariant>initTbl, bool isInput)
{
	throw __FUNCTION__"Not Implement!";
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
void AlgGraphNode::Run()
{

}
void AlgGraphNode::Output()
{

}

void AlgGraphNode::Pause(bool isPause)
{

}
void AlgGraphNode::Stop(bool isStop)
{

}

QVariantHash AlgGraphNode::_LoadInput()
{
	throw __FUNCTION__"Not Implement!";
}
QVariantHash AlgGraphNode::_Run(QVariantHash data)
{
	throw __FUNCTION__"Not Implement!";
}
void AlgGraphNode::_LoadOutput(QVariantHash result)
{

}
#pragma endregion

#pragma region GuiGraphItem
GuiGraphItemVertex::GuiGraphItemVertex(GuiGraphItemNode&parent, const AlgGraphVertex&vertex, bool isInput) :QGraphicsItem(&parent), _nodeItem(parent), _vertex(vertex), _isInput(isInput)
{
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsScenePositionChanges);
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
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
}


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

GuiGraphItemNode* GuiGraphController::InitApperance(QPointF center /*= QPointF(0, 0)*/, QRectF size /*= QRectF(0, 0, 100, 100)*/)
{
	if (_nodeItem != nullptr)
	{
		delete _nodeItem;
		_nodeItem = nullptr;
	}
	_nodeItem = new GuiGraphItemNode(QRectF(0, 0, 100, 100), nullptr, *this);
	
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

#pragma endregion GuiGraphItem

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

GuiGraphItemArrow::GuiGraphItemArrow(const GuiGraphItemVertex*src, const GuiGraphItemVertex*dst)
	:QGraphicsLineItem(nullptr),srcItemVertex(src),dstItemVertex(dst)
{
	updatePosition();
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
}

void GuiGraphItemArrow::updatePosition()
{
	QPointF p1 = (srcItemVertex != nullptr) ? (srcItemVertex->mapToScene(srcItemVertex->ArrowAttachPosition())) : line().p1(),
		p2 = (dstItemVertex != nullptr) ? (dstItemVertex->mapToScene(dstItemVertex->ArrowAttachPosition())) : line().p1();

	setLine(QLineF(p1, p2));
}
