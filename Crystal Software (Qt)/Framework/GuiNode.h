#pragma once
#include "qgraphicsitem.h"
#include <atomic>

class GuiNode :
	public QGraphicsObject
{
public:
	struct FactoryInfo
	{
		QString key;//用于唯一标识的key
		QString title;//显示用的标题
		QString description;//描述
		std::function<QSharedPointer<GuiNode>()>defaultConstructor;//默认构造函数

		FactoryInfo() {}
		FactoryInfo(QString key,QString description, std::function<QSharedPointer<GuiNode>()>defaultConstructor)
			:key(key),description(description),defaultConstructor(defaultConstructor)
		{}
	};
	GuiNode();
	virtual ~GuiNode();

	static size_t GetAmount() { return _amount; }
protected:

private:
	static std::atomic_uint64_t _amount;//类实例总数
};

