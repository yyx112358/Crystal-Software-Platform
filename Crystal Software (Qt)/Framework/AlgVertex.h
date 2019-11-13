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
		INPUT,//输入
		OUTPUT,//输出
	};
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

	AlgVertex(QObject *parent);
	~AlgVertex();

	static size_t GetAmount() { return _amount; }
protected:
	QVariant data;

private:
	static std::atomic_uint64_t _amount;//类实例总数
};

