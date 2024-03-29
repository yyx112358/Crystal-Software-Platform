#pragma once

#include <QObject>
#include "AlgNode.h"

class AlgNode_Input 
	: public AlgNode
{
	Q_OBJECT
		Q_DISABLE_COPY(AlgNode_Input)
		friend class QSharedPointer<AlgNode_Input>;
public:
	virtual ~AlgNode_Input() {}

	virtual void Init() override;
	virtual QString GetGuiAdvice() const override { return "Basic.Input"; }

	virtual Category GetCategory() const{return AlgNode::Category::INPUT;}

protected:
	AlgNode_Input(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//放在这里表明只能由QSharedPointer构造

	virtual QVariantHash _Run(QVariantHash data) override;
};
