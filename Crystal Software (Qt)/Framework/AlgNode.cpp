#include "stdafx.h"
#include "AlgNode.h"
#include "AlgVertex.h"
#include "GuiNode.h"

std::atomic_uint64_t AlgNode::_amount = 0;
std::atomic_uint64_t AlgNode::_runningAmount = 0;

AlgNode::AlgNode(QThreadPool&pool, QObject*parent)
	:QObject(parent),_pool(pool)
{
	_amount++;
	qDebug() << objectName() << __FUNCTION__;
	GRAPH_INFORM(objectName() + __FUNCTION__);
#ifdef _DEBUG
	connect(this, &AlgNode::objectNameChanged, [this](QString str) {__debugname = str; });
#endif // _DEBUG
}

AlgNode::~AlgNode()
{
	qDebug() << objectName() << __FUNCTION__;
	emit sig_Destroyed(_weakRef);
	try
	{
		Release();
	}
	catch (...)
	{
		qDebug() << "Error in " __FUNCTION__;
		assert(0);
	}
	--_amount;
}
void AlgNode::Release()
{
	_result.waitForFinished();
	//_inputVertex.clear();
	//_outputVertex.clear();
	//_gui.clear();
}
void AlgNode::Init()
{
	AddVertexAuto(AlgVertex::VertexType::INPUT, "in");
	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "out");
}

QWeakPointer<AlgVertex> AlgNode::AddVertex(AlgVertex::VertexType vertexType, QString name, 
	AlgVertex::Behavior_DefaultActivateState defaultState, AlgVertex::Behavior_BeforeActivate beforeBehavior,
	AlgVertex::Behavior_AfterActivate afterBehavior, QVariant defaultData /*= QVariant()*/)
{
	qDebug() << objectName() + ':' + name << vertexType << __FUNCTION__;
	GRAPH_ASSERT(_isUnchange == false);
	GRAPH_ASSERT(GetVertexNames(vertexType).contains(name) == false);//TODO:需要判定重名，或者自动添加尾注（例如in_1,in_2）
	
	auto pv = AlgVertex::Create(WeakRef(), vertexType, name, defaultState, beforeBehavior, afterBehavior, defaultData);
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
		pv = AddVertex(vertexType, name, AlgVertex::Behavior_DefaultActivateState::DEFAULT_NOT_ACTIVATE,
			AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::KEEP, defaultData);
		break;
	case AlgVertex::VertexType::OUTPUT:
		pv = AddVertex(vertexType, name, AlgVertex::Behavior_DefaultActivateState::DEFAULT_NOT_ACTIVATE,
			AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::RESET, defaultData);
		break;
	default:
		GRAPH_NOT_IMPLEMENT;
		break;
	}
	if (_gui.isNull() == false)
	{
		auto pgvtx=_gui->AddVertex(QWeakPointer<AlgVertex>());
		pv->AttachGui(pgvtx);
	}
	return pv;
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

QStringList AlgNode::GetVertexNames(AlgVertex::VertexType type) const
{
	QStringList result;
	for (auto pn : _Vertexes(type))
		result.append(pn->objectName());
	return result;
}



