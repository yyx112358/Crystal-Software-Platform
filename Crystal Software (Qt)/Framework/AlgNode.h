#pragma once
#include "global.h"
#include "AlgVertex.h"

class AlgNode
	:public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(AlgNode)
public:
	AlgNode();
	virtual ~AlgNode();

	static size_t GetAmount() { return _amount; }
	static size_t GetRunningAmount() { return _runningAmount; }
protected:

private:
	static std::atomic_uint64_t _amount;//类实例总数
	static std::atomic_uint64_t _runningAmount;
};

