#include "stdafx.h"
#include "Controller.h"
#include <QMessageBox>
#include "GraphError.h"


Controller::Controller(QWidget *parent)
	: QMainWindow(parent), _pool(this), _scene(this), _factory(this)
{
	ui.setupUi(this);

	connect(ui.actionStart, &QAction::triggered, this, &Controller::slot_Start);
	connect(ui.actionDebug, &QAction::triggered, [this]
	{
		QString s;
		for (auto n : _nodes) 
		{
			n->dumpObjectInfo();
			n->dumpObjectTree();
		}
	});

	int i = 0;
	for (auto n : _factory.GetAlgNodeTbl())
	{
		auto pbtn = new QPushButton(n.title, this);
		pbtn->setObjectName(n.key);
		ui.gridLayout_Node->addWidget(pbtn, i / 4, i % 4);
		connect(pbtn, &QPushButton::clicked, this,&Controller::slot_CreateNode);
		i++;
	}

	_monitorTimerId = startTimer(100, Qt::TimerType::CoarseTimer);
	qDebug() << _factory.GetAlgNodeNames();
}

Controller::~Controller()
{
	try 
	{
		Release();
	}
	catch (...)
	{
		qDebug() << "Error in " __FUNCTION__;
		assert(0);
	}
}

void Controller::Release()
{
	_nodes.clear();
}

void Controller::slot_CreateNode()
{
	//auto node = QSharedPointer<AlgNode>::create(); 		
	try
	{
		auto s = sender();
		auto name = qobject_cast<QPushButton*>(sender())->objectName();
		auto node = _factory.CreateAlgNode(name);
		_nodes.append(node);
		node->setObjectName(node->metaObject()->className() + QString::number(_nodes.size()));
	}
	catch (GraphError&e)
	{
		QMessageBox::information(this, e.err, e.msg);
	}
}

void Controller::slot_Start()
{
	try
	{
		GRAPH_NOT_IMPLEMENT;
	}
	catch (GraphError&e)
	{
		QMessageBox::information(this, e.err, e.msg);
	}
}

void Controller::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _monitorTimerId)//用来监视避免内存泄露的
	{
		ui.lcdNumber_AlgNode->display(static_cast<int> (AlgNode::GetAmount()));
 		ui.lcdNumber_AlgVertex->display(static_cast<int> (AlgVertex::GetAmount()));
// 		ui.lcdNumber_GuiController->display(static_cast<int> (GuiGraphController::GetAmount()));
// 		ui.lcdNumber_GuiNode->display(static_cast<int> (GuiNode::GetAmount()));
// 		ui.lcdNumber_GuiItemVertex->display(static_cast<int> (GuiGraphItemVertex::GetAmount()));
// 		ui.lcdNumber_GuiItemArrow->display(static_cast<int> (GuiGraphItemArrow::GetAmount()));
// 		ui.lcdNumber_Running->display(static_cast<int> (AlgGraphNode::GetRunningAmount()));
	}
}
