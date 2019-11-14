#pragma once
#include <QString>
#include <QtPlugin>

#include "GraphError.h"
#include "AlgNode.h"
#include "GuiNode.h"

class Interface_Factory
{
public:	
	virtual const QHash<QString, AlgNode::FactoryInfo>& GetAlgNodeTbl()const { return _algNodeTbl; }
	QStringList GetAlgNodeNames() { return _algNodeTbl.keys(); }
	QSharedPointer<AlgNode> CreateAlgNode(QString classname)
	{
		if (_algNodeTbl.contains(classname) == true) 
		{
			auto node = _algNodeTbl.value(classname).defaultConstructor();
			node->SetWeakRef(node);
			return node;
		}
		else
			throw GraphError(GraphError::NotExist, QString("[%1] Not Exist in [Factory]").arg( classname), __FUNCTION__, __FILE__, __LINE__);
	}

	virtual const QHash<QString, GuiNode::FactoryInfo>& GetGuiNodeTbl()const { return _guiNodeTbl; }
	QStringList GetGuiNodeNames() { return _guiNodeTbl.keys(); }
	QSharedPointer<GuiNode> CreateGuiNode(QString classname,AlgNode&algnode)
	{
		if (_guiNodeTbl.contains(classname) == true) 
		{
			auto node = _guiNodeTbl.value(classname).defaultConstructor(algnode);
			node->SetWeakRef(node);
			return node;
		}
		else
			throw GraphError(GraphError::NotExist, QString("[%1] Not Exist in [Factory]").arg(classname), __FUNCTION__, __FILE__, __LINE__);
	}
protected:
	QHash<QString, AlgNode::FactoryInfo>_algNodeTbl;
	QHash<QString, GuiNode::FactoryInfo>_guiNodeTbl;

	virtual const QHash<QString, AlgNode::FactoryInfo>& GetDefaultAlgNodeTbl()const = 0;
	virtual const QHash<QString, GuiNode::FactoryInfo>& GetDefaultGuiNodeTbl()const = 0;
};
#define FACTORYINFO_DECLARE_ALGNODE(nodeclass) AlgNode::FactoryInfo(#nodeclass,\
	[]{return QSharedPointer<nodeclass>::create();})

Q_DECLARE_INTERFACE(Interface_Factory, "software.interface.factory.Interface_Factory/0.1");