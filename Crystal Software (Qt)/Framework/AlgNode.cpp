#include "stdafx.h"
#include "AlgNode.h"

std::atomic_uint64_t AlgNode::_amount = 0;
std::atomic_uint64_t AlgNode::_runningAmount = 0;

AlgNode::AlgNode(QThreadPool&pool, QObject*parent)
	:QObject(parent),_pool(pool)
{
	_amount++;
#ifdef _DEBUG
	qDebug() << objectName() << __FUNCTION__;
	GRAPH_INFORM(__FUNCTION__); 
	connect(this, &AlgNode::objectNameChanged, [this](QString str) {__debugname = str; });
#endif // _DEBUG
}


AlgNode::~AlgNode()
{
	qDebug() << objectName() << __FUNCTION__;
	emit sig_Destroyed(_weakRef);
	--_amount;
}

void AlgNode::Init()
{

}


QWeakPointer<AlgVertex> AlgNode::AddVertex(AlgVertex::VertexType vertexType, QString name, 
	AlgVertex::Behavior_DefaultActivateState defaultState, AlgVertex::Behavior_BeforeActivate beforeBehavior,
	AlgVertex::Behavior_AfterActivate afterBehavior, QVariant defaultValue /*= QVariant()*/)
{
	qDebug() << objectName() + ':' + name << vertexType << __FUNCTION__;
	GRAPH_ASSERT(_isUnchange = false);
	GRAPH_WARNING("aaa");
	//auto pvtx = QSharedPointer::create()
	return QWeakPointer<AlgVertex>();
}

QWeakPointer<AlgVertex> AlgNode::AddVertexAuto(AlgVertex::VertexType vertexType, QString name /*= ""*/)
{
	return QWeakPointer<AlgVertex>();
}

QStringList AlgNode::GetVertexNames(AlgVertex::VertexType type) const
{
	QStringList result;
	switch (type)
	{
	case AlgVertex::VertexType::INPUT:	for (auto pn : _inputVertex)result.append(pn->objectName()); break;
	case AlgVertex::VertexType::OUTPUT:for (auto pn : _outputVertex)result.append(pn->objectName()); break;
	default:		GRAPH_NOT_IMPLEMENT;		break;
	}
	return result;
}

