#include "stdafx.h"
#include "Factory_Basic.h"

const QHash<QString, AlgNode::FactoryInfo>& Factory_Basic::GetAlgNodeTbl() const
{
	const static QHash<QString, AlgNode::FactoryInfo>tbl =
	{
 		{ "Basic.Basic",AlgNode::FactoryInfo("Basic.Basic",[] {return QSharedPointer<AlgNode>::create(); }) },
 		{ "Basic.Input",AlgNode::FactoryInfo("Basic.Input",[] {return QSharedPointer<AlgNode>::create(); }) },
 		{ "Basic.Output",AlgNode::FactoryInfo("Basic.Output",[] {return QSharedPointer<AlgNode>::create(); }) },
		{ "Basic.Add",AlgNode::FactoryInfo("Basic.Add",[] {return QSharedPointer<AlgNode>::create(); }) },
		{ "Basic.Buffer",AlgNode::FactoryInfo("Basic.Buffer",[] {return QSharedPointer<AlgNode>::create(); }) },
	};
	return tbl;
}

const QHash<QString, GuiNode::FactoryInfo>& Factory_Basic::GetGuiNodeTbl() const
{
	throw std::logic_error("The method or operation is not implemented.");
}
