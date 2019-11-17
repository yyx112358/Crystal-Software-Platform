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
		INPUT,//输入
		OUTPUT,//输出
	};
	friend QDebug& operator<<(QDebug&qd, VertexType vt) { qd << ((vt == AlgVertex::VertexType::INPUT) ? "INPUT" : "OUTPUT");	return qd; }
	enum class Behavior_DefaultActivateState :unsigned char//默认激活状态
	{
		DEFAULT_ACTIVATE,
		DEFAULT_NOT_ACTIVATE,
	};
	enum class Behavior_BeforeActivate :unsigned char//激活前行为
	{
		DIRECT,//直接激活，不缓冲【可能丢失激活信号】
		BUFFER,//缓冲，确保所有激活都被保存
	};
	enum class Behavior_AfterActivate :unsigned char//激活后行为
	{
		KEEP,//激活后保持数据
		RESET,//激活后清除数据
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
	QQueue<QVariant>_qdata;//数据队列
	QVariant _defaultData;//默认数据
	QVariantMap additionInfo;//附加信息
	QList<std::function<bool(const AlgVertex*const, const QVariant&)>>inputAssertFunctions;//输入校验。为true才会激活
	QList<std::function<bool(const AlgVertex*const, const AlgVertex*const)>> connectAssertFunctions;//连接校验

	QList<QWeakPointer<AlgVertex>> prevVertexes;//连接到的上一级端口
	QList<QWeakPointer<AlgVertex>> nextVertexes;//连接到的下一级端口

	std::atomic_bool isActivated = false;//激活标志
	std::atomic_bool isEnabled = true;//使能标志
	const VertexType _type;
	Behavior_AfterActivate _behaviorAfter;
	Behavior_BeforeActivate _behaviorBefore;
	Behavior_DefaultActivateState _behaviorDefault;

	QWeakPointer<AlgNode>node;//从属的节点
	QWeakPointer<GuiVertex>gui;
private:
#ifdef _DEBUG
	QString __debugname;//调试用的，方便看名字
#endif // _DEBUG
	static std::atomic_uint64_t _amount;//类实例总数
};

