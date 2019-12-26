#pragma once

#include "plugin_global.h"
#include "..\Framework\Interface_Factory.h"

class PLUGIN_EXPORT PluginFactory
	:public QObject,public Interface_Factory
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "software.factory.basic/0.1")
	Q_INTERFACES(Interface_Factory)
public:
	PluginFactory();

protected:
	virtual const QHash<QString, AlgNode::FactoryInfo>& GetDefaultAlgNodeTbl() const override;
	virtual const QHash<QString, GuiNode::FactoryInfo>& GetDefaultGuiNodeTbl() const override;

};
