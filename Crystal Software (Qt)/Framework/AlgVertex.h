#pragma once
#include "global.h"
#include <QObject>
#include <QVariant>
#include <atomic>

class AlgVertex : public QObject
{
	Q_OBJECT

public:
	enum class VertexType :unsigned char
	{
		INPUT,//����
		OUTPUT,//���
	};
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

	AlgVertex(QObject *parent);
	~AlgVertex();

	static size_t GetAmount() { return _amount; }
protected:
	QVariant data;

private:
	static std::atomic_uint64_t _amount;//��ʵ������
};

