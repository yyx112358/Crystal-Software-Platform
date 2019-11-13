#include "stdafx.h"
#include "AlgVertex.h"

AlgVertex::AlgVertex(QObject *parent)
	: QObject(parent)
{
}

AlgVertex::~AlgVertex()
{
}

std::atomic_uint64_t AlgVertex::_amount;
