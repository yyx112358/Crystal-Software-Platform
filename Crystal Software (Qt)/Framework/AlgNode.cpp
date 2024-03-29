#include "stdafx.h"
#include "AlgNode.h"
#include "AlgVertex.h"
#include "GuiNode.h"
#include <QtConcurrent>
#include <QMessageBox>
#include <QInputDialog>

std::atomic_uint64_t AlgNode::_amount = 0;
std::atomic_uint64_t AlgNode::_runningAmount = 0;

AlgNode::AlgNode(QThreadPool&pool, QObject*parent)
	:QObject(parent),_pool(pool)
{
	_amount++;
	qDebug() << objectName() << __FUNCTION__;
	GRAPH_INFORM(objectName() + __FUNCTION__);
#ifdef _DEBUG
	connect(this, &AlgNode::objectNameChanged, [this](QString str) {__debugName = str; });
#endif // _DEBUG
	connect(&_resultWatcher, &QFutureWatcher<QVariantHash>::finished, this, &AlgNode::Output);
}

AlgNode::~AlgNode()
{
	qDebug() << objectName() << __FUNCTION__;
	emit sig_Destroyed(sharedFromThis());
	try
	{
		_resultWatcher.waitForFinished();
		_gui.clear();
		_inputVertex.clear();
		_outputVertex.clear();
	}
	catch (...)
	{
		qDebug() << "Error in " __FUNCTION__;
		assert(0);
	}
	--_amount;
}
QSharedPointer<AlgNode> AlgNode::Clone()const
{
	GRAPH_NOT_IMPLEMENT;
}
void AlgNode::Init()
{
	AddVertexAuto(AlgVertex::VertexType::INPUT, "in");
	AddVertexAuto(AlgVertex::VertexType::INPUT, "in");
	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "out");
	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "out");
}

void AlgNode::Reset()
{
	if (_isRunning == true) 
	{
		Stop(true);
// 		blockSignals(true);
// 		_resultWatcher.waitForFinished();
// 		blockSignals(false);
		QSharedPointer<QMessageBox>pmsgBox;
		while (_isRunning == true)
		{
			if (pmsgBox == nullptr) 
			{
				pmsgBox = QSharedPointer<QMessageBox>::create(QMessageBox::Information, QStringLiteral(""), "");
				pmsgBox->show();
			}
			pmsgBox->setText(QStringLiteral("正在停止节点……\n剩余:") + QString::number(GetRunningAmount()));
			QApplication::processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
			QThread::msleep(1);
		}
	}
	for (auto v : _inputVertex)
		v->Reset();
	for (auto v : _outputVertex)
		v->Reset();
	_pause = false;
	_stop = false;
	_isRunning = false;
	emit sig_ResetFinished(sharedFromThis());
}

QWeakPointer<AlgVertex> AlgNode::AddVertex(AlgVertex::VertexType vertexType, QString name, 
	AlgVertex::Behavior_NoData defaultState, AlgVertex::Behavior_BeforeActivate beforeBehavior,
	AlgVertex::Behavior_AfterActivate afterBehavior, QVariant defaultData /*= QVariant()*/)
{
	qDebug() << objectName() + ':' + name << vertexType << __FUNCTION__;
	GRAPH_ASSERT(_isUnchange == false);
	GRAPH_ASSERT(GetVertexNames(vertexType).contains(name) == false);//TODO:需要判定重名，或者自动添加尾注（例如in_1,in_2）
	
	auto pv = AlgVertex::Create(sharedFromThis(), vertexType, name, defaultState, beforeBehavior, afterBehavior, defaultData);
	switch (vertexType)
	{
	case AlgVertex::VertexType::INPUT:
		connect(pv.data(), &AlgVertex::sig_Activated, this, &AlgNode::Activate);
		connect(this, &AlgNode::sig_ActivateFinished, pv.data(), &AlgVertex::slot_ActivateSuccess, Qt::QueuedConnection);//必须改成QueuedConnection，否则是直连，相当于会递归调用进行深度优先遍历
		//TODO:是否要添加自动解离？
		break;
	case AlgVertex::VertexType::OUTPUT:
		connect(pv.data(), &AlgVertex::sig_ActivateEnd, pv.data(), &AlgVertex::slot_ActivateSuccess);
		break;
	default:
		break;
	}
	//TODO:要不要自动添加GUI？
	_Vertexes(vertexType).append(pv);
	return pv;
}

QWeakPointer<AlgVertex> AlgNode::AddVertexAuto(AlgVertex::VertexType vertexType, QString name /*= ""*/, QVariant defaultData /*= QVariant()*/)
{
	GRAPH_ASSERT(_isUnchange == false);
	QSharedPointer<AlgVertex>pv;
	/*TODO:需要判定重名，或者自动添加尾注（例如in_1,in_2）
	int conflictAmount = 0;
	for (auto s : GetVertexNames(vertexType))
	if (s.contains(QRegExp(name + "_*[0-9]*")))
	conflictAmount++;*/
	if (GetVertexNames(vertexType).contains(name))
		name = name + '_' + QString::number(_Vertexes(vertexType).size());
	switch (vertexType)
	{
	case AlgVertex::VertexType::INPUT:
		pv = AddVertex(vertexType, name, AlgVertex::Behavior_NoData::USE_LAST,
			AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::KEEP, defaultData);
		break;
	case AlgVertex::VertexType::OUTPUT:
		pv = AddVertex(vertexType, name, AlgVertex::Behavior_NoData::USE_NULL,
			AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::RESET, defaultData);
		break;
	default:
		GRAPH_NOT_IMPLEMENT;
		break;
	}
	if (_gui.isNull() == false)
	{
		auto pgvtx = _gui->AddVertex(QWeakPointer<AlgVertex>());
		pv->AttachGui(pgvtx);
	}
	return pv;
}

bool AlgNode::RemoveVertex(AlgVertex::VertexType vertexType, QString name)
{
	auto &vtxs = _Vertexes(vertexType);
	for(auto i=0;i<vtxs.size();i++)
	{
		if (vtxs[i]->objectName() == name)
		{
			if (_gui.isNull() == false)
				_gui->RemoveVertex(vtxs[i]);
			vtxs.removeAt(i);
			return true;
		}
	}
	return false;
}

bool AlgNode::ConnectVertex(AlgVertex::VertexType vertexType, QString vertexName, 
	QSharedPointer<AlgNode>dstNode, AlgVertex::VertexType dstVertexType, QString dstVertexName)
{
	QSharedPointer<AlgVertex>srcVertex=_FindVertex(vertexType,vertexName),
		dstVertex=dstNode->_FindVertex(dstVertexType,dstVertexName);
	GRAPH_ASSERT(srcVertex != nullptr&&dstVertex != nullptr&&srcVertex != dstVertex);
	qDebug() << srcVertex->objectName() + ':' + srcVertex->objectName()
		<< dstNode->objectName() + ':' + dstVertex->objectName() << __FUNCTION__;
	srcVertex->Connect(dstVertex);

	return true;
}

void AlgNode::DisconnectVertex(AlgVertex::VertexType vertexType, QString vertexName,
	QSharedPointer<AlgNode>dstNode, AlgVertex::VertexType dstVertexType, QString dstVertexName)
{
	QSharedPointer<AlgVertex>srcVertex = _FindVertex(vertexType, vertexName),
		dstVertex = dstNode->_FindVertex(dstVertexType, dstVertexName);
	GRAPH_ASSERT(srcVertex != nullptr&&dstVertex != nullptr&&srcVertex != dstVertex);
	qDebug() << srcVertex->objectName() + ':' + srcVertex->objectName()
		<< dstNode->objectName() + ':' + dstVertex->objectName() << __FUNCTION__;
	srcVertex->Disconnect(dstVertex);
}

void AlgNode::Activate()
{
	qDebug() << objectName() << __FUNCTION__;
	if (_isRunning == false && _stop == false)//运行期间，阻塞输入
	{
		for (auto v : _inputVertex)//检查是否全部激活
			if (v->GetData().isValid()==false)
				return;
		//TODO:暂停
		_isRunning = true;
		_runningAmount++;
		Run();
	}
}

void AlgNode::Run()
{
	qDebug() << objectName() << __FUNCTION__;
	auto data = _LoadInput();
	emit sig_ActivateFinished(sharedFromThis());
	//TODO:暂停和退出
	switch (_mode)
	{
	case AlgNode::RunMode::Thread:
		_resultWatcher.setFuture(QtConcurrent::run(&_pool, this, &AlgNode::_Run, data));
		break;
	case AlgNode::RunMode::Direct:
		try
		{
			_result=_Run(data);
		}
		catch (GraphError&e)
		{
			qDebug() << e.msg;
			Stop(true);
		}
		catch (QException&e)
		{
			qDebug() << e.what();
			Stop(true);
		}
		Output();
		break;
	case AlgNode::RunMode::Function:
		GRAPH_NOT_IMPLEMENT;
		break;
	default:
		break;
	}
}

void AlgNode::Output()
{
	//TODO:加锁
	qDebug() << "====Output====:" << objectName();
	if (_mode != RunMode::Direct)
	{
		try
		{
			_resultWatcher.waitForFinished();
			_result = _resultWatcher.result();
		}
		catch (GraphError&e)
		{
			qDebug() << e.msg;
			Stop(true);
		}
		catch (QException&e)
		{
			qDebug() << e.what();
			Stop(true);
		}
	}
	emit sig_RunFinished(sharedFromThis());
	if (_stop == false)
	{
		_LoadOutput(_result);
		emit sig_OutputFinished(sharedFromThis());
	}
	_runningAmount--;
	_isRunning = false;
	//emit sig_ActivateReady(this);
}



QVariantHash AlgNode::_LoadInput()
{
	QVariantHash result;
	result.reserve(_inputVertex.size());
	for (auto v : _inputVertex)
		result.insert(v->objectName(), v->GetData());
	result.squeeze();
	return result;
}

QVariantHash AlgNode::_Run(QVariantHash data)
{
	qDebug() << objectName() << data;
	QVariantHash result;
 #ifdef _DEBUG
 	QThread::msleep(500);
 #endif // _DEBUG
	result["out"] = data["in"];
	result["out_1"] = data["in_1"];
	return result;
}

void AlgNode::_LoadOutput(QVariantHash result)
{
	for (auto v : _outputVertex)
		if (result.contains(v->objectName()) == true)
			v->Activate(result.value(v->objectName()));
}



QList<QSharedPointer<AlgVertex>>& AlgNode::_Vertexes(AlgVertex::VertexType type)
{
	switch (type)
	{
	case AlgVertex::VertexType::INPUT:return _inputVertex;
	case AlgVertex::VertexType::OUTPUT:return _outputVertex;
	default:GRAPH_NOT_IMPLEMENT; break;
	}
}

const QList<QSharedPointer<AlgVertex>>& AlgNode::_Vertexes(AlgVertex::VertexType type) const
{
	switch (type)
	{
	case AlgVertex::VertexType::INPUT:return _inputVertex;
	case AlgVertex::VertexType::OUTPUT:return _outputVertex;
	default:GRAPH_NOT_IMPLEMENT; break;
	}
}

QSharedPointer<AlgVertex> AlgNode::_FindVertex(AlgVertex::VertexType type, QString name)
{
	for (auto v : _Vertexes(type))
		if (v->objectName() == name)
			return v;
	return QSharedPointer<AlgVertex>();
}

const QSharedPointer<AlgVertex> AlgNode::_FindVertex(AlgVertex::VertexType type, QString name) const
{
	for (auto v : _Vertexes(type))
		if (v->objectName() == name)
			return v;
	return QSharedPointer<AlgVertex>();
}

QList<QSharedPointer<const AlgVertex>> AlgNode::GetVertexes(AlgVertex::VertexType type) const
{
	QList<QSharedPointer<const AlgVertex>>result;	
	switch (type)
	{
	case AlgVertex::VertexType::INPUT:
		result.reserve(_inputVertex.size());
		for (const auto &v : _inputVertex)
			result.append(v); 
		return result;
	case AlgVertex::VertexType::OUTPUT:
		result.reserve(_outputVertex.size());
		for (const auto &v : _outputVertex)
			result.append(v); 
		return result;
	default:GRAPH_NOT_IMPLEMENT; break;
	}
}

void AlgNode::ProcessAction(QString action, bool isChecked)
{
	if (action.isEmpty() == true)
		return;
	else if (action == "Thread")
	{
		GRAPH_ASSERT(_mode != RunMode::Function);
		if (isChecked == true)
			_mode = RunMode::Thread;
		else
			_mode = RunMode::Direct;
	}
	else if (action == "Add Input Auto")
	{
		AddVertexAuto(AlgVertex::VertexType::INPUT);
	}
	else if (action == "Add Output Auto")
	{
		AddVertexAuto(AlgVertex::VertexType::OUTPUT);
	}
	else if (action == "Rename")
	{
		QString newname = QInputDialog::getText(nullptr, "Rename", "Input a new name", QLineEdit::EchoMode::Normal,
			objectName());
		if (newname != nullptr)
			setObjectName(newname);
	}
}

QStringList AlgNode::GetVertexNames(AlgVertex::VertexType type) const
{
	QStringList result;
	for (auto pn : _Vertexes(type))
		result.append(pn->objectName());
	return result;
}



