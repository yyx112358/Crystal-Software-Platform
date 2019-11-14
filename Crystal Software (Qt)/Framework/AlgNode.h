#pragma once
#include "global.h"
#include "AlgVertex.h"

#include "GraphError.h"
#include <functional>
#include <atomic>

class GuiNode;

class AlgNode
	:public QObject
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
	QSharedPointer<AlgNode>clone()const;

	virtual void Init() {}//初始化一些参数，如果初始化后将_isUnchange设为true，则不能更改设定
	virtual void Reset() {}//重置运行状态，清除所有运行时参数（运行后可变的参数）
	virtual void Clear() {}
	virtual void Release() {}
	virtual QString GetGuiAdvice()const { return QString(); }

	void AttachGui(QSharedPointer<GuiNode>gui) { GRAPH_ASSERT(gui.isNull() == false); _gui = gui; }
	QWeakPointer<AlgNode>WeakRef()const { return _weakRef; }

	static size_t GetAmount() { return _amount; }
	static size_t GetRunningAmount() { return _runningAmount; }

signals:
	void sig_Destroyed(QWeakPointer<AlgNode>wp);
protected:
	AlgNode(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//放在这里表明只能由QSharedPointer构造

	QList<QSharedPointer<AlgVertex>>_inputVertex;
	QList<QSharedPointer<AlgVertex>>_outputVertex;
	QHash<QString, QWeakPointer<AlgVertex>>_inputVertexNamedTbl;//用于快速查找的查找表
	QHash<QString, QWeakPointer<AlgVertex>>_outputVertexNamedTbl;//用于快速查找的查找表

	QThreadPool&_pool;//使用的线程池

	QSharedPointer<GuiNode>_gui = nullptr;
private:
#ifdef _DEBUG
	QString name;//调试用的，方便看名字
#endif // _DEBUG
	void SetWeakRef(QWeakPointer<AlgNode>wp) { _weakRef = wp; }
	QWeakPointer<AlgNode>_weakRef;//自身的weakRef，用于

	static std::atomic_uint64_t _amount;//类实例总数
	static std::atomic_uint64_t _runningAmount;
};

