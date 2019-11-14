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
		QString key;//����Ψһ��ʶ��key
		QString title;//��ʾ�õı���
		QString description;//����
		std::function<QSharedPointer<GuiNode>(AlgNode&)>defaultConstructor;//Ĭ�Ϲ��캯��

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
	static std::atomic_uint64_t _amount;//��ʵ������
};

