#pragma once

#include <QObject>
#include "AlgNode.h"

class AlgNode_Input 
	: public AlgNode,private QEnableSharedFromThis<AlgNode_Input>
{
	Q_OBJECT
	Q_DISABLE_COPY(AlgNode_Input)
public:
	friend class Interface_Factory;
	friend class QSharedPointer<AlgNode>;

	virtual ~AlgNode_Input();

	virtual void Init() override;

protected:
	AlgNode_Input(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//�����������ֻ����QSharedPointer����


	virtual QVariantHash _Run(QVariantHash data) override;

};
