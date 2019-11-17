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
		Thread,//���̷߳�ʽ��Ĭ�ϡ�
		Direct,//��ֱ�ӷ�ʽ���������߳�
		Function,//������ʽ��Node�ڲ�Ƕ����һ��Graph��
	};
	struct FactoryInfo//���ڹ��������Ϣ
	{
		QString key;//����Ψһ��ʶ��key
		QString title;//��ʾ�õı���
		QString description;//����
		std::function<QSharedPointer<AlgNode>()>defaultConstructor;//Ĭ�Ϲ��캯��
		std::function<QSharedPointer<AlgNode>(QString)>commandConstructor;//��������ʽ���캯��

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
	virtual void Write(){}
	virtual void Read(){}

	virtual void Init();//��ʼ��һЩ�����������ʼ����_isUnchange��Ϊtrue�����ܸ����趨
	virtual void Reset() {}//��������״̬�������������ʱ���������к�ɱ�Ĳ�����
	virtual void Clear() {}
	virtual void Release() {}


	virtual QWeakPointer<AlgVertex> AddVertex(AlgVertex::VertexType vertexType, QString name,
		AlgVertex::Behavior_DefaultActivateState defaultState, AlgVertex::Behavior_BeforeActivate beforeBehavior,
		AlgVertex::Behavior_AfterActivate afterBehavior, QVariant defaultValue = QVariant());
	virtual QWeakPointer<AlgVertex> AddVertexAuto(AlgVertex::VertexType vertexType, QString name = "");

	void AttachGui(QSharedPointer<GuiNode>gui) { GRAPH_ASSERT(gui.isNull() == false); _gui = gui; }
	virtual QString GetGuiAdvice()const { return QString(); }

	QStringList GetVertexNames(AlgVertex::VertexType type)const;
	QSharedPointer<AlgNode>StrongRef()const { return _weakRef.lock(); }
	QWeakPointer<AlgNode>WeakRef()const { return _weakRef; }

	static size_t GetAmount() { return _amount; }
	static size_t GetRunningAmount() { return _runningAmount; }
signals:
	void sig_Destroyed(QWeakPointer<AlgNode>wp);
protected:
	AlgNode(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//�����������ֻ����QSharedPointer����


	QList<QSharedPointer<AlgVertex>>_inputVertex;
	QList<QSharedPointer<AlgVertex>>_outputVertex;
// 	QHash<QString, QWeakPointer<AlgVertex>>_inputVertexSearchTbl;//���ڿ��ٲ��ҵĲ��ұ�//TODO:�������ֱ��˳����һ�ã���Ϊ����һ�㲻��
// 	QHash<QString, QWeakPointer<AlgVertex>>_outputVertexSearchTbl;//���ڿ��ٲ��ҵĲ��ұ�

	QFutureWatcher<QVariantHash> _result;//�������й۲���
	QThreadPool&_pool;//ʹ�õ��̳߳�
	QReadWriteLock _lock;//�������ܲ�����Ҫ����Ϊ��д���������������̡߳�

	RunMode _mode = RunMode::Thread;//���з�ʽ

	std::atomic_bool _isRunning = false;//���б�־���������ź�ȷ�ϼ����ʼ����_LoadInput()ǰ���������������������(sig_OutputFinished()ǰ)
	std::atomic_bool _isEnable = true;//ʹ��
	std::atomic_bool _isUnchange = false;//�����ڴ������޸�
	std::atomic_bool _pause = false;//��ͣ��־
	std::atomic_bool _stop = false;//������־

	QSharedPointer<GuiNode>_gui = nullptr;
private:
#ifdef _DEBUG
	QString __debugname;//�����õģ����㿴����
#endif // _DEBUG
	void SetWeakRef(QWeakPointer<AlgNode>wp) { _weakRef = wp; }
	QWeakPointer<AlgNode>_weakRef;//�����weakRef�����ڴ���this��������ָ��

	static std::atomic_uint64_t _amount;//��ʵ������
	static std::atomic_uint64_t _runningAmount;
};

