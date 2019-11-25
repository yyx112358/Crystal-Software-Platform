#include "stdafx.h"
#include "Controller.h"
#include <QMessageBox>
#include "GraphError.h"
#include "GuiNode.h"

Controller::Controller(QWidget *parent)
	: QMainWindow(parent), _pool(this), _scene(this), _factory(this)
{
	//ui初始化
	ui.setupUi(this);
	ui.graphicsView->setScene(&_scene);
	connect(&_scene, &GraphScene::sig_RemoveItems, this, &Controller::slot_RemoveItems, Qt::DirectConnection);
	//连接信号
	connect(ui.actionStart, &QAction::triggered, this, &Controller::slot_Start);
	connect(ui.actionDebug, &QAction::triggered, [this]
	{
		auto in1 = AddNode("Basic.Input", ""), in2 = AddNode("Basic.Input", ""), add1 = AddNode("", ""),
			out1=AddNode("Basic.Output",""), out2 = AddNode("Basic.Output", "");
		in1->ConnectVertex(AlgVertex::VertexType::OUTPUT, "out", add1, AlgVertex::VertexType::INPUT, "in");
		in2->ConnectVertex(AlgVertex::VertexType::OUTPUT, "out", add1, AlgVertex::VertexType::INPUT, "in_1");
		add1->ConnectVertex(AlgVertex::VertexType::OUTPUT, "out", out1, AlgVertex::VertexType::INPUT, "in");
		add1->ConnectVertex(AlgVertex::VertexType::OUTPUT, "out_1", out2, AlgVertex::VertexType::INPUT, "in");
// 		for (auto n : _nodes) 
// 		{
// 			n->dumpObjectInfo();
// 			n->dumpObjectTree();
// 		}
	});
	connect(ui.actionClear, &QAction::triggered, this, &Controller::Release);
	GraphWarning::connectInform(this, [this](QString msg) {statusBar()->showMessage(msg, 2000); /*qDebug() << "Graph Infrom[" << msg << "]";*/ });
	GraphWarning::connectWarning(this, [this](QString msg) {QMessageBox::warning(this, "Graph Warning", msg); });
	//调用工厂
	LoadFactory();
	//监视器
	_monitorTimerId = startTimer(100, Qt::TimerType::CoarseTimer);
	_refreshTimerId = startTimer(1000, Qt::TimerType::CoarseTimer);
	qDebug() << _factory.GetAlgNodeNames();
}

Controller::~Controller()
{
	try 
	{
		Release();
	}
	catch (...)//如果无法避免Release()抛出异常，至少要无条件吞掉异常
	{
		qDebug() << "Error in " __FUNCTION__;
		assert(0);
	}
}

void Controller::Release()
{
	_nodes.clear();
}

void Controller::LoadFactory()
{
	//TODO:其它工厂
	//按照title排序
	QList<AlgNode::FactoryInfo>infoList;
	for (auto n : _factory.GetAlgNodeTbl())
		infoList.append(n);
	std::sort(infoList.begin(), infoList.end(), [](AlgNode::FactoryInfo&a,
		AlgNode::FactoryInfo&b) {return a.title < b.title; });
	//生成对应按钮
	for (int i=0;i<infoList.size();i++)
	{
		auto &n = infoList[i];
		auto pbtn = new QPushButton(n.title, this);
		if (n.title.isEmpty() == true)//默认节点给一个默认名称
			pbtn->setText(QStringLiteral("基本"));
		pbtn->setObjectName(n.key);//objectName必须是key
		pbtn->setToolTip(n.description);
		ui.gridLayout_Node->addWidget(pbtn, i / 4, i % 4);
		connect(pbtn, &QPushButton::clicked, this, &Controller::slot_CreateNodeByButton);
	}
}



QSharedPointer<AlgNode> Controller::AddNode(QString nodeClassname, QString guiClassname /*= QString()*/)
{
	try
	{
		//生成AlgNode
		QSharedPointer<AlgNode> node = _factory.CreateAlgNode(nodeClassname);//生成失败会抛出异常
// 		connect(node.get(), &AlgNode::sig_Destroyed, this, [this](QWeakPointer<AlgNode>wp)//删除后自动从_nodes中删除
// 		{
// 			_nodes.removeOne(wp);
// 		}, Qt::DirectConnection);//注意不能连接QObject::destroyed信号，因为发出该信号时候派生类已经被析构了
		node->setObjectName(node->metaObject()->className() + QString::number(_nodes.size()));//简单重命名
		node->Init();

		//生成GuiNode
		if (guiClassname.isEmpty() == true || _factory.GetGuiNodeTbl().contains(guiClassname) == false)//优先级：guiClassname>GetGuiAdvice()>默认
		{
			auto oldname = guiClassname;
			guiClassname = node->GetGuiAdvice();
			if (_factory.GetGuiNodeTbl().contains(guiClassname) == false)
				guiClassname = "";
			if (oldname.isEmpty() == false)
				GRAPH_WARNING(QString("GUI [%1] not exist!\nUse [%2]").arg(oldname).arg(guiClassname));
		}
		QSharedPointer<GuiNode> gui = _factory.CreateGuiNode(guiClassname, node->sharedFromThis());//不会抛出异常，如果没有合适的，则生成一个基本GUI
		node->AttachGui(gui);
		//简单的移动，避免重叠
		static QPointF nextLocation = QPointF(0, 0);
		if (_scene.itemAt(QPointF(0, 0), QTransform()) == nullptr)
			nextLocation = QPointF(0, 0);
		else
			nextLocation += QPointF(10, 10);
		gui->InitApperance(nextLocation);

		connect(gui.data(), &GuiNode::sig_SendActionToController, this, &Controller::slot_ProcessGuiAction, Qt::DirectConnection);
		_scene.addItem(gui.get());

		auto panel = gui->InitWidget(ui.scrollAreaWidgetContents_Input);
		//TODO:分配不同种类
		if (panel.isNull() == false) 
		{
			//if(gui->objectName()==)
			ui.scrollAreaWidgetContents_Input->layout()->addWidget(panel.data());
		}

		_nodes.append(node);//放在最后，保证如果中间出现异常node可以被析构
		return node;
	}
	catch (GraphError&e)
	{
		QMessageBox::information(this, e.err, e.msg);
		return nullptr;
	}
}

void Controller::slot_CreateNodeByButton()
{	
	auto name = sender()->objectName();
	AddNode(name);
}

void Controller::slot_RemoveItems(QList<QGraphicsItem*>items)
{
	//原则上仅允许这里使用const_cast
	for (auto item : items)
	{
		switch (item->type())
		{
		case GuiNode::Type: 
		{
//			auto target = qgraphicsitem_cast<GuiNode*>(item)->algnode;
// 			for(auto it=_nodes.begin();it!=_nodes.end();++it)
// 				if (it->data() == target.data())
// 				{
// 					_nodes.erase(it);
// 					break;
// 				}
			_nodes.removeAll(const_cast<AlgNode*>(qgraphicsitem_cast<GuiNode*>(item)->algnode.data())->sharedFromThis());
			break;
		}
		case GuiVertex::Type:
		{
  			auto guiVtx = qgraphicsitem_cast<GuiVertex*>(item);
			auto algVtx = guiVtx->algVertex.lock();
			algVtx.constCast<AlgVertex>()->RemoveFromParent();
// 			auto algNode = guiVtx->guiNode.lock()->algnode.lock();
// 			algNode.constCast<AlgNode>()->RemoveVertex(algVtx->type, algVtx->objectName());
			break;
		}
		default:
			break;
		}
	}
}

void Controller::slot_Start()
{
	try
	{
		for (auto &n : _nodes)
		{
			n->Reset();
			n->Activate();
		}
	}
	catch (GraphError&e)
	{
		QMessageBox::information(this, e.err, e.msg);
	}
}

void Controller::slot_ProcessGuiAction(QWeakPointer<GuiNode>wp, QString action, bool isChecked)
{
	auto guiNode = wp.lock();
	switch (guiNode->type())
	{
	case GuiNode::Type:
		if (action == "delete")
			//_nodes.removeAll(guiNode->algnode);
			slot_RemoveItems({ guiNode.data() });
		else
			GRAPH_NOT_IMPLEMENT;
		break;
	case GuiVertex::Type:
		if (action == "delete")
		{	/*_nodes.removeAll(guiNode->algnode);*/}
		else
			GRAPH_NOT_IMPLEMENT;
		break;
	default:
		break;
	}
}

void Controller::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _monitorTimerId)//用来监视避免内存泄露的
	{
		ui.lcdNumber_AlgNode->display(static_cast<int> (AlgNode::GetAmount()));
 		ui.lcdNumber_AlgVertex->display(static_cast<int> (AlgVertex::GetAmount()));
 		ui.lcdNumber_GuiNode->display(static_cast<int> (GuiNode::GetAmount()));
 		ui.lcdNumber_GuiVertex->display(static_cast<int> (GuiVertex::GetAmount()));
// 		ui.lcdNumber_GuiItemArrow->display(static_cast<int> (GuiGraphItemArrow::GetAmount()));
 		ui.lcdNumber->display(static_cast<int> (_nodes.size()));
	}
	if (event->timerId() == _refreshTimerId)
	{
/*
		static int it = 0; 
		if (it % 2 == 0)
		{
			for (auto i = 0; i < 50; i++)
				AddNode("");
		}
		else
			Release();
		it++;*/
	}
}

// class Base
// {
// public:
// 	struct bigdata {};
// 	QSharedDataPointer<bigdata>copydata=GetDefault();
// private:
// 	virtual QSharedDataPointer<bigdata>GetDefault()const
// 	{
// 		static QSharedDataPointer<bigdata> data;
// 		return data;
// 	}
// };

QtPrivate::GraphWarning_StaticHandler GraphWarning::handler;