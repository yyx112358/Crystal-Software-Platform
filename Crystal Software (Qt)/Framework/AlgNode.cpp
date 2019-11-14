#include "stdafx.h"
#include "AlgNode.h"

std::atomic_uint64_t AlgNode::_amount = 0;
std::atomic_uint64_t AlgNode::_runningAmount = 0;

AlgNode::AlgNode(QThreadPool&pool, QObject*parent)
	:QObject(parent),_pool(pool)
{
	_amount++;
#ifdef _DEBUG
	connect(this, &AlgNode::objectNameChanged, [this](QString str) {name = str; });
#endif // _DEBUG
}


AlgNode::~AlgNode()
{
	qDebug() << objectName() << __FUNCTION__;
	emit sig_Destroyed(_weakRef);
	--_amount;
}


