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

	QList<AlgGraphNode*>nodes;
private:
	Ui::TestAnythingClass ui;
};

//TODO:InputVertex,OutputVertex,TriggerVertex,EnableVertex
class VertexInfo
{
public:
	QVariant param;
	QVariant defaultValue;
	
	//QMap<QString, QVariant>additionInfo;
	//QString name;
	//QString description;
	//std::function<bool(VertexInfo&)>assertFunction;

	QList<VertexInfo*>connectedVertexs;

	bool isActivated = false;
	bool isEnabled = true;

	//VertexInfo&operator=(VertexInfo&&);
	void Reset();
	void Release();
};

//TODO:�����������㷨���������ⲿ���롢�ⲿ�����������while���߼����㡢��ʱ��ѭ��������
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
	void Init()
	{
		VertexInfo in1, out1;
		in1.defaultValue = "in1";
		out1.defaultValue = "out1";
		_inputVertex.insert("in1", in1);
		_outputVertex.insert("out1", out1);
	}
	void Reset()
	{
		for (auto &v : _inputVertex)
		{
			v.isActivated = false;
			v.isEnabled = true;
		}
		for (auto &v : _outputVertex)
		{
			v.isActivated = false;
			v.isEnabled = true;
		}
	}
	void Release();
	friend void Connect(AlgGraphNode&srcNode,QString srcVertexName,AlgGraphNode&dstNode,QString dstVertexName)
	{
		//TODO:�Ϸ��Լ��
		assert(srcNode._outputVertex.contains(srcVertexName) && dstNode._inputVertex.contains(dstVertexName));
		connect(&srcNode, &AlgGraphNode::sig_Activate, &dstNode, &AlgGraphNode::Activate);
		VertexInfo&srcV = srcNode._outputVertex[srcVertexName], &dstV = dstNode._inputVertex[dstVertexName];
		srcV.connectedVertexs.append(&dstV);
		dstV.connectedVertexs.append(&srcV);
	}
	void Activate(QVariant var=QVariant(),VertexInfo*vtx = nullptr, bool b = true)
	{
		//TODO:��Ҫ����
		if (vtx != nullptr)
		{
			if (vtx->isEnabled == false)//���ö��㣬����
				return;
			//TODO:�����ж�
			vtx->param = var;
			vtx->isActivated = b;
			emit sig_VertexActivated();
		}
		if (_result.isRunning() == false)//�����ڼ䣬��������
		{
			for (auto const &v : _inputVertex)//����Ƿ�ȫ������
			{
				if (v.isActivated == false)
					return;
			}
			//TODO:��ȡ����
			emit sig_NodeActivated();
			//TODO:��ͣ���˳�
			_result.setFuture(QtConcurrent::run(&_pool, this, &AlgGraphNode::Run));
			//TODO:��ͣ���˳�
			//emit sig_ResultReady();
		}
	}
	void Run(/*QMap<QString,QVariant>*/);
	void Output()
	{
		//TODO:����
		qDebug() << "====Output====:" << QThread::currentThread();
		for (auto &v : _outputVertex)
		{
			for (auto cv : v.connectedVertexs)
				emit sig_Activate(v.param, cv, true);
			emit sig_Output(v.param);
			v.param.clear();
		}
		emit sig_OutputFinished();
	}
	void Pause();
	void Stop();

	void GetVertexInfo()const;
	void Create();

signals:
	void sig_Activate(QVariant var = QVariant(), VertexInfo*vtx = nullptr, bool b = true);
	void sig_VertexActivated();
	void sig_NodeActivated();
	void sig_Output(QVariant var = QVariant()/*, VertexInfo*vtx = nullptr, bool b = true*/);
	void sig_OutputFinished();

	void sig_ReportProgress(float);
	void sig_ResultReady();
#ifndef _DEBUG
protected:
#endif // _DEBUG
	bool _enable = true;
	QFutureWatcher<void>_result;
	QMap<QString, VertexInfo>_inputVertex;
	QMap<QString, VertexInfo>_outputVertex;

	QThreadPool&_pool;
};