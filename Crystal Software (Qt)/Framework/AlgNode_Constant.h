#pragma once

#include "AlgNode.h"

//����Node
//��ӵ��һ��USE_LAST_FIRST_DEFAULT,DIRECT,KEEP��OUTPUT Vertex����������
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
	AlgNode_Constant(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//�����������ֻ����QSharedPointer����

	virtual QVariantHash _Run(QVariantHash data) override;

	QVariant _data;
};
