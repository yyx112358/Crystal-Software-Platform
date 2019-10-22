#include "TestAnything.h"
#include <QPlainTextEdit>
#include <QGraphicsSceneMouseEvent>

#include <qDebug>
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
		auto node = new AlgGraphNode(this, _pool);
		AddNode(*node, new GuiGraphNode(*node, _scene), QPointF(-100, 0));
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

	connect(&_scene, &GraphScene::sig_RemoveItems, this, &TestAnything::slot_RemoveItems);
}
TestAnything::~TestAnything()
{
	for (auto n : _nodes)
		delete n;
	_nodes.clear();
}

AlgGraphNode& TestAnything::AddNode(AlgGraphNode&node, GuiGraphNode*guiNode /*= nullptr*/, QPointF center /*= QPointF(0, 0)*/)
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
GuiGraphNode* TestAnything::AddGuiNode(AlgGraphNode&node, GuiGraphNode*guiNode, QPointF center /*= QPointF(0, 0)*/)
{
	assert(&node == &guiNode->_node);
	node.AttachGui(guiNode);
	connect(guiNode, &GuiGraphNode::sig_Destroyed, &node, &AlgGraphNode::DetachGui, Qt::DirectConnection);//GUI自动解除
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

void TestAnything::slot_RemoveItems(QList<QGraphicsItem*>items)
{
	for (auto item : items)
	{
		switch (item->type())
		{
		case GuiGraphItemNode::Type:
			RemoveNode(const_cast<AlgGraphNode*> (&((qgraphicsitem_cast<GuiGraphItemNode*>(item))->_holder._node)));
			break;
		case GuiGraphItemVertex::Type:
			RemoveVertex(const_cast<AlgGraphVertex*>(&(qgraphicsitem_cast<GuiGraphItemVertex*>(item))->_vertex));//TODO:以后改成右键菜单删除
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
GuiGraphNode::GuiGraphNode(const AlgGraphNode&node, GraphScene&scene)
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

GuiGraphNode::~GuiGraphNode()
{
	qDebug() << _node.objectName() << __FUNCTION__;
	emit sig_Destroyed(this);
	if (_nodeItem != nullptr)
		delete _nodeItem;
	if (_panel.isNull() == false)
		delete _panel;
}
QWidget* GuiGraphNode::InitWidget(QWidget*parent)
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
#pragma endregion GuiGraphNode
#pragma region GuiGraphItem
GuiGraphItemNode* GuiGraphNode::InitApperance(QPointF center /*= QPointF(0, 0)*/, QRectF size /*= QRectF(0, 0, 100, 100)*/)
{
	if (_nodeItem != nullptr)
	{
		delete _nodeItem;
		_nodeItem = nullptr;
	}
	_nodeItem = new GuiGraphItemNode(QRectF(0, 0, 100, 100), nullptr, *this);
	for (auto vtx : _node.GetVertexes(true)) 
	{
		auto vtxItem = new GuiGraphItemVertex(*_nodeItem, *vtx,true);
		connect(vtx, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			delete _nodeItem->_inputVertexItem.take(vertex);
			_nodeItem->Refresh();//TODO:重设后变混乱，需要研究
		});
		_nodeItem->_inputVertexItem.insert(vtx, vtxItem);
	}
	for (auto vtx : _node.GetVertexes(false))
	{
		auto vtxItem = new GuiGraphItemVertex(*_nodeItem, *vtx, false);
		connect(vtx, &AlgGraphVertex::sig_Destroyed, this, [this](AlgGraphNode*node, AlgGraphVertex*vertex)
		{
			delete _nodeItem->_outputVertexItem.take(vertex);
			_nodeItem->Refresh();
		});
		_nodeItem->_outputVertexItem.insert(vtx, vtxItem);
	}
	_nodeItem->Refresh();

// 	int h = title->boundingRect().height();
// 	for (auto vtx : _node.GetVertexes(true))
// 	{
// 		QString name = vtx->objectName();
// 		GuiGraphItemVertex*pnode = new GuiGraphItemVertex(_nodeItem, *vtx);
// 		pnode->setPos(pnode->mapFromParent(0, h));
// 		h += pnode->boundingRect().height();
// 		pnode->setAcceptHoverEvents(true);
// 		pnode->setZValue(_nodeItem->zValue() + 0.1);
// 
// 		vtx->gui = pnode;
// 		_inputVertex.insert(name, vtx);
// 		_inputVertexItem.insert(vtx, pnode);
// 	}
// 	h = title->boundingRect().height();
// 	for (auto vtx : _node.GetVertexes(false))
// 	{
// 		QString name = vtx->objectName();
// 		GuiGraphItemVertex*pnode = new GuiGraphItemVertex(_nodeItem, *vtx);
// 		pnode->setPos(pnode->mapFromParent(box.width() - pnode->boundingRect().width(), h));
// 		h += pnode->boundingRect().height();
// 		pnode->setAcceptHoverEvents(true);
// 		pnode->setZValue(_nodeItem->zValue() + 0.1);
// 
// 		vtx->gui = pnode;
// 		_inputVertex.insert(name, vtx);
// 		_inputVertexItem.insert(vtx, pnode);
// 	}

	_scene.addItem(_nodeItem);
	return _nodeItem;
}

GuiGraphItemNode::~GuiGraphItemNode()
{
	qDebug() << __FUNCTION__;
	_holder.DetachItem();
}

void GuiGraphItemNode::Refresh()
{
	SetTitle(_holder.objectName());
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
