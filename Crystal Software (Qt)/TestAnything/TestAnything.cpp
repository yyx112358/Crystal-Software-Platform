#include "TestAnything.h"

#include <QDebug>
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
		AddNode(new AlgGraphNode_Input(), QPointF(0, 0));
	});
	connect(ui.actionStart, &QAction::triggered, this, &TestAnything::slot_Start);
	sizeof(AlgGraphVertex);
	sizeof(AlgGraphNode);
	
}

void TestAnything::AddNode(const AlgGraphNode*node, QPointF center)
{

}

void TestAnything::slot_Start(bool b)
{
	qDebug() << __FUNCSIG__;
	ui.label->setText("");
	ui.label_2->setText("");
}

#if 0
TestAnything::TestAnything(QWidget *parent)
	: QMainWindow(parent),pool(this)
{
	ui.setupUi(this);

	connect(ui.actionStart, &QAction::triggered, this,&TestAnything::slot_Start);

	auto scene = new GraphScene(this);
	ui.graphicsView->setScene(&scene->scene);
	
	nodes["in1"]=new AlgGraphNode(this, pool);
	nodes["in1"]->setObjectName("in1");
	nodes["in1"]->AddVertex("in1","in1",true);
	nodes["in1"]->AddVertex("out1", "out1", false);	
	scene->AddNode(nodes["in1"], 0, 0, 50, 50);
	
	nodes["in2"] = new AlgGraphNode(this, pool);
	nodes["in2"]->setObjectName("in2");
	nodes["in2"]->AddVertex("in1", "in1", true);
	nodes["in2"]->AddVertex("out1", "out1", false);
	scene->AddNode(nodes["in2"], 0, 100, 50, 50);

	nodes["add1"] = new AlgGraphNode(this, pool);
	nodes["add1"]->setObjectName("add1");
	nodes["add1"]->AddVertex("in1", "in1", true);
	nodes["add1"]->AddVertex("in2", "in2", true);
	nodes["add1"]->AddVertex("out1", "out1", false);
	scene->AddNode(nodes["add1"], 100, 50, 50, 50);

	nodes["out1"]=new AlgGraphNode(this, pool);
	nodes["out1"]->setObjectName("out1");
	nodes["out1"]->AddVertex("in1", "in1", true);
	nodes["out1"]->AddVertex("out1", "out1", false);
	scene->AddNode(nodes["out1"], 200, 50, 50, 50);

	nodes["out2"] = new AlgGraphNode(this, pool);
	nodes["out2"]->setObjectName("out2");
	nodes["out2"]->AddVertex("in1", "in1", true);
	nodes["out2"]->AddVertex("out1", "out1", false);
	scene->AddNode(nodes["out2"], 200, 0, 50, 50);

	ConnectVertex(*nodes["in1"], "out1", *nodes["add1"], "in1");
	ConnectVertex(*nodes["in2"], "out1", *nodes["add1"], "in2");
	ConnectVertex(*nodes["add1"], "out1", *nodes["out1"], "in1");
	ConnectVertex(*nodes["in1"], "out1", *nodes["out2"], "in1");

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

AlgGraphNode::AlgGraphNode(QObject*parent, QThreadPool&pool) :QObject(parent), _pool(pool)
{
	sizeof(VertexInfo);
	connect(&_result, &QFutureWatcher<void>::finished, this, &AlgGraphNode::Output);
	connect(&_result, &QFutureWatcher<void>::finished, this, [this] {emit sig_TaskFinished(this); });
}

AlgGraphNode::~AlgGraphNode()
{
	qDebug() << __FUNCSIG__;
}

void AlgGraphNode::AddVertex(QString name, QVariant defaultValue, bool isInput)
{
	assert(name.isEmpty() == false);
	VertexInfo vtx;
	vtx.defaultValue = defaultValue;
	auto &group = (isInput) ? (_inputVertex) : (_outputVertex);
	group.insert(name, vtx);
	emit sig_VertexAdded(&group[name], isInput);
}

void AlgGraphNode::Reset()
{
	for (auto &v : _inputVertex)
	{
		v.isActivated = false;
		v.isEnabled = true;
		v.param.clear();
	}
	for (auto &v : _outputVertex)
	{
		v.isActivated = false;
		v.isEnabled = true;
		v.param.clear();
	}
}

void AlgGraphNode::Release()
{
	_inputVertex.clear();
	_outputVertex.clear();
}

void AlgGraphNode::Activate(QVariant var, VertexInfo*vtx / *= nullptr* /, bool b / *= true* /)
{
	//TODO:需要加锁
	if (vtx != nullptr)
	{
		if (vtx->isEnabled == false)//禁用顶点，阻塞
			return;
		//TODO:类型判断
		vtx->param = var;
		vtx->isActivated = b;
		emit sig_VertexActivated(b);
	}
	if (_result.isRunning() == false)//运行期间，阻塞输入
	{
		for (auto const &v : _inputVertex)//检查是否全部激活
		{
			if (v.isActivated == false)
				return;
		}
		//TODO:读取输入
		emit sig_NodeActivated(this);
		//TODO:暂停和退出
		_result.setFuture(QtConcurrent::run(&_pool, this, &AlgGraphNode::Run));
		//TODO:暂停和退出
		//emit sig_ResultReady();
	}
}

void AlgGraphNode::Activate(QVariant var / *= QVariant()* /, QString vtxName / *= QString()* /, bool b / *= true* /)
{
	if (vtxName.isEmpty() == false)
		Activate(var, &_inputVertex[vtxName], b);
	else
		Activate(var, nullptr, b);
}

void AlgGraphNode::Run(/ *QMap<QString,QVariant>* /)
{
	_Run();
}

void AlgGraphNode::Output()
{
	//TODO:加锁
	//qDebug() << "====Output====:" << QThread::currentThread();
	for (auto &v : _outputVertex)
	{
		for (auto cv : v.connectedVertexs)
			emit sig_Activate(v.param, cv, true);
		emit sig_Output(v.param, &v);
		v.param.clear();
	}
	emit sig_OutputFinished();
}

void AlgGraphNode::_Run()
{
 	qDebug() << "====Start====:" << QThread::currentThread() << QTime::currentTime();
	for (auto &v : _inputVertex)
		qDebug() << "<in>" << objectName() << ':' << v.param;
	QThread::msleep(500);

	QVariant test = __FUNCSIG__;
	_outputVertex["out1"].param = "";
	for (auto const&v : _inputVertex)
		_outputVertex["out1"].param = _outputVertex["out1"].param.toString() + v.param.toString();

	for (auto &v : _outputVertex)
		qDebug() << "<out>" << objectName() << ':' << v.param;
 	qDebug() << "End:" << QThread::currentThread() << QTime::currentTime();
}
#endif

