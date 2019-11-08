#pragma once

#include "pluginstatictool_global.h"
#include "..\PluginApp\Interface.h"

class PLUGINSTATICTOOL_EXPORT PluginStaticTool
	:public QObject, public Interface_Plugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "Tool.PluginStaticTool")
	Q_INTERFACES(Interface_Plugin)
public:
	virtual void Print() override;
};


