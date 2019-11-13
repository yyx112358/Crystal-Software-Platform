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
		QString key;//用于唯一标识的key
		QString title;//显示用的标题
		QString description;//描述
		std::function<QSharedPointer<AlgNode>()>defaultConstructor;//默认构造函数
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
	static std::atomic_uint64_t _amount;//类实例总数
	static std::atomic_uint64_t _runningAmount;
};

