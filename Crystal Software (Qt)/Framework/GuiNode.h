#pragma once
#include "global.h"
#include "qgraphicsitem.h"
#include <atomic>
#include "AlgNode.h"
#include "GuiVertex.h"

class GuiNode :
	public QGraphicsObject
{
	Q_OBJECT
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
	enum { Type = GuiType_Node };
	virtual int type()const { return Type; }

	friend class Interface_Factory;
	friend class QSharedPointer<GuiNode>;

	virtual ~GuiNode();

	virtual void InitApperance(QPointF center);

	virtual QWidget* InitWidget(QWidget*parent) { return nullptr; }

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	QWeakPointer<GuiNode>WeakRef()const { return _weakRef; }
	static size_t GetAmount() { return _amount; }
	const QWeakPointer<AlgNode> algnode;
signals:
	void sig_SendActionToAlg(QString action, bool isChecked);
	void sig_SendActionToController(QWeakPointer<GuiNode>wp, QString action, bool isChecked);
protected:
	QMap<QString, QWeakPointer<GuiVertex>>_inputVertex;
	QMap<QString, QWeakPointer<GuiVertex>>_outputVertex;
	QSharedPointer<QWidget>_panel;

	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
	GuiNode(AlgNode&parent);
	void SetWeakRef(QWeakPointer<GuiNode>wp) { _weakRef = wp; }
	QWeakPointer<GuiNode>_weakRef;//�����weakRef������

	static std::atomic_uint64_t _amount;//��ʵ������
};

