#pragma once

#include <QObject>
#include "Interface_Factory.h"

class Factory_Basic
	: public QObject, public Interface_Factory
{
	Q_OBJECT
	Q_INTERFACES(Interface_Factory)
public:
	Factory_Basic::Factory_Basic(QObject *parent) : QObject(parent) 
	{
		_algNodeTbl = GetDefaultAlgNodeTbl();
		_guiNodeTbl = GetDefaultGuiNodeTbl();
	}
	virtual Factory_Basic::~Factory_Basic() {}
	
	//从另一个工厂another当中合并所有的信息
	//autoRename为true，则会自动将冲突的key-value重命名；否则，将把原来的替换为another当中的
	//返回发生冲突的名字
	QMap<QString,QStringList> LoadFromAnother(Interface_Factory&another, bool autoRename = false);
protected:
	virtual QHash<QString, AlgNode::FactoryInfo>& GetDefaultAlgNodeTbl() const override;
	virtual QHash<QString, GuiNode::FactoryInfo>& GetDefaultGuiNodeTbl() const override;
};
