#pragma once

#include "AlgNode.h"
#include <opencv2/core/mat.hpp>
#include "AlgNode_Input.h"
#include "AlgNode_Output.h"
#include "GuiNode_Output.h"

class AlgNode_DEMO_InputImage
	:public AlgNode_Input
{
	Q_OBJECT
		Q_DISABLE_COPY(AlgNode_DEMO_InputImage)
		friend class QSharedPointer<AlgNode_DEMO_InputImage>;
public:
	virtual ~AlgNode_DEMO_InputImage() {}
	virtual void Init() override;
	//virtual QString GetGuiAdvice() const override { return "DEMO.InputImage"; }
	
	virtual void SetImagePath(QString path);
protected:
	AlgNode_DEMO_InputImage(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//放在这里表明只能由QSharedPointer构造

	virtual QVariantHash _Run(QVariantHash data) override;
//	QVariant _data;
};

class AlgNode_DEMO_ShowImage
	:public AlgNode_Output
{
	Q_OBJECT
		Q_DISABLE_COPY(AlgNode_DEMO_ShowImage)
		friend class QSharedPointer<AlgNode_DEMO_ShowImage>;
public:
	virtual ~AlgNode_DEMO_ShowImage() {}

	virtual void Init() override;

	//	virtual void Init() override;
	virtual QString GetGuiAdvice() const override { return "DEMO.ShowImage"; }

protected:
	AlgNode_DEMO_ShowImage(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//放在这里表明只能由QSharedPointer构造

	virtual QVariantHash _Run(QVariantHash data) override;
};

class GuiNode_DEMO_ShowImage
	:public GuiNode_Output
{
	Q_OBJECT
		friend class QSharedPointer<GuiNode_DEMO_ShowImage>;
public:
	GuiNode_DEMO_ShowImage(QSharedPointer<AlgNode>parent) :GuiNode_Output(parent) {}
	virtual ~GuiNode_DEMO_ShowImage() {}

	//virtual QSharedPointer<QWidget> InitWidget(QWidget*parent) override;

	virtual QVariant GetData() override;
	virtual void SetData(QVariant data) override;
};

class AlgNode_DEMO_ImageROI
	:public AlgNode
{
	Q_OBJECT
		Q_DISABLE_COPY(AlgNode_DEMO_ImageROI)
		friend class QSharedPointer<AlgNode_DEMO_ImageROI>;
public:
	virtual ~AlgNode_DEMO_ImageROI() {}

	virtual void Init() override;

	virtual Category GetCategory() const override { return AlgNode::Category::ALGORITHM; }

	//virtual QString GetGuiAdvice() const override { return "DEMO.ShowImage"; }

protected:
	AlgNode_DEMO_ImageROI(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//放在这里表明只能由QSharedPointer构造

	virtual QVariantHash _Run(QVariantHash data) override;
};

class AlgNode_DEMO_AddWeighted
	:public AlgNode
{
	Q_OBJECT
		Q_DISABLE_COPY(AlgNode_DEMO_AddWeighted)
		friend class QSharedPointer<AlgNode_DEMO_AddWeighted>;
public:
	virtual ~AlgNode_DEMO_AddWeighted() {}

	virtual void Init() override;

	virtual Category GetCategory() const override { return AlgNode::Category::ALGORITHM; }

	//virtual QString GetGuiAdvice() const override { return "DEMO.ShowImage"; }

protected:
	AlgNode_DEMO_AddWeighted(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//放在这里表明只能由QSharedPointer构造

	virtual QVariantHash _Run(QVariantHash data) override;
};

class AlgNode_DEMO_SetROI
	:public AlgNode
{
	Q_OBJECT
		Q_DISABLE_COPY(AlgNode_DEMO_SetROI)
		friend class QSharedPointer<AlgNode_DEMO_SetROI>;
public:
	virtual ~AlgNode_DEMO_SetROI() {}

	virtual void Init() override;

	virtual Category GetCategory() const override { return AlgNode::Category::ALGORITHM; }

	//virtual QString GetGuiAdvice() const override { return "DEMO.ShowImage"; }

protected:
	AlgNode_DEMO_SetROI(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr)//放在这里表明只能由QSharedPointer构造
		:AlgNode(pool,parent){}
	virtual QVariantHash _Run(QVariantHash data) override;
};

class AlgNode_DEMO_BiggerThan
	:public AlgNode
{
	Q_OBJECT
		Q_DISABLE_COPY(AlgNode_DEMO_BiggerThan)
		friend class QSharedPointer<AlgNode_DEMO_BiggerThan>;
public:
	virtual ~AlgNode_DEMO_BiggerThan() {}

	virtual void Init() override;

	virtual Category GetCategory() const override { return AlgNode::Category::OPERATION; }

	//virtual QString GetGuiAdvice() const override { return "DEMO.ShowImage"; }

protected:
	AlgNode_DEMO_BiggerThan(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr)//放在这里表明只能由QSharedPointer构造
		:AlgNode(pool, parent) {}
	virtual QVariantHash _Run(QVariantHash data) override;
};
class AlgNode_DEMO_Switch
	:public AlgNode
{
	Q_OBJECT
		Q_DISABLE_COPY(AlgNode_DEMO_Switch)
		friend class QSharedPointer<AlgNode_DEMO_Switch>;
public:
	virtual ~AlgNode_DEMO_Switch() {}

	virtual void Init() override;

	virtual Category GetCategory() const override { return AlgNode::Category::OPERATION; }

	//virtual QString GetGuiAdvice() const override { return "DEMO.ShowImage"; }

protected:
	AlgNode_DEMO_Switch(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr)//放在这里表明只能由QSharedPointer构造
		:AlgNode(pool, parent) {}
	virtual QVariantHash _Run(QVariantHash data) override;
};
class AlgNode_DEMO_Count
	:public AlgNode
{
	Q_OBJECT
		Q_DISABLE_COPY(AlgNode_DEMO_Count)
		friend class QSharedPointer<AlgNode_DEMO_Count>;
public:
	virtual ~AlgNode_DEMO_Count() {}

	virtual void Init() override;

	virtual Category GetCategory() const override { return AlgNode::Category::OPERATION; }


	virtual void Reset() override;

	//virtual QString GetGuiAdvice() const override { return "DEMO.ShowImage"; }

protected:
	AlgNode_DEMO_Count(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr)//放在这里表明只能由QSharedPointer构造
		:AlgNode(pool, parent) {}
	virtual QVariantHash _Run(QVariantHash data) override;
	int _cnt = 0;
};