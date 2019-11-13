#pragma once
#include "qgraphicsitem.h"
#include <atomic>

class GuiNode :
	public QGraphicsObject
{
public:
	struct FactoryInfo
	{
		QString key;//����Ψһ��ʶ��key
		QString title;//��ʾ�õı���
		QString description;//����
		std::function<QSharedPointer<GuiNode>()>defaultConstructor;//Ĭ�Ϲ��캯��

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
	static std::atomic_uint64_t _amount;//��ʵ������
};

