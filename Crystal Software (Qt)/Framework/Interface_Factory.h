#pragma once
#include <QString>
#include <QtPlugin>

class Interface_Factory
{
public:
	virtual void LoadToStaticTbl() = 0;
	virtual QStringList GetAlgNodeNames() = 0;
	virtual QStringList GetGuiNodeNames() = 0;
};

Q_DECLARE_INTERFACE(Interface_Factory, "software.interface.factory.Interface_Factory/0.1");