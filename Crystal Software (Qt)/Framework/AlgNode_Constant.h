#pragma once

#include "AlgNode.h"

//常量Node
//仅拥有一个USE_LAST_FIRST_DEFAULT,DIRECT,KEEP的OUTPUT Vertex，不可增改
//
class AlgNode_Constant 
	: public AlgNode
{
	Q_OBJECT
		Q_DISABLE_COPY(AlgNode_Constant)
		friend class QSharedPointer<AlgNode_Constant>;
public:
	virtual ~AlgNode_Constant() {}

	virtual void Init() override;
	virtual QString GetGuiAdvice() const override { return "Basic.Constant"; }
	
	virtual Category GetCategory() const { return AlgNode::Category::CONSTANT; }

	void SetData(QVariant data) { _data = data; }
	QVariant GetData()const { return _data; }
protected:
	AlgNode_Constant(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//放在这里表明只能由QSharedPointer构造

	virtual QVariantHash _Run(QVariantHash data) override;

	QVariant _data;
};
