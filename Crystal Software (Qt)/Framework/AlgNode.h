#pragma once
#include "global.h"
#include "AlgVertex.h"

#include "GraphError.h"
#include <functional>
#include <atomic>

class GuiNode;

class AlgNode
	:public QObject,private QEnableSharedFromThis<AlgNode>//TODO:下一次更新用这个*/
{
	Q_OBJECT
	Q_DISABLE_COPY(AlgNode)
public:
	enum class RunMode :unsigned char
	{
		Thread,//多线程方式【默认】
		Direct,//简单直接方式，不开多线程
		Function,//函数方式（Node内部嵌套另一个Graph）
	};
	struct FactoryInfo//用于工厂类的信息
	{
		QString key;//用于唯一标识的key
		QString title;//显示用的标题
		QString description;//描述
		std::function<QSharedPointer<AlgNode>()>defaultConstructor;//默认构造函数
		std::function<QSharedPointer<AlgNode>(QString)>commandConstructor;//命令行形式构造函数

		FactoryInfo(){}
		FactoryInfo(QString key, std::function<QSharedPointer<AlgNode>()>defaultConstructor, QString description = "")
			:key(key), title(key), description(description), defaultConstructor(defaultConstructor){}
		FactoryInfo(QString key,QString title, std::function<QSharedPointer<AlgNode>()>defaultConstructor, QString description = "")
			:key(key), title(title), description(description), defaultConstructor(defaultConstructor) {}
	};
	friend class Interface_Factory;
	friend class QSharedPointer<AlgNode>;

	virtual ~AlgNode();
	virtual QSharedPointer<AlgNode>Clone()const { GRAPH_NOT_IMPLEMENT; }
	virtual void Write() { GRAPH_NOT_IMPLEMENT; }
	virtual void Read() { GRAPH_NOT_IMPLEMENT; }

	virtual void Init();//初始化一些参数，如果初始化后将_isUnchange设为true，则不能更改设定
	virtual void Reset();//重置运行状态，清除所有运行时参数（运行后可变的参数）
	virtual void Clear() { GRAPH_NOT_IMPLEMENT; }
	
	//自动添加顶点（默认为BUFFER）
	virtual QWeakPointer<AlgVertex> AddVertexAuto(AlgVertex::VertexType vertexType, QString name = "", QVariant defaultData = QVariant());
	virtual QWeakPointer<AlgVertex> AddVertex(AlgVertex::VertexType vertexType, QString name,
		AlgVertex::Behavior_NoData defaultState, AlgVertex::Behavior_BeforeActivate beforeBehavior,
		AlgVertex::Behavior_AfterActivate afterBehavior, QVariant defaultData = QVariant());//添加Vertex
	virtual void RemoveVertex(AlgVertex::VertexType vertexType, QString name);//删除Vertex
	virtual bool ConnectVertex(AlgVertex::VertexType vertexType, QString vertexName,
		QSharedPointer<AlgNode>dstNode, AlgVertex::VertexType dstVertexType, QString dstVertexName);
	virtual void DisconnectVertex(AlgVertex::VertexType vertexType, QString vertexName) { GRAPH_NOT_IMPLEMENT; }

	void Activate();
	void Run();
	void Output();
	void Pause(bool isPause) { _pause = isPause; }
	void Stop(bool isStop) { _stop = isStop; }

	void AttachGui(QSharedPointer<GuiNode>gui) { GRAPH_ASSERT(gui.isNull() == false); _gui = gui; }
	virtual QString GetGuiAdvice()const { return QString(); }

	QStringList GetVertexNames(AlgVertex::VertexType type)const;
	QList<QSharedPointer<const AlgVertex>> GetVertexes(AlgVertex::VertexType type)const;
	QSharedPointer<const AlgVertex>GetOneVertex(AlgVertex::VertexType type, QString name)const { return _FindVertex(type, name); }
	QSharedPointer<AlgNode>StrongRef()const { return _weakRef.lock(); }
	QWeakPointer<AlgNode>WeakRef()const { return _weakRef; }

	static size_t GetAmount() { return _amount; }
	static size_t GetRunningAmount() { return _runningAmount; }
signals:
	void sig_ActivateFinished(QWeakPointer<AlgNode>node);
	void sig_RunFinished(QWeakPointer<AlgNode>node);//运行结束
	void sig_OutputFinished(QWeakPointer<AlgNode>node);//输出结束

	void sig_Destroyed(QWeakPointer<AlgNode>wp);
protected:
	AlgNode(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//放在这里表明只能由QSharedPointer构造
	QList<QSharedPointer<AlgVertex>>&_Vertexes(AlgVertex::VertexType type);
	const QList<QSharedPointer<AlgVertex>>& _Vertexes(AlgVertex::VertexType type)const;
	QSharedPointer<AlgVertex>_FindVertex(AlgVertex::VertexType type, QString name);
	const QSharedPointer<AlgVertex>_FindVertex(AlgVertex::VertexType type, QString name)const;

	virtual QVariantHash _LoadInput();/*自定义读取输入，默认直接将数据从Vertex当中复制一份，以免运行过程中输入被修改*/
	virtual QVariantHash _Run(QVariantHash data);//主要的运行部分，【将在另一个线程中运行】
	virtual void _LoadOutput(QVariantHash result);/*自定义加载输出，默认直接将数据从临时数据中加载到输出中*/

	QList<QSharedPointer<AlgVertex>>_inputVertex;
	QList<QSharedPointer<AlgVertex>>_outputVertex;
// 	QHash<QString, QWeakPointer<AlgVertex>>_inputVertexSearchTbl;//用于快速查找的查找表//TODO:或许可以直接顺序查找获得，因为个数一般不多
// 	QHash<QString, QWeakPointer<AlgVertex>>_outputVertexSearchTbl;//用于快速查找的查找表

	QFutureWatcher<QVariantHash> _resultWatcher;//程序运行观测器
	QVariantHash _result;//程序运行结果
	QThreadPool&_pool;//使用的线程池
	QReadWriteLock _lock;//锁【可能并不需要，因为读写参数都发生在主线程】

	RunMode _mode = RunMode::Thread;//运行方式

	std::atomic_bool _isRunning = false;//运行标志。从所有信号确认激活后开始（在_LoadInput()前），在所有输出激活后结束(sig_OutputFinished()前)
	std::atomic_bool _isEnable = true;//使能
	std::atomic_bool _isUnchange = false;//不可在创建后修改
	std::atomic_bool _pause = false;//暂停标志
	std::atomic_bool _stop = false;//结束标志

	QSharedPointer<GuiNode>_gui = nullptr;
private:
#ifdef _DEBUG
	QString __debugname;//调试用的，方便看名字
	time_t __RunTime;//计时用的
#endif // _DEBUG
	void SetWeakRef(QWeakPointer<AlgNode>wp) { GRAPH_ASSERT(_weakRef.isNull() == true); _weakRef = wp; }
	QWeakPointer<AlgNode>_weakRef;//自身的weakRef，用于代替this传递自身指针

	static std::atomic_uint64_t _amount;//类实例总数
	static std::atomic_uint64_t _runningAmount;
};

