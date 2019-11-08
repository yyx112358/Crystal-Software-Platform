#pragma once

#include <QtPlugin>
class Interface_Plugin
{
public:
	virtual ~Interface_Plugin(){}
	virtual void Print() {}
};
Q_DECLARE_INTERFACE(Interface_Plugin, "App.Interface_Plugin/1.0")