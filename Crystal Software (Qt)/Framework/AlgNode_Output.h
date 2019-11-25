#pragma once

#include "AlgNode.h"

class AlgNode_Output 
	: public AlgNode
{
	Q_OBJECT
	Q_DISABLE_COPY(AlgNode_Output)
public:
	virtual ~AlgNode_Output(){}

	virtual void Init() override;
	virtual QString GetGuiAdvice() const override { return "Basic.Output"; }
protected:
	friend class QSharedPointer<AlgNode_Output>;

	AlgNode_Output(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//�����������ֻ����QSharedPointer����

	virtual QVariantHash _Run(QVariantHash data) override;
};
