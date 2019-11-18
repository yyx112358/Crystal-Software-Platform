#pragma once
#include "global.h"
#include "GraphError.h"
#include <QObject>
#include <QVariant>
#include <atomic>

class AlgNode;
class GuiVertex;

class AlgVertex : public QObject
{
	Q_OBJECT

public:
	enum class VertexType :unsigned char
	{
		INPUT,//����
		OUTPUT,//���
	};
	friend QDebug& operator<<(QDebug&qd, VertexType vt) { qd << ((vt == AlgVertex::VertexType::INPUT) ? "INPUT" : "OUTPUT");	return qd; }
	enum class Behavior_DefaultActivateState :unsigned char//Ĭ�ϼ���״̬
	{
		DEFAULT_ACTIVATE,
		DEFAULT_NOT_ACTIVATE,
	};
	enum class Behavior_BeforeActivate :unsigned char//����ǰ��Ϊ
	{
		DIRECT,//ֱ�Ӽ�������塾���ܶ�ʧ�����źš�
		BUFFER,//���壬ȷ�����м��������
	};
	enum class Behavior_AfterActivate :unsigned char//�������Ϊ
	{
		KEEP,//����󱣳�����
		RESET,//������������
	};

	friend class AlgNode;
	friend class QSharedPointer<AlgVertex>;
	static QSharedPointer<AlgVertex>Create(QWeakPointer<AlgNode>parent, VertexType type, QString name, Behavior_DefaultActivateState defaultState,
		Behavior_BeforeActivate beforeBehavior, Behavior_AfterActivate afterBehavior, QVariant defaultData = QVariant());
	~AlgVertex();


	AlgVertex::Behavior_AfterActivate GetBehaviorAfter() const { return _behaviorAfter; }
	void SetBehaviorAfter(AlgVertex::Behavior_AfterActivate val) { _behaviorAfter = val; }
	AlgVertex::Behavior_BeforeActivate GetBehaviorBefore() const { return _behaviorBefore; }
	void SetBehaviorBefore(AlgVertex::Behavior_BeforeActivate val) { _behaviorBefore = val; }
	AlgVertex::Behavior_DefaultActivateState GetBehaviorDefault() const { return _behaviorDefault; }
	void SetBehaviorDefault(AlgVertex::Behavior_DefaultActivateState val) { _behaviorDefault = val; }

	void AttachGui(QSharedPointer<GuiVertex>gui) { GRAPH_ASSERT(gui.isNull() == false); _gui = gui; }
	QWeakPointer<AlgVertex>WeakRef()const { return _weakRef; }

	static size_t GetAmount() { return _amount; }
	const VertexType type;
protected:
	AlgVertex(QWeakPointer<AlgNode>parent, VertexType type, QString name, Behavior_DefaultActivateState defaultState,
		Behavior_BeforeActivate beforeBehavior, Behavior_AfterActivate afterBehavior, QVariant defaultData = QVariant());

	//QVariant _data;
	QQueue<QVariant>_qdata;//���ݶ���
	QVariant _defaultData;//Ĭ������
	QVariantMap additionInfo;//������Ϣ
	QList<std::function<bool(/*const AlgVertex*const, */const QVariant&)>>inputAssertFunctions;//����У�顣Ϊtrue�Żἤ��//TODO:������ܻ�ĳɴ����ֺ�std::function���Զ�����
	QList<std::function<bool(/*const AlgVertex*const, */const AlgVertex*const)>> connectAssertFunctions;//����У��

	QList<QWeakPointer<AlgVertex>> prevVertexes;//���ӵ�����һ���˿�
	QList<QWeakPointer<AlgVertex>> nextVertexes;//���ӵ�����һ���˿�

	std::atomic_bool isActivated = false;//�����־
	std::atomic_bool isEnabled = true;//ʹ�ܱ�־
	
	Behavior_AfterActivate _behaviorAfter;
	Behavior_BeforeActivate _behaviorBefore;
	Behavior_DefaultActivateState _behaviorDefault;

	QWeakPointer<AlgNode>_node;//�����Ľڵ�
	QWeakPointer<GuiVertex>_gui;
private:
#ifdef _DEBUG
	QString __debugname;//�����õģ����㿴����
#endif // _DEBUG	
	void SetWeakRef(QWeakPointer<AlgVertex>wp) { GRAPH_ASSERT(_weakRef.isNull() == true); _weakRef = wp; }
	QWeakPointer<AlgVertex>_weakRef;//�����weakRef�����ڴ���this��������ָ��

	static std::atomic_uint64_t _amount;//��ʵ������
};

