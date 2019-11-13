#pragma once
#include "global.h"
#include "AlgVertex.h"
#include "GuiNode.h"
#include <functional>
#include <atomic>

class AlgNode
	:public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(AlgNode)
public:
	struct FactoryInfo
	{
		QString key;//����Ψһ��ʶ��key
		QString title;//��ʾ�õı���
		QString description;//����
		std::function<QSharedPointer<AlgNode>()>defaultConstructor;//Ĭ�Ϲ��캯��
		std::function<QSharedPointer<AlgNode>(QString)>commandConstructor;

		FactoryInfo(){}
		FactoryInfo(QString key, std::function<QSharedPointer<AlgNode>()>defaultConstructor, QString description = "")
			:key(key), title(key), description(description), defaultConstructor(defaultConstructor){}
		
	};

	AlgNode();
	virtual ~AlgNode();

	static size_t GetAmount() { return _amount; }
	static size_t GetRunningAmount() { return _runningAmount; }
protected:

	QSharedPointer<GuiNode>_gui;
private:
	static std::atomic_uint64_t _amount;//��ʵ������
	static std::atomic_uint64_t _runningAmount;
};

