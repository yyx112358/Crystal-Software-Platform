#include "stdafx.h"
#include "AlgNode_Input.h"



AlgNode_Input::AlgNode_Input(QThreadPool&pool /*= *QThreadPool::globalInstance()*/, QObject*parent /*= nullptr*/)
	: AlgNode(pool, parent)
{

}

void AlgNode_Input::Init()
{
	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "out");
}
QVariantHash AlgNode_Input::_Run(QVariantHash data)
{
	throw std::logic_error("The method or operation is not implemented.");
}

AlgNode_Input::~AlgNode_Input()
{
}


