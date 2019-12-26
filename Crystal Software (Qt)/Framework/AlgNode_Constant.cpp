#include "stdafx.h"
#include "AlgNode_Constant.h"

AlgNode_Constant::AlgNode_Constant(QThreadPool&pool /*= *QThreadPool::globalInstance()*/, QObject*parent /*= nullptr*/)
{
	_mode = RunMode::Direct;
	_isUnchange = true;
#ifdef _DEBUG
	_data = 1.5;
#endif // _DEBUG
}

QVariantHash AlgNode_Constant::_Run(QVariantHash data)
{
	return QVariantHash({ std::pair<QString, QVariant>("", _data) });
}

void AlgNode_Constant::Init()
{
	_isUnchange = false;
	AddVertex(AlgVertex::VertexType::OUTPUT, "  ", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::KEEP, QVariant());
	_isUnchange = true;
}
