#include "TestAnything.h"

#include <QDebug>
#include <opencv.hpp>
#include <functional>
#include <vld.h>
using namespace cv;

TestAnything::TestAnything(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.actionStart, &QAction::triggered, this,&TestAnything::slot_Start);

	nodes.append(new AlgGraphNode(this, *new QThreadPool(this)));
	nodes.append(new AlgGraphNode(this, *new QThreadPool(this)));
	nodes[0]->Init("in1","in1",true);
	nodes[0]->Init("out1", "out1", false);
	nodes[1]->Init("in1", "in1", true);
	nodes[1]->Init("out1", "out1", false);
	Connect(*nodes[0], "out1", *nodes[1], "in1");
	connect(nodes[1], &AlgGraphNode::sig_Output, this, [this](QVariant var)
	{
		ui.label->setText(var.toString());
	});
	
}



void TestAnything::slot_Start(bool b)
{
	qDebug() << std::is_standard_layout<VertexInfo>::value;
// 	qDebug() << __FUNCSIG__;
// 	ui.plainTextEdit->setPlainText(ui.plainTextEdit->toPlainText()+__FUNCSIG__"\n");
// 	//QtConcurrent::run()
// 
	for(auto node:nodes)
		node->Activate();
	nodes[0]->Activate(ui.plainTextEdit->toPlainText(), "in1");
}

void AlgGraphNode::Run(/*QMap<QString,QVariant>*/)
{

	qDebug() << "====Start====:" << QThread::currentThread() << QTime::currentTime();
	for (auto &v : _inputVertex)
		qDebug() << v.param;
	QThread::msleep(500);

	QVariant test = __FUNCSIG__;
	_outputVertex["out1"].param = _inputVertex["in1"].param.toString()+"4";
	for (auto &v : _outputVertex)
		qDebug() << v.param;
	qDebug() << "End:" << QThread::currentThread() << QTime::currentTime();
}
