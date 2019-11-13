#pragma once
#include <QString>
#include <QtPlugin>

#include "GraphError.h"
#include "AlgNode.h"
#include "GuiNode.h"

class Interface_Factory
{
public:	
	virtual const QHash<QString, AlgNode::FactoryInfo>& GetAlgNodeTbl()const = 0;
	virtual const QHash<QString, GuiNode::FactoryInfo>& GetGuiNodeTbl()const = 0;

	QStringList GetAlgNodeNames() { return GetAlgNodeTbl().keys(); }
	QStringList GetGuiNodeNames() { return GetGuiNodeTbl().keys(); }

	auto CreateAlgNode(QString classname)
	{
		if (GetAlgNodeTbl().contains(classname) == true)
			return GetAlgNodeTbl().value(classname).defaultConstructor();
		else
			//GRAPH_NOT_EXIST(_GetAlgNodeTbl(), classname);
			throw GraphError(GraphError::NotExist, QString("[%1] Not Exist in [Factory]").arg( classname), __FUNCTION__, __FILE__, __LINE__);
	}
	auto CreateGuiNode(QString classname)
	{
		if (GetGuiNodeTbl().contains(classname) == true)
			return GetGuiNodeTbl().value(classname).defaultConstructor;
		else
			throw GraphError(GraphError::NotExist, QString("[%1] Not Exist in [Factory]").arg(classname), __FUNCTION__, __FILE__, __LINE__);
	}

};
#define FACTORYINFO_DECLARE_ALGNODE(nodeclass) AlgNode::FactoryInfo(#nodeclass,\
	[]{return QSharedPointer<nodeclass>::create();})

Q_DECLARE_INTERFACE(Interface_Factory, "software.interface.factory.Interface_Factory/0.1");