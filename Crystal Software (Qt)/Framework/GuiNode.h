#pragma once
#include "qgraphicsitem.h"
#include <atomic>
#include "AlgNode.h"

class GuiNode :
	public QGraphicsObject
{
public:
	struct FactoryInfo
	{
		QString key;//用于唯一标识的key
		QString title;//显示用的标题
		QString description;//描述
		std::function<QSharedPointer<GuiNode>(AlgNode&)>defaultConstructor;//默认构造函数

		FactoryInfo() {}
		FactoryInfo(QString key, std::function<QSharedPointer<GuiNode>(AlgNode&)>defaultConstructor, QString description = QString())
			:key(key), title(key), description(description), defaultConstructor(defaultConstructor)
		{}
	};
	GuiNode(AlgNode&parent);
	virtual ~GuiNode();

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	static size_t GetAmount() { return _amount; }
	const AlgNode&algnode;
protected:

private:
	static std::atomic_uint64_t _amount;//类实例总数
};

