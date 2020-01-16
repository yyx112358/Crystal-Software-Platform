#include "stdafx.h"
#include "AlgNode_Input.h"
#include "GuiNode_Input.h"

AlgNode_Input::AlgNode_Input(QThreadPool&pool /*= *QThreadPool::globalInstance()*/, QObject*parent /*= nullptr*/)
	: AlgNode(pool, parent)
{
	_mode = RunMode::Direct;

}

void AlgNode_Input::Init()
{
	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "out");
}
QVariantHash AlgNode_Input::_Run(QVariantHash data)
{
	GRAPH_ASSERT(_gui.isNull() == false);
	//QThread::msleep(500);
	data["out"] = _gui->GetData();
	return data;
}


