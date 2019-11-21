#pragma once
#include "global.h"
#include "GraphError.h"
#include <QObject>
#include <QVariant>
#include <atomic>

class AlgNode;
class GuiVertex;

const size_t AlgVertex_MaxBufferSize = 256;

class AlgVertex : public QObject
{
	Q_OBJECT

public:
	enum class VertexType :unsigned char
	{
		INPUT,//����
		OUTPUT,//���
		//ENABLE,//ʹ��
		//TRIGGER,//����
	};
	friend QDebug& operator<<(QDebug&qd, VertexType vt) { qd << ((vt == AlgVertex::VertexType::INPUT) ? "INPUT" : "OUTPUT");	return qd; }
// 	enum class Behavior_DefaultActivateState :unsigned char//Ĭ�ϼ���״̬
// 	{
// 		DEFAULT_ACTIVATE,
// 		DEFAULT_NOT_ACTIVATE,
// 	};
	enum class Behavior_NoData :unsigned char//������������ʱGetData()�ķ���ֵ
	{
		USE_NULL,
		USE_NULL_FIRST_DEFAULT,
		USE_LAST,
		USE_LAST_FIRST_DEFAULT,
		USE_DEFAULT,
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


	static QSharedPointer<AlgVertex>Create(QWeakPointer<AlgNode>parent, VertexType type, QString name, Behavior_NoData defaultState,
		Behavior_BeforeActivate beforeBehavior, Behavior_AfterActivate afterBehavior, QVariant defaultData = QVariant());
	~AlgVertex();


	//������������ʹ�ܣ�isEnable==true�����ȵ������е�assertFunction��У�飬ͨ���������һ������
	void Activate(QVariant var/*, bool isNow = true*/);
	void slot_ActivateSuccess();//�ɹ�����֮��ĺ���ͨ����INPUT��NODE��sig_ActivateFinished()���ã�OUTPUT�������sig_ActivateEnd()���ã�
	//���������ڵ㣬����this=>dstVertex����Ҫ���޸�nextVertexes��dstVertex->prevVertexes������this=>dstVertex�ļ����ź�
	void Connect(QSharedPointer<AlgVertex>dstVertex);
	void Disconnect(QSharedPointer<AlgVertex>another);
	
	void Reset();//���ã���Ҫ���������ʱ״̬��׼����ȫͼ����һ�����У�ͼ���к��ı�ģ���Ҫ�����ݼ�����λ��
	void Clear();//������������ʱ״̬�Ͷ�̬״̬��ָ����ͼʱ��ɱ�ģ���Ҫ��ʹ��λisEnabled�����ӽڵ㣩	
	QStringList Write()const { throw "Not Implement"; }//TODO:�־û�������ڵ���Ϣ�ͽṹ��*�Ƿ񱣴����ݣ���
	void Read(QStringList) { throw "Not Implement"; }//TODO:�ӳ־û���Ϣ�лָ�
	//QString GetGuiAdvice()const { return "normal"; }//TODO:�Թ����������GUI���飬���ܲ������������еķ�ʽ
	
	inline bool IsActivated()const { return _qdata.size() > 0; }
	QVariant GetData()const;
	//QVariant TakeData()
	AlgVertex::Behavior_AfterActivate GetBehaviorAfter() const { return _behaviorAfter; }
	void SetBehaviorAfter(AlgVertex::Behavior_AfterActivate val) { _behaviorAfter = val; }
	AlgVertex::Behavior_BeforeActivate GetBehaviorBefore() const { return _behaviorBefore; }
	void SetBehaviorBefore(AlgVertex::Behavior_BeforeActivate val) { _behaviorBefore = val; }
// 	AlgVertex::Behavior_DefaultActivateState GetBehaviorDefault() const { return _behaviorDefault; }
// 	void SetBehaviorDefault(AlgVertex::Behavior_DefaultActivateState val) { _behaviorDefault = val; }

	void AttachGui(QSharedPointer<GuiVertex>gui) { GRAPH_ASSERT(gui.isNull() == false); _gui = gui; }
	QWeakPointer<GuiVertex>GetGui()const { return _gui; }
	QWeakPointer<AlgVertex>WeakRef()const { return _weakRef; }

	static size_t GetAmount() { return _amount; }
	const VertexType type;

signals:
	void sig_ActivateBegin();//���ʼ
	void sig_Activated(QVariant var/*, bool is_Activated*/);//�����źţ�������������һ���ڵ�
	void sig_ActivateEnd();//�����������һ������ɹ���

	void sig_ConnectionAdded(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst);//���ӽ����ɹ�
	void sig_ConnectionRemoved(QSharedPointer<const AlgVertex>src, QSharedPointer<const AlgVertex>dst);//�����Ƴ��ɹ�

	void sig_Destroyed(QWeakPointer<AlgNode>node, QWeakPointer<AlgVertex>vertex);//ɾ���ɹ�
protected:
	AlgVertex(QWeakPointer<AlgNode>parent, VertexType type, QString name, Behavior_NoData defaultState,
		Behavior_BeforeActivate beforeBehavior, Behavior_AfterActivate afterBehavior, QVariant defaultData = QVariant());

	QVariant _lastdata;
	QQueue<QVariant>_qdata;//���ݶ��У���֤Activate()�źŲ���ʧ
	QVariant _defaultData;//Ĭ������
	QVariantMap additionInfo;//������Ϣ
	QList<std::function<bool(/*const AlgVertex*const, */const QVariant&)>>inputAssertFunctions;//����У�顣Ϊtrue�Żἤ��//TODO:������ܻ�ĳɴ����ֺ�std::function���Զ�����
	QList<std::function<bool(/*const AlgVertex*const, */const QSharedPointer<const AlgVertex>)>> connectAssertFunctions;//����У��

	QList<QWeakPointer<AlgVertex>> _prevVertexes;//���ӵ�����һ���˿�
	QList<QWeakPointer<AlgVertex>> _nextVertexes;//���ӵ�����һ���˿�

	//std::atomic_bool _isActivated = false;//�����־
	std::atomic_bool _isEnabled = true;//ʹ�ܱ�־
	
	Behavior_AfterActivate _behaviorAfter;
	Behavior_BeforeActivate _behaviorBefore;
	Behavior_NoData _behaviorNoData;

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

