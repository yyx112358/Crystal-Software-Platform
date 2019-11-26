#pragma once
#include "global.h"
#include "GraphError.h"
#include "GraphSharedClass.h"
#include <QObject>
#include <QVariant>
#include <atomic>

class AlgNode;
class GuiVertex;

const size_t AlgVertex_MaxBufferSize = 256;

class AlgVertex : public QObject, private QEnableSharedFromThis<AlgVertex>
{
	Q_OBJECT
	Q_DISABLE_COPY(AlgVertex)
	GRAPH_ENABLE_SHARED(AlgVertex)
public:
	enum class VertexType :unsigned char
	{
		INPUT,//输入
		OUTPUT,//输出
		//ENABLE,//使能
		//TRIGGER,//触发
	};
	friend QDebug& operator<<(QDebug&qd, VertexType vt) { qd << ((vt == AlgVertex::VertexType::INPUT) ? "INPUT" : "OUTPUT");	return qd; }
	enum class Behavior_NoData :unsigned char//决定当无数据时GetData()的返回值
	{
		USE_NULL,
		USE_NULL_FIRST_DEFAULT,
		USE_LAST,
		USE_LAST_FIRST_DEFAULT,
		USE_DEFAULT,
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


	static QSharedPointer<AlgVertex>Create(QWeakPointer<AlgNode>parent, VertexType type, QString name, Behavior_NoData defaultState,
		Behavior_BeforeActivate beforeBehavior, Behavior_AfterActivate afterBehavior, QVariant defaultData = QVariant());
	bool RemoveFromParent();
	~AlgVertex();


	//《激活函数》如果使能（isEnable==true）首先调用所有的assertFunction做校验，通过后进行下一步激活
	void Activate(QVariant var/*, bool isNow = true*/);
	void slot_ActivateSuccess();//成功激活之后的后处理（通常，INPUT由NODE的sig_ActivateFinished()调用，OUTPUT由自身的sig_ActivateEnd()调用）
	//连接两个节点，方向this=>dstVertex，主要是修改nextVertexes和dstVertex->prevVertexes并连接this=>dstVertex的激活信号
	void Connect(QSharedPointer<AlgVertex>dstVertex);
	void Disconnect(QWeakPointer<AlgVertex>another);
	
	void Reset();//重置，主要是清除运行时状态，准备好全图的下一次运行（图运行后会改变的，主要是数据及激活位）
	void Clear();//清除，清除运行时状态和动态状态（指调节图时候可变的，主要是使能位isEnabled和连接节点）	
	QStringList Write()const { throw "Not Implement"; }//TODO:持久化，保存节点信息和结构（*是否保存数据？）
	void Read(QStringList) { throw "Not Implement"; }//TODO:从持久化信息中恢复
	//QString GetGuiAdvice()const { return "normal"; }//TODO:对工厂类给出的GUI建议，可能采用类似命令行的方式
	
	inline bool IsActivated()const { return _qdata.size() > 0; }
	QVariant GetData()const;
	//QVariant TakeData()
	AlgVertex::Behavior_AfterActivate GetBehaviorAfter() const { return _behaviorAfter; }
	void SetBehaviorAfter(AlgVertex::Behavior_AfterActivate val) { _behaviorAfter = val; }
	AlgVertex::Behavior_BeforeActivate GetBehaviorBefore() const { return _behaviorBefore; }
	void SetBehaviorBefore(AlgVertex::Behavior_BeforeActivate val) { _behaviorBefore = val; }

	void AttachGui(QSharedPointer<GuiVertex>gui) { GRAPH_ASSERT(gui.isNull() == false); _gui = gui; }
	QWeakPointer<GuiVertex>GetGui()const { return _gui; }

	static size_t GetAmount() { return _amount; }
	const VertexType type;

signals:
	void sig_ActivateBegin();//激活开始
	void sig_Activated(QVariant var/*, bool is_Activated*/);//激活信号，可用来激活下一个节点
	void sig_ActivateEnd();//激活结束【不一定激活成功】

	void sig_ConnectionAdded(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst);//连接建立成功
	void sig_ConnectionRemoved(QWeakPointer<const AlgVertex>src, QWeakPointer<const AlgVertex>dst);//连接移除成功

	void sig_Destroyed(QWeakPointer<AlgNode>node, QWeakPointer<AlgVertex>vertex);//删除成功
protected:
	AlgVertex(QWeakPointer<AlgNode>parent, VertexType type, QString name, Behavior_NoData defaultState,
		Behavior_BeforeActivate beforeBehavior, Behavior_AfterActivate afterBehavior, QVariant defaultData = QVariant());

	QVariant _lastdata;
	QQueue<QVariant>_qdata;//数据队列，保证Activate()信号不丢失
	QVariant _defaultData;//默认数据
	QVariantMap _additionInfo;//附加信息
	QList<std::function<bool(/*const AlgVertex*const, */const QVariant&)>>inputAssertFunctions;//输入校验。为true才会激活//TODO:后面可能会改成带名字和std::function的自定义类
	QList<std::function<bool(/*const AlgVertex*const, */const QSharedPointer<const AlgVertex>)>> connectAssertFunctions;//连接校验

	QList<QWeakPointer<AlgVertex>> _prevVertexes;//连接到的上一级端口
	QList<QWeakPointer<AlgVertex>> _nextVertexes;//连接到的下一级端口

	//std::atomic_bool _isActivated = false;//激活标志
	std::atomic_bool _isEnabled = true;//使能标志
	
	Behavior_AfterActivate _behaviorAfter;
	Behavior_BeforeActivate _behaviorBefore;
	Behavior_NoData _behaviorNoData;

	QWeakPointer<AlgNode>_node;//从属的节点
	mutable QWeakPointer<GuiVertex>_gui;//对应的GUI
private:
#ifdef _DEBUG
	QString __debugname;//调试用的，方便看名字
#endif // _DEBUG	

	static std::atomic_uint64_t _amount;//类实例总数
};

