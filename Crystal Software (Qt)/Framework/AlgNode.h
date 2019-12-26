#pragma once
#include "global.h"
#include "AlgVertex.h"

#include "GraphError.h"
#include <functional>
#include <atomic>

class GuiNode;

class AlgNode
	:public QObject, public QEnableSharedFromThis<AlgNode>
{
	Q_OBJECT
	GRAPH_SHARED_BASE_QOBJECT(AlgNode)
public:
	enum class RunMode :unsigned char
	{
		Thread,//多线程方式【默认】
		Direct,//简单直接方式，不开多线程
		Function,//函数方式（Node内部嵌套另一个Graph）
	};
	enum class Category//TODO:分类
	{
		BASIC,//基类
		ALGORITHM,//算法
		FUNCTION,//函数
		INPUT,//输入
		OUTPUT,//输出
		CONSTANT,//常量
		OPERATION,//运算符：四则运算、逻辑运算……
		CONTROL,//流程控制：条件、循环、延时、同步、派发、组合
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

	virtual ~AlgNode();
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
	virtual bool RemoveVertex(AlgVertex::VertexType vertexType, QString name);//删除Vertex
	virtual bool ConnectVertex(AlgVertex::VertexType vertexType, QString vertexName,
		QSharedPointer<AlgNode>dstNode, AlgVertex::VertexType dstVertexType, QString dstVertexName);
	virtual void DisconnectVertex(AlgVertex::VertexType vertexType, QString vertexName,
		QSharedPointer<AlgNode>dstNode, AlgVertex::VertexType dstVertexType, QString dstVertexName);

	void Activate();
	void Run();
	void Output();
	void Pause(bool isPause) { _pause = isPause; }
	void Stop(bool isStop) { _stop = isStop; }

	void AttachGui(QSharedPointer<GuiNode>gui) { GRAPH_ASSERT(gui.isNull() == false); _gui = gui; }
	QSharedPointer<GuiNode>GetGui() { return _gui; }
	QSharedPointer<const GuiNode>GetGui()const { return _gui; }
	virtual QString GetGuiAdvice()const { return QString(); }

	virtual QCommandLineParser& GetCommandLineParser()const {GRAPH_NOT_IMPLEMENT;}//TODO:命令行解析，也可以用来生成菜单或者进行存储
	virtual bool ExecCommaneLine(const QStringList &arguments){GRAPH_NOT_IMPLEMENT;} //处理命令行
	virtual void ProcessAction(QString action, bool isChecked);//TODO:暂时性的action处理函数，之后用ExecCommaneLine()替代

	QStringList GetVertexNames(AlgVertex::VertexType type)const;
	QList<QSharedPointer<const AlgVertex>> GetVertexes(AlgVertex::VertexType type)const;
	QSharedPointer<const AlgVertex>GetOneVertex(AlgVertex::VertexType type, QString name)const { return _FindVertex(type, name); }

	bool IsRunning()const { return _isRunning; }
	bool IsEnable()const { return _isEnable; }
	bool IsUnchange()const { return _isUnchange; }
	bool IsPause()const { return _pause; }
	bool IsStop()const { return _stop; }
	virtual Category GetCategory()const { return Category::BASIC; }
	RunMode GetRunMode()const { return _mode; }

	QVariantHash GetUserProperty()const { return _userProperty; }
	void SetUserProperty(QVariantHash property) { _userProperty = property; }

	
	static size_t GetRunningAmount() { return _runningAmount; }
signals:
	void sig_ResetFinished(QSharedPointer<AlgNode>node);//重置结束

	void sig_ActivateFinished(QSharedPointer<AlgNode>node);
	void sig_RunFinished(QSharedPointer<AlgNode>node);//运行结束
	void sig_OutputFinished(QSharedPointer<AlgNode>node);//输出结束

	void sig_Destroyed(QWeakPointer<AlgNode>wp);//删除信号
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

	QVariantHash _userProperty;//自定义信息

	mutable QSharedPointer<GuiNode>_gui = nullptr;
private:
	static std::atomic_uint64_t _runningAmount;
};

