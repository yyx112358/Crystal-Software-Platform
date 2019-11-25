#include "stdafx.h"
#include "AlgNode_Output.h"
#include "GuiNode_Output.h"

AlgNode_Output::AlgNode_Output(QThreadPool&pool /*= *QThreadPool::globalInstance()*/, QObject*parent /*= nullptr*/)
	:AlgNode(pool,parent)
{
	_mode = RunMode::Direct;
}

void AlgNode_Output::Init()
{
	AddVertexAuto(AlgVertex::VertexType::INPUT, "in");
}

QVariantHash AlgNode_Output::_Run(QVariantHash data)
{
	GRAPH_ASSERT(_gui.isNull() == false);
	_gui->SetData(data.value("in"));
	return data;
}

