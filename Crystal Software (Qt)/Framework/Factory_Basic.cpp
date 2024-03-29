#include "stdafx.h"
#include "Factory_Basic.h"

QMap<QString, QStringList> Factory_Basic::LoadFromAnother(Interface_Factory&another, bool autoRename/* = false*/)
{
	QMap<QString, QStringList>result;
	if (autoRename == false)
	{
		{
			auto tbl = another.GetAlgNodeTbl();
			QStringList conflicts;
			for (auto it = tbl.cbegin(); it != tbl.cend(); ++it)
			{
				if (_algNodeTbl.contains(it.key()) == true)
					conflicts.append(it.key());
				_algNodeTbl.insert(it.key(), it.value());
			}
			if (conflicts.size() > 0)
				result.insert(NAME2STR(_algNodeTbl), conflicts);
		}
		{
			auto tbl = another.GetGuiNodeTbl();
			QStringList conflicts;
			for (auto it = tbl.cbegin(); it != tbl.cend(); ++it)
			{
				if (_algNodeTbl.contains(it.key()) == true)
					conflicts.append(it.key());
				_guiNodeTbl.insert(it.key(), it.value());
			}
			if (conflicts.size() > 0)
				result.insert(NAME2STR(_guiNodeTbl), conflicts);
		}
	}
	else
		GRAPH_NOT_IMPLEMENT;
	return result;
}

#include "AlgNode_Input.h"
#include "AlgNode_Output.h"
#include "AlgNode_Constant.h"
#include "AlgNode_DEMO.h"
QHash<QString, AlgNode::FactoryInfo>& Factory_Basic::GetDefaultAlgNodeTbl() const
{
	static QHash<QString, AlgNode::FactoryInfo>tbl =
	{
 		{ "",AlgNode::FactoryInfo("",QStringLiteral("基本"),[] {return QSharedPointer<AlgNode>::create(); },QStringLiteral("基本节点")) },
 		{ "Basic.Input",AlgNode::FactoryInfo("Basic.Input",[] {return QSharedPointer<AlgNode_Input>::create(); }) },
 		{ "Basic.Output",AlgNode::FactoryInfo("Basic.Output",[] {return QSharedPointer<AlgNode_Output>::create(); }) },
		{ "Basic.Add",AlgNode::FactoryInfo("Basic.Add",[] {return QSharedPointer<AlgNode>::create(); }) },
		{ "Basic.Constant",AlgNode::FactoryInfo("Basic.Constant",[] {return QSharedPointer<AlgNode_Constant>::create(); }) },
		{"DEMO.InputImage",AlgNode::FactoryInfo("DEMO.InputImage",[] {return QSharedPointer<AlgNode_DEMO_InputImage>::create(); })},
		{ "DEMO.ShowImage",AlgNode::FactoryInfo("DEMO.ShowImage",[] {return QSharedPointer<AlgNode_DEMO_ShowImage>::create(); }) },
		{ "DEMO.ImageROI",AlgNode::FactoryInfo("DEMO.ImageROI",[] {return QSharedPointer<AlgNode_DEMO_ImageROI>::create(); }) },
		{ "DEMO.AddWeighted",AlgNode::FactoryInfo("DEMO.AddWeighted",[] {return QSharedPointer<AlgNode_DEMO_AddWeighted>::create(); }) },
		{ "DEMO.SetROI",AlgNode::FactoryInfo("DEMO.SetROI",[] {return QSharedPointer<AlgNode_DEMO_SetROI>::create(); }) },
		{ "DEMO.BiggerThan",AlgNode::FactoryInfo("DEMO.BiggerThan",[] {return QSharedPointer<AlgNode_DEMO_BiggerThan>::create(); }) },
		{ "DEMO.Count",AlgNode::FactoryInfo("DEMO.Count",[] {return QSharedPointer<AlgNode_DEMO_Count>::create(); }) },
		{ "DEMO.Switch",AlgNode::FactoryInfo("DEMO.Switch",[] {return QSharedPointer<AlgNode_DEMO_Switch>::create(); }) },
		
	};
	return tbl;
}

#include "GuiNode_Input.h"
#include "GuiNode_Output.h"
#include "GuiNode_Constant.h"
QHash<QString, GuiNode::FactoryInfo>& Factory_Basic::GetDefaultGuiNodeTbl() const
{
	static QHash<QString, GuiNode::FactoryInfo>tbl =
	{
		{"",GuiNode::FactoryInfo("",[](QSharedPointer<AlgNode> node) {return QSharedPointer<GuiNode>::create(node); })},
		{"Basic.Input",GuiNode::FactoryInfo("Basic.Input",[](QSharedPointer<AlgNode> node) {return QSharedPointer<GuiNode_Input>::create(node); })},
		{"Basic.Output",GuiNode::FactoryInfo("Basic.Output",[](QSharedPointer<AlgNode> node) {return QSharedPointer<GuiNode_Output>::create(node); }) },
		{ "Basic.Constant",GuiNode::FactoryInfo("Basic.Constant",[](QSharedPointer<AlgNode> node) {return QSharedPointer<GuiNode_Constant>::create(node); }) },
		{ "DEMO.ShowImage",GuiNode::FactoryInfo("DEMO.ShowImage",[](QSharedPointer<AlgNode> node) {return QSharedPointer<GuiNode_DEMO_ShowImage>::create(node); }) } ,

	};
	return tbl;
}

