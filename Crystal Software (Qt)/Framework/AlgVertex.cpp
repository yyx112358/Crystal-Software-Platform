#include "stdafx.h"
#include "AlgVertex.h"
#include "AlgNode.h"
#include "GuiVertex.h"

AlgVertex::AlgVertex(QWeakPointer<AlgNode>parent, VertexType type, QString name, Behavior_DefaultActivateState defaultState,
	Behavior_BeforeActivate beforeBehavior, Behavior_AfterActivate afterBehavior, QVariant defaultData/* = QVariant()*/)
	:_node(parent),type(type),_behaviorDefault(defaultState),
	_behaviorBefore(beforeBehavior),_behaviorAfter(afterBehavior), _defaultData(defaultData)
{
	_amount++;
	setObjectName(name);
#ifdef _DEBUG
	connect(this, &AlgNode::objectNameChanged, [this](QString str) {__debugname = str; });
#endif // _DEBUG
	inputAssertFunctions.append([this](const QVariant&)
	{if (_qdata.size() < 256)return true; else return false; });
	bool b= inputAssertFunctions[0](QVariant());
}

QSharedPointer<AlgVertex> AlgVertex::Create(QWeakPointer<AlgNode>parent, VertexType type, QString name,
	Behavior_DefaultActivateState defaultState, Behavior_BeforeActivate beforeBehavior, 
	Behavior_AfterActivate afterBehavior, QVariant defaultData /*= QVariant()*/)
{
	auto pvtx = QSharedPointer<AlgVertex>::create(parent, type, name, defaultState, beforeBehavior, afterBehavior, defaultData);
	pvtx->SetWeakRef(pvtx);
	return pvtx;
}

AlgVertex::~AlgVertex()
{
	qDebug() << objectName() << __FUNCTION__;
	_amount--;
}

std::atomic_uint64_t AlgVertex::_amount;
