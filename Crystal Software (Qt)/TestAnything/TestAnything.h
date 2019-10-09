/*!
 * \file TestAnything.h
 * \date 2019/10/08 11:10
 *
 * \author Yyx112358
 * Contact: user@company.com
 *
 * \brief 
 *
 * TODO: long description
 *
 * \note
*/
#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TestAnything.h"
#include <QtConcurrent>
#include <QVariant>
#include <QDebug>
#include <QTime>

class VertexInfo;
class AlgGraphNode;

class TestAnything : public QMainWindow
{
	Q_OBJECT

public:
	TestAnything(QWidget *parent = Q_NULLPTR);

	void slot_Start(bool b);

	QHash<QString,AlgGraphNode*>nodes;
	QThreadPool pool;
private:
	Ui::TestAnythingClass ui;
};

//TODO:InputVertex,OutputVertex,TriggerVertex,EnableVertex
class VertexInfo
{
public:
	QVariant param;
	QVariant defaultValue;
	
	QMap<QString, QVariant>additionInfo;
	//QString name;
	//QString description;
	std::function<bool(const VertexInfo&)>*assertFunction;

	QList<VertexInfo*>connectedVertexs;

	bool isActivated = false;
	bool isEnabled = true;

	//TODO:一些方便用的函数和转移语义
	//VertexInfo&operator=(VertexInfo&&);
	void Reset();
	void Release();
	void Load(QVariant var);
	//virtual bool _AssertFunction(QVariant var);
};

//TODO:派生：基本算法、常量、外部输入、外部输出、条件、while、逻辑运算、延时、循环、缓冲
class AlgGraphNode
	:public QObject//, public Interface_Alg
{
	Q_OBJECT
public:
	AlgGraphNode(QObject*parent, QThreadPool&pool) :QObject(parent), _pool(pool) 
	{
		sizeof(VertexInfo);
		connect(&_result, &QFutureWatcher<void>::finished, this, &AlgGraphNode::Output);
	}
	void AddVertex(QString name,QVariant defaultValue,bool isInput)
	{
		assert(name.isEmpty() == false);
		VertexInfo vtx;
		vtx.defaultValue = defaultValue;
		if (isInput)
			_inputVertex.insert(name, vtx);
		else
			_outputVertex.insert(name, vtx);
	}
	void AddVertex(QHash<QString, QVariant>initTbl, bool isInput);
	void Reset()
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
	void Release();
	friend void Connect(AlgGraphNode&srcNode,QString srcVertexName,AlgGraphNode&dstNode,QString dstVertexName)
	{
		//TODO:合法性检查
		assert(srcNode._outputVertex.contains(srcVertexName) && dstNode._inputVertex.contains(dstVertexName));
		void (AlgGraphNode::*pActivate)(QVariant, VertexInfo*, bool)=&AlgGraphNode::Activate;//注意这里要这样写来区分重载函数
		connect(&srcNode, &AlgGraphNode::sig_Activate, &dstNode, pActivate);
		VertexInfo&srcV = srcNode._outputVertex[srcVertexName], &dstV = dstNode._inputVertex[dstVertexName];
		srcV.connectedVertexs.append(&dstV);
		dstV.connectedVertexs.append(&srcV);
	}
	void Activate(QVariant var = QVariant(), QString vtxName = QString(), bool b = true)
	{
		if(vtxName.isEmpty()==false)
			Activate(var, &_inputVertex[vtxName], b);
		else
			Activate(var, nullptr, b);
	}
	void Activate(QVariant var,VertexInfo*vtx = nullptr, bool b = true)
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
			emit sig_NodeActivated();
			//TODO:暂停和退出
			_result.setFuture(QtConcurrent::run(&_pool, this, &AlgGraphNode::Run));
			//TODO:暂停和退出
			//emit sig_ResultReady();
		}
	}
	void Run(/*QMap<QString,QVariant>*/)
	{
		_Run();
	}

	void Output()
	{
		//TODO:加锁
		//qDebug() << "====Output====:" << QThread::currentThread();
		for (auto &v : _outputVertex)
		{
			for (auto cv : v.connectedVertexs)
				emit sig_Activate(v.param, cv, true);
			emit sig_Output(v.param,&v);
			v.param.clear();
		}
		emit sig_OutputFinished();
	}
	void Pause();
	void Stop();

	QStringList GetVertexNames()const;
	const VertexInfo* GetVertexInfo()const;
	void Create();
	QString name;

signals:
	void sig_Activate(QVariant var = QVariant(), VertexInfo*vtx = nullptr, bool b = true);
	void sig_VertexActivated(bool);
	void sig_NodeActivated();
	void sig_Output(QVariant var = QVariant(), VertexInfo*srcVertex = nullptr/*, bool b = true*/);
	void sig_OutputFinished();

	void sig_ReportProgress(float);
	void sig_ResultReady();
protected:
	virtual void _Run();

	bool _enable = true;
	
	QFutureWatcher<void>_result;
	QHash<QString, VertexInfo>_inputVertex;
	QHash<QString, VertexInfo>_outputVertex;

	QThreadPool&_pool;
};