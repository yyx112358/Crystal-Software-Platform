#include "TestAnything.h"
#include <QPlainTextEdit>
#include <QGraphicsSceneMouseEvent>

#include <qDebug>
#include <opencv.hpp>
#include <functional>
#include <vld.h>
using namespace cv;

static GuiGraphItemVertex*GetVertexAlg2Gui(const AlgGraphVertex&algvtx)
{
	auto nodeItem = algvtx.node.GetGui()->GetItem();
	if (nodeItem->_inputVertexItem.contains(&algvtx))
		return nodeItem->_inputVertexItem.value(&algvtx);
	else if (nodeItem->_outputVertexItem.contains(&algvtx))
		return nodeItem->_outputVertexItem.value(&algvtx);
	else
		throw "Not Exist!";
}

TestAnything::TestAnything(QWidget *parent)
	: QMainWindow(parent),_pool(this),_scene(this)
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
		if (_nodes.size() > 0)
		{
			auto node = _nodes[0]->GetGui()->GetItem();
			const_cast<GuiGraphItemNode*>(node)->Refresh();
		}
	});
	void (TestAnything::*pAddConnection)(GuiGraphItemVertex*, GuiGraphItemVertex*) = &TestAnything::AddConnection;//注意这里要这样写来区分重载函数
	connect(&_scene, &GraphScene::sig_ConnectionAdded, this, pAddConnection);
	connect(&_scene, &GraphScene::sig_RemoveItems, this, &TestAnything::slot_RemoveItems);
}
TestAnything::~TestAnything()
{
	for (auto n : _nodes)
		delete n;
	_nodes.clear();
}

AlgGraphNode& TestAnything::AddNode(AlgGraphNode&node, GuiGraphController*guiNode /*= nullptr*/, QPointF center /*= QPointF(0, 0)*/)
{
	//AlgGraphNode初始化
	assert(_nodes.contains(&node) == false);
	
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

	if(node.objectName().isEmpty()==true)
		node.SetName(node.metaObject()->className()+QString::number(_nodes.size()));//TODO:自动命名	

	return node;
}
GuiGraphController* TestAnything::AddGuiNode(AlgGraphNode&node, GuiGraphController*guiNode, QPointF center /*= QPointF(0, 0)*/)
{
	assert(&node == &guiNode->_node);
	node.AttachGui(guiNode);
	connect(guiNode, &GuiGraphController::sig_Destroyed, &node, &AlgGraphNode::DetachGui, Qt::DirectConnection);//GUI自动解除
	auto guiItem = guiNode->InitApperance(center);
	return guiNode;
}

void TestAnything::RemoveNode(AlgGraphNode*node)
{	
	_nodes.removeOne(node);
	delete node;	
}
void TestAnything::RemoveVertex(AlgGraphVertex*vertex)
{
	delete vertex;
}

void TestAnything::AddConnection(GuiGraphItemVertex*srcVertexItem, GuiGraphItemVertex*dstVertexItem)
{
	auto srcVertex = const_cast<AlgGraphVertex*>(&srcVertexItem->_vertex), dstVertex = const_cast<AlgGraphVertex*>(&dstVertexItem->_vertex);
	bool b = srcVertexItem != dstVertexItem //不允许指向同一个
		&& srcVertex->connectedVertexes.contains(dstVertex) == false//不允许重复的连接
		&& dstVertex->connectedVertexes.contains(srcVertex) == false;
	assert(b);
	if (!b)
		return;
	srcVertex->Connect(dstVertex);
	connect(srcVertex, &AlgGraphVertex::sig_Activated, dstVertex, &AlgGraphVertex::Activate);//连接激活信号

	b = srcVertex->node.GetGui() != nullptr&&dstVertex->node.GetGui() != nullptr;//检查是否有GUI
	assert(b);
	if (!b)
		return;
	auto arrow = new GuiGraphItemArrow(srcVertexItem, dstVertexItem);
	arrow->setZValue(srcVertexItem->zValue() - 0.2);
	srcVertexItem->_nodeItem.controller.AddArrow(arrow);
	_scene.addItem(arrow);
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
			RemoveVertex(const_cast<AlgGraphVertex*>(&(qgraphicsitem_cast<GuiGraphItemVertex*>(item))->_vertex));//TODO:以后改成右键菜单删除
			break;
		case GuiGraphItemArrow::Type:
			delete qgraphicsitem_cast<GuiGraphItemArrow*>(item);
			break;
		default:
			break;
		}
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
	emit sig_Destroyed(this);
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

}

AlgGraphVertex* AlgGraphNode::AddVertex(QString name, QVariant defaultValue, bool isInput)
{
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
	}
	else
	{
		pv = new AlgGraphVertex_Output(*this, newName);
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

void AlgGraphNode::RemoveVertex(QString name, bool isInput)
{
	auto &vtxs = isInput ? _inputVertex : _outputVertex;
	assert(vtxs.contains(name) == true);
	auto vtx = vtxs[name];
	vtxs.remove(name);
	delete vtx;
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

void AlgGraphNode::SetName(QString name)
{
	setObjectName(name); 
	if (_gui != nullptr)
		_gui->setObjectName(name);
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

#pragma region GuiGraphNode
GuiGraphController::GuiGraphController(const AlgGraphNode&node, GraphScene&scene)
	:QObject(const_cast<AlgGraphNode*>(&node)), _node(node),_scene(scene)
{
	setObjectName(_node.objectName());
	connect(&node, &AlgGraphNode::objectNameChanged, this, [this](QString name)
	{
		setObjectName(name);
		if (_nodeItem != nullptr)
			_nodeItem->SetTitle(name);
	});
}

GuiGraphController::~GuiGraphController()
{
	qDebug() << _node.objectName() << __FUNCTION__;
	emit sig_Destroyed(this);
	if (_nodeItem != nullptr)
		delete _nodeItem;
	if (_panel.isNull() == false)
		delete _panel;
}
QWidget* GuiGraphController::InitWidget(QWidget*parent)
{
	throw "Not Implement!";
// 	_panel = new QPlainTextEdit(parent);
// 	//connect(qobject_cast<QPlainTextEdit*>(_panel), &QPlainTextEdit::textChanged, this, &GuiGraphNode::sig_ValueChanged);
// // 		connect(this, &GuiGraphNode::destroyed, this, [this] 
// // 		{
// // 			_nodeItem->scene()->removeItem(_nodeItem); 
// // 			delete _nodeItem;
// // 		});
// 	return _panel;
}

GuiGraphItemArrow* GuiGraphController::AddArrow(GuiGraphItemArrow*arrow)
{
	assert(arrow != nullptr);
	if (arrow == nullptr)
		return arrow;
	//auto vtxItem = (&arrow->srcVertex->_nodeItem.controller == this) ? (&arrow->srcVertex->_nodeItem.controller);
	arrow->srcVertex->_arrows.append(arrow);
	arrow->dstVertex->_arrows.append(arrow);
	connect(&arrow->srcVertex->_vertex, &AlgGraphVertex::sig_ConnectionRemoved, [](AlgGraphVertex*src, AlgGraphVertex*dst)
	{
		auto srcItem = GetVertexAlg2Gui(*src), dstItem = GetVertexAlg2Gui(*dst);
		for (auto a : srcItem->_arrows)
		{
			if (a->dstVertex == dstItem)
			{
				delete a;
				break;
			}
		}
	});
	return arrow;
}

#pragma endregion GuiGraphNode
#pragma region GuiGraphItem
GuiGraphItemNode* GuiGraphController::InitApperance(QPointF center /*= QPointF(0, 0)*/, QRectF size /*= QRectF(0, 0, 100, 100)*/)
{
	if (_nodeItem != nullptr)
	{
		delete _nodeItem;
		_nodeItem = nullptr;
	}
	_nodeItem = new GuiGraphItemNode(QRectF(0, 0, 100, 100), nullptr, *this);
	for (auto vtx : _node.GetVertexes(true))
	{
		auto vtxItem = new GuiGraphItemVertex(*_nodeItem, *vtx, true);
		vtx->gui = vtxItem;
		connect(vtx, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			delete _nodeItem->_inputVertexItem.take(vertex);
			_nodeItem->Refresh();
		});
		_nodeItem->_inputVertexItem.insert(vtx, vtxItem);
	}
	for (auto vtx : _node.GetVertexes(false))
	{
		auto vtxItem = new GuiGraphItemVertex(*_nodeItem, *vtx, false);
		vtx->gui = vtxItem;
		connect(vtx, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			delete _nodeItem->_outputVertexItem.take(vertex);
			_nodeItem->Refresh();
		});
		_nodeItem->_outputVertexItem.insert(vtx, vtxItem);
	}
	_nodeItem->Refresh();

	_scene.addItem(_nodeItem);
	return _nodeItem;
}

GuiGraphItemNode::~GuiGraphItemNode()
{
	qDebug() << __FUNCTION__;
	controller.DetachItem();
}

void GuiGraphItemNode::Refresh()
{
	SetTitle(controller.objectName());
	int h = _title.boundingRect().height();
	for (auto vtxItem : _inputVertexItem)
	{
		vtxItem->setPos(0, h);
		h += vtxItem->boundingRect().height();
	}
	h = _title.boundingRect().height();
	for (auto vtxItem : _outputVertexItem)
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
			if (/*item != nullptr && item->type() == GuiGraphItemVertex::Type*/qgraphicsitem_cast<GuiGraphItemVertex*>(item) != nullptr)
			{
				auto nodeItem = qgraphicsitem_cast<GuiGraphItemVertex*>(item);
				auto pos = nodeItem->mapToScene(nodeItem->boundingRect().center());
				
				arrow = new GuiGraphItemArrow(nodeItem, nullptr);
				arrow->setLine(QLineF(pos, pos));
				connect(&nodeItem->_vertex, &AlgGraphVertex::sig_Destroyed, this, [this]
				{
					const_cast<GuiGraphItemVertex*>(arrow->srcVertex) = nullptr;
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

GuiGraphItemVertex::GuiGraphItemVertex(GuiGraphItemNode&parent, const AlgGraphVertex&vertex, bool isInput) :QGraphicsItem(&parent), _vertex(vertex), _nodeItem(parent), _isInput(isInput)
{
	setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsScenePositionChanges);
	setAcceptHoverEvents(true);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setZValue(_nodeItem.zValue() + 0.1);
}

GuiGraphItemVertex::~GuiGraphItemVertex()
{
	qDebug() << _vertex.objectName() << __FUNCTION__;
	for (auto a : _arrows) 
	{
		if (scene() != nullptr)
			scene()->removeItem(a);
		delete a;
	}
}

QRectF GuiGraphItemVertex::boundingRect() const
{
	QFontMetrics fm(scene() != nullptr ? (scene()->font()) : (QApplication::font()));
	return QRectF(0, 0, fm.width(_vertex.objectName()), fm.height());
}
void GuiGraphItemVertex::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
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

void GuiGraphItemVertex::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	QMenu menu;
	auto paction = menu.addAction("Delete");
// 	if(menu.exec(event->screenPos())==paction)
// 		_nodeItem._holder.Re
	QGraphicsItem::contextMenuEvent(event);
}

GuiGraphItemArrow::GuiGraphItemArrow(GuiGraphItemVertex*const src, GuiGraphItemVertex*const dst)
	:srcVertex(src),dstVertex(dst)
{
	updatePosition();
}

GuiGraphItemArrow::~GuiGraphItemArrow()
{
	qDebug() << __FUNCTION__
		<< (srcVertex != nullptr ? srcVertex->_vertex.objectName() : "nullptr")
		<< (dstVertex != nullptr ? dstVertex->_vertex.objectName() : "nullptr");
	if (srcVertex != nullptr)
	{
		bool b= srcVertex->_arrows.removeOne(this);
		//assert(b == true);//TODO:警告
	}
	if (dstVertex != nullptr)
	{
		bool b = dstVertex->_arrows.removeOne(this);
		//assert(b == true);
	}
}

void GuiGraphItemArrow::updatePosition()
{
	QPointF p1 = (srcVertex!=nullptr) ? (srcVertex->mapToScene(srcVertex->ArrowAttachPosition())) : line().p1(),
		p2 = (dstVertex !=nullptr) ? (dstVertex->mapToScene(dstVertex->ArrowAttachPosition())) : line().p2();
	setLine(QLineF(p1, p2));
}

void AlgGraphVertex::Activate(QVariant var, bool isActivate /*= true*/)
{
// 	qDebug() << objectName() << __FUNCTION__;
// 	if (_result.isRunning() == false)//运行期间，阻塞输入
// 	{
// 		for (auto it = _inputVertex.begin(); it != _inputVertex.end();)//检查是否全部激活
// 		{
// 			if (it->isNull() == false)
// 			{
// 				if ((*it)->isActivated == false)
// 					return;
// 				++it;
// 			}
// 			else
// 			{
// 				qDebug() << __FUNCTION__ << it.key() << "Not exist";
// 				it = _inputVertex.erase(it);
// 			}
// 		}
// 		Run();
// 	}
}

void AlgGraphVertex::Connect(AlgGraphVertex*dstVertex)
{
	assert(dstVertex != nullptr);
	connectedVertexes.append(dstVertex);
	dstVertex->connectedVertexes.append(this);
	emit sig_ConnectionAdded(this, dstVertex);
}

void AlgGraphVertex::Disconnect(AlgGraphVertex*another)
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

