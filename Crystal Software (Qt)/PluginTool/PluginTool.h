#pragma once

#include "plugintool_global.h"
#include "..\PluginApp\Interface.h"

class PLUGINTOOL_EXPORT PluginTool
	:public QObject,public Interface_Plugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "Tool.PluginTool")
	Q_INTERFACES(Interface_Plugin)
public:	
	virtual void Print() override;
};
