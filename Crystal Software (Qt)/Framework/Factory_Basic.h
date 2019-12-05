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
	
	//����һ������another���кϲ����е���Ϣ
	//autoRenameΪtrue������Զ�����ͻ��key-value�����������򣬽���ԭ�����滻Ϊanother���е�
	//���ط�����ͻ������
	QMap<QString,QStringList> LoadFromAnother(Interface_Factory&another, bool autoRename = false);
protected:
	virtual QHash<QString, AlgNode::FactoryInfo>& GetDefaultAlgNodeTbl() const override;
	virtual QHash<QString, GuiNode::FactoryInfo>& GetDefaultGuiNodeTbl() const override;
};
