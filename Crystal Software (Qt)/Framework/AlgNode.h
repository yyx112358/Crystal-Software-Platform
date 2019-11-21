#pragma once
#include "global.h"
#include "AlgVertex.h"

#include "GraphError.h"
#include <functional>
#include <atomic>

class GuiNode;

class AlgNode
	:public QObject,private QEnableSharedFromThis<AlgNode>//TODO:��һ�θ��������*/
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
	virtual QSharedPointer<AlgNode>Clone()const { GRAPH_NOT_IMPLEMENT; }
	virtual void Write() { GRAPH_NOT_IMPLEMENT; }
	virtual void Read() { GRAPH_NOT_IMPLEMENT; }

	virtual void Init();//��ʼ��һЩ�����������ʼ����_isUnchange��Ϊtrue�����ܸ����趨
	virtual void Reset();//��������״̬�������������ʱ���������к�ɱ�Ĳ�����
	virtual void Clear() { GRAPH_NOT_IMPLEMENT; }
	
	//�Զ���Ӷ��㣨Ĭ��ΪBUFFER��
	virtual QWeakPointer<AlgVertex> AddVertexAuto(AlgVertex::VertexType vertexType, QString name = "", QVariant defaultData = QVariant());
	virtual QWeakPointer<AlgVertex> AddVertex(AlgVertex::VertexType vertexType, QString name,
		AlgVertex::Behavior_NoData defaultState, AlgVertex::Behavior_BeforeActivate beforeBehavior,
		AlgVertex::Behavior_AfterActivate afterBehavior, QVariant defaultData = QVariant());//���Vertex
	virtual void RemoveVertex(AlgVertex::VertexType vertexType, QString name);//ɾ��Vertex
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
	void sig_RunFinished(QWeakPointer<AlgNode>node);//���н���
	void sig_OutputFinished(QWeakPointer<AlgNode>node);//�������

	void sig_Destroyed(QWeakPointer<AlgNode>wp);
protected:
	AlgNode(QThreadPool&pool = *QThreadPool::globalInstance(), QObject*parent = nullptr);//�����������ֻ����QSharedPointer����
	QList<QSharedPointer<AlgVertex>>&_Vertexes(AlgVertex::VertexType type);
	const QList<QSharedPointer<AlgVertex>>& _Vertexes(AlgVertex::VertexType type)const;
	QSharedPointer<AlgVertex>_FindVertex(AlgVertex::VertexType type, QString name);
	const QSharedPointer<AlgVertex>_FindVertex(AlgVertex::VertexType type, QString name)const;

	virtual QVariantHash _LoadInput();/*�Զ����ȡ���룬Ĭ��ֱ�ӽ����ݴ�Vertex���и���һ�ݣ��������й��������뱻�޸�*/
	virtual QVariantHash _Run(QVariantHash data);//��Ҫ�����в��֣���������һ���߳������С�
	virtual void _LoadOutput(QVariantHash result);/*�Զ�����������Ĭ��ֱ�ӽ����ݴ���ʱ�����м��ص������*/

	QList<QSharedPointer<AlgVertex>>_inputVertex;
	QList<QSharedPointer<AlgVertex>>_outputVertex;
// 	QHash<QString, QWeakPointer<AlgVertex>>_inputVertexSearchTbl;//���ڿ��ٲ��ҵĲ��ұ�//TODO:�������ֱ��˳����һ�ã���Ϊ����һ�㲻��
// 	QHash<QString, QWeakPointer<AlgVertex>>_outputVertexSearchTbl;//���ڿ��ٲ��ҵĲ��ұ�

	QFutureWatcher<QVariantHash> _resultWatcher;//�������й۲���
	QVariantHash _result;//�������н��
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
	time_t __RunTime;//��ʱ�õ�
#endif // _DEBUG
	void SetWeakRef(QWeakPointer<AlgNode>wp) { GRAPH_ASSERT(_weakRef.isNull() == true); _weakRef = wp; }
	QWeakPointer<AlgNode>_weakRef;//�����weakRef�����ڴ���this��������ָ��

	static std::atomic_uint64_t _amount;//��ʵ������
	static std::atomic_uint64_t _runningAmount;
};

