#include "stdafx.h"
#include "AlgNode.h"

std::atomic_uint64_t AlgNode::_amount = 0;
std::atomic_uint64_t AlgNode::_runningAmount = 0;

AlgNode::AlgNode()
{
	_amount++;
}


AlgNode::~AlgNode()
{
	qDebug() << objectName() << __FUNCTION__;
	--_amount;
}


