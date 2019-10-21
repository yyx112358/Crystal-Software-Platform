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
		node->setObjectName(QString("input") + QString::number(_nodes.size()));

		AddNode(*node, new GuiGraphNode(*node, _scene), QPointF(-100, 0));
	});
	connect(ui.pushButton_7, &QPushButton::clicked, this, [this](bool b)
	{
		if (_nodes.size() > 0)
		{
			auto node = _nodes[0];
			delete node;
		}
	});
	connect(ui.pushButton_5, &QPushButton::clicked, this, [this](bool b)
	{
		_scene.clear();
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
	return node;
}

GuiGraphNode* TestAnything::AddGuiNode(AlgGraphNode&node, GuiGraphNode*guiNode, QPointF center /*= QPointF(0, 0)*/)
{
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

void TestAnything::slot_RemoveItems(QList<QGraphicsItem*>items)
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

}
void AlgGraphNode::Reset()
{

}
void AlgGraphNode::Release()
{

}

AlgGraphVertex* AlgGraphNode::AddVertex(QString name, QVariant defaultValue, bool isInput)
{
	throw __FUNCTION__"Not Implement!";
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

#pragma region GuiGraphNode
GuiGraphNode::GuiGraphNode(const AlgGraphNode&node, GraphScene&scene)
	:QObject(const_cast<AlgGraphNode*>(&node)), _node(node),_scene(scene)
{
	setObjectName(_node.objectName());
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
	_panel = new QPlainTextEdit(parent);
	//connect(qobject_cast<QPlainTextEdit*>(_panel), &QPlainTextEdit::textChanged, this, &GuiGraphNode::sig_ValueChanged);
// 		connect(this, &GuiGraphNode::destroyed, this, [this] 
// 		{
// 			_nodeItem->scene()->removeItem(_nodeItem); 
// 			delete _nodeItem;
// 		});
	return _panel;
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
	_nodeItem->InitApperance();	

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

void GuiGraphItemNode::InitApperance()
{
	auto box = boundingRect();
	QGraphicsTextItem*title = new QGraphicsTextItem(this);
	title->setPlainText(_holder.objectName());
	title->setPos(title->mapFromParent(box.width() / 2 - title->boundingRect().width() / 2, 0));
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
