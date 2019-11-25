#pragma once

#include <QObject>
#include "AlgNode.h"

class AlgNode_Input 
	: public AlgNode
{
	Q_OBJECT
	Q_DISABLE_COPY(AlgNode_Input)
public:
	virtual ~AlgNode_Input() {}

	virtual void Init() override;
	virtual QString GetGuiAdvice() const override { return "Basic.Input"; }

protected:
	friend class QSharedPointer<AlgNode_Input>;

	AlgNode_Input(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//放在这里表明只能由QSharedPointer构造


	virtual QVariantHash _Run(QVariantHash data) override;

};
