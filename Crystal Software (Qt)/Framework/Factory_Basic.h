#pragma once

#include <QObject>
#include "Interface_Factory.h"

class Factory_Basic
	: public QObject, public Interface_Factory
{
	Q_OBJECT
	Q_INTERFACES(Interface_Factory)
public:
	Factory_Basic::Factory_Basic(QObject *parent) : QObject(parent) {}
	Factory_Basic::~Factory_Basic() {}

	virtual const QHash<QString, AlgNode::FactoryInfo>& GetAlgNodeTbl() const override;
	virtual const QHash<QString, GuiNode::FactoryInfo>& GetGuiNodeTbl() const override;
};
