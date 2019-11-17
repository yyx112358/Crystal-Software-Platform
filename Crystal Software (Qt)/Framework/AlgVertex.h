#pragma once
#include "global.h"
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

	AlgVertex(QWeakPointer<AlgNode>parent, VertexType type, QString name, Behavior_DefaultActivateState defaultState,
		Behavior_BeforeActivate beforeBehavior, Behavior_AfterActivate afterBehavior);
	~AlgVertex();


	AlgVertex::Behavior_AfterActivate GetBehaviorAfter() const { return _behaviorAfter; }
	void SetBehaviorAfter(AlgVertex::Behavior_AfterActivate val) { _behaviorAfter = val; }
	AlgVertex::Behavior_BeforeActivate GetBehaviorBefore() const { return _behaviorBefore; }
	void SetBehaviorBefore(AlgVertex::Behavior_BeforeActivate val) { _behaviorBefore = val; }
	AlgVertex::Behavior_DefaultActivateState GetBehaviorDefault() const { return _behaviorDefault; }
	void SetBehaviorDefault(AlgVertex::Behavior_DefaultActivateState val) { _behaviorDefault = val; }
	static size_t GetAmount() { return _amount; }
protected:
	//QVariant _data;
	QQueue<QVariant>_qdata;//���ݶ���
	QVariant _defaultData;//Ĭ������
	QVariantMap additionInfo;//������Ϣ
	QList<std::function<bool(const AlgVertex*const, const QVariant&)>>inputAssertFunctions;//����У�顣Ϊtrue�Żἤ��
	QList<std::function<bool(const AlgVertex*const, const AlgVertex*const)>> connectAssertFunctions;//����У��

	QList<QWeakPointer<AlgVertex>> prevVertexes;//���ӵ�����һ���˿�
	QList<QWeakPointer<AlgVertex>> nextVertexes;//���ӵ�����һ���˿�

	std::atomic_bool isActivated = false;//�����־
	std::atomic_bool isEnabled = true;//ʹ�ܱ�־
	const VertexType _type;
	Behavior_AfterActivate _behaviorAfter;
	Behavior_BeforeActivate _behaviorBefore;
	Behavior_DefaultActivateState _behaviorDefault;

	QWeakPointer<AlgNode>node;//�����Ľڵ�
	QWeakPointer<GuiVertex>gui;
private:
#ifdef _DEBUG
	QString __debugname;//�����õģ����㿴����
#endif // _DEBUG
	static std::atomic_uint64_t _amount;//��ʵ������
};

