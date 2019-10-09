#include "TestAnything.h"

#include <QDebug>
#include <opencv.hpp>
#include <functional>
#include <vld.h>
using namespace cv;

TestAnything::TestAnything(QWidget *parent)
	: QMainWindow(parent),pool(this)
{
	ui.setupUi(this);

	connect(ui.actionStart, &QAction::triggered, this,&TestAnything::slot_Start);

	nodes["in1"]=new AlgGraphNode(this, pool);
	nodes["in1"]->name = "in1";
	nodes["in1"]->AddVertex("in1","in1",true);
	nodes["in1"]->AddVertex("out1", "out1", false);	
	
	nodes["in2"] = new AlgGraphNode(this, pool);
	nodes["in2"]->name = "in2"; 
	nodes["in2"]->AddVertex("in1", "in1", true);
	nodes["in2"]->AddVertex("out1", "out1", false);

	nodes["add1"] = new AlgGraphNode(this, pool);
	nodes["add1"]->name = "add1"; 
	nodes["add1"]->AddVertex("in1", "in1", true);
	nodes["add1"]->AddVertex("in2", "in2", true);
	nodes["add1"]->AddVertex("out1", "out1", false);

	nodes["out1"]=new AlgGraphNode(this, pool);
	nodes["out1"]->name = "out1"; 
	nodes["out1"]->AddVertex("in1", "in1", true);
	nodes["out1"]->AddVertex("out1", "out1", false);

	nodes["out2"] = new AlgGraphNode(this, pool);
	nodes["out2"]->name = "out2";
	nodes["out2"]->AddVertex("in1", "in1", true);
	nodes["out2"]->AddVertex("out1", "out1", false);

	Connect(*nodes["in1"], "out1", *nodes["add1"], "in1");
	Connect(*nodes["in2"], "out1", *nodes["add1"], "in2");
	Connect(*nodes["add1"], "out1", *nodes["out1"], "in1");
	Connect(*nodes["in1"], "out1", *nodes["out2"], "in1");

	connect(ui.plainTextEdit, &QPlainTextEdit::textChanged, this, [this] 
	{ 
		nodes["in1"]->Activate(ui.plainTextEdit->toPlainText(), "in1"); 
	});
	connect(ui.plainTextEdit_2, &QPlainTextEdit::textChanged, this, [this] 
	{ 	
		nodes["in2"]->Activate(ui.plainTextEdit_2->toPlainText(), "in1"); 
	});
	connect(nodes["out1"], &AlgGraphNode::sig_Output, this, [this](QVariant var)
	{
		ui.label->setText(var.toString());
	});
	connect(nodes["out2"], &AlgGraphNode::sig_Output, this, [this](QVariant var)
	{
		ui.label_2->setText(var.toString());
	});

}



void TestAnything::slot_Start(bool b)
{
	qDebug() << __FUNCSIG__;
// 	ui.plainTextEdit->setPlainText(ui.plainTextEdit->toPlainText()+__FUNCSIG__"\n");
// 	//QtConcurrent::run()
// 
	ui.label->setText("");
	ui.label_2->setText("");
	for (auto node : nodes) 
	{
		node->Reset();
		node->Activate();
	}
	

}

void AlgGraphNode::_Run()
{
 	qDebug() << "====Start====:" << QThread::currentThread() << QTime::currentTime();
	for (auto &v : _inputVertex)
		qDebug() << "<in>" << name << ':' << v.param;
	QThread::msleep(500);

	QVariant test = __FUNCSIG__;
	_outputVertex["out1"].param = "";
	for (auto const&v : _inputVertex)
		_outputVertex["out1"].param = _outputVertex["out1"].param.toString() + v.param.toString();

	for (auto &v : _outputVertex)
		qDebug() << "<out>" << name << ':' << v.param;
 	qDebug() << "End:" << QThread::currentThread() << QTime::currentTime();
}
