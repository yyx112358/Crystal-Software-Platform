#include "stdafx.h"
#include "Controller.h"
#include <QMessageBox>
#include "GraphError.h"

Controller::Controller(QWidget *parent)
	: QMainWindow(parent),_pool(this),_scene(this)
{
	ui.setupUi(this);

	connect(ui.actionStart, &QAction::triggered, this, &Controller::slot_Start);
	connect(ui.pushButton_Input, &QPushButton::clicked, [this]
	{
		auto node = QSharedPointer<AlgNode>::create();
		_nodes.append(node);
	});
	_monitorTimerId = startTimer(100, Qt::TimerType::CoarseTimer);
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
	_nodeSearchTbl.clear();
	_nodes.clear();
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
		ui.lcdNumber_Node->display(static_cast<int> (AlgNode::GetAmount()));
// 		ui.lcdNumber_AlgVertex->display(static_cast<int> (AlgGraphVertex::GetAmount()));
// 		ui.lcdNumber_GuiController->display(static_cast<int> (GuiGraphController::GetAmount()));
// 		ui.lcdNumber_GuiItemNode->display(static_cast<int> (GuiGraphItemNode::GetAmount()));
// 		ui.lcdNumber_GuiItemVertex->display(static_cast<int> (GuiGraphItemVertex::GetAmount()));
// 		ui.lcdNumber_GuiItemArrow->display(static_cast<int> (GuiGraphItemArrow::GetAmount()));
// 		ui.lcdNumber_Running->display(static_cast<int> (AlgGraphNode::GetRunningAmount()));
	}
}
