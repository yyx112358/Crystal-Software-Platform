#include "stdafx.h"
#include "AlgVertex.h"
#include "AlgNode.h"


AlgVertex::AlgVertex(QWeakPointer<AlgNode>parent, VertexType type, QString name, Behavior_DefaultActivateState defaultState,
	Behavior_BeforeActivate beforeBehavior, Behavior_AfterActivate afterBehavior)
	:node(parent),_type(type),_behaviorDefault(defaultState),_behaviorBefore(beforeBehavior),_behaviorAfter(afterBehavior)
{
	sizeof(AlgVertex);
	setObjectName(name);
#ifdef _DEBUG
	connect(this, &AlgNode::objectNameChanged, [this](QString str) {__debugname = str; });
#endif // _DEBUG
}

AlgVertex::~AlgVertex()
{
}

std::atomic_uint64_t AlgVertex::_amount;
