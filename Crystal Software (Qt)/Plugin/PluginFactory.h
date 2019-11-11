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

	virtual void LoadToStaticTbl() override;


	virtual QStringList GetAlgNodeNames() override;


	virtual QStringList GetGuiNodeNames() override;

};
