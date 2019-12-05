#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(FRAMEWORK_LIB)
#  define FRAMEWORK_EXPORT Q_DECL_EXPORT
# else
#  define FRAMEWORK_EXPORT Q_DECL_IMPORT
# endif
#else
# define FRAMEWORK_EXPORT
#endif

#define SOFTWARE_VERSION 0x000001//版本号（MM = major, NN = minor, PP = patch）
#define NAME2STR(name) #name//把表达式转变为字符串

#ifdef _DEBUG
#define GRAPH_SHARED_BASE_QOBJECT(Tp)\
	public:\
	friend class QSharedPointer<Tp>;\
	virtual QSharedPointer<Tp>Clone()const;\
	static uint64_t GetAmount(){return _amount;}\
	private:\
	Q_DISABLE_COPY(Tp)\
	QString __debugName;/*方便调试的名字，需要在构造函数加入connect(this, &QObject::objectNameChanged, [this](QString str) {__debugname = str; });*/\
	std::atomic_uint64_t __RunTime;/*计时用的*/\
	static std::atomic_uint64_t _amount;/*类实例个数*/

#else
#define GRAPH_SHARED_BASE_QOBJECT(Tp)\
	public:\
	friend class QSharedPointer<Tp>;\
	virtual QSharedPointer<Tp>Clone()const;\
	static uint64_t GetAmount(){return _amount;}\
	private:\
	Q_DISABLE_COPY(Tp)\
	static std::atomic_uint64_t _amount;/*类实例个数*/

#endif // DEBUG




enum GuiType
{
	GuiType_Node = 0x10000 + 0x1000,
	GuiType_Vertex = 0x10000 + 0x2000,
	GuiType_Connection = 0x10000 + 0x3000,
};