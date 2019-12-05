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

#define SOFTWARE_VERSION 0x000001//�汾�ţ�MM = major, NN = minor, PP = patch��
#define NAME2STR(name) #name//�ѱ��ʽת��Ϊ�ַ���

#ifdef _DEBUG
#define GRAPH_SHARED_BASE_QOBJECT(Tp)\
	public:\
	friend class QSharedPointer<Tp>;\
	virtual QSharedPointer<Tp>Clone()const;\
	static uint64_t GetAmount(){return _amount;}\
	private:\
	Q_DISABLE_COPY(Tp)\
	QString __debugName;/*������Ե����֣���Ҫ�ڹ��캯������connect(this, &QObject::objectNameChanged, [this](QString str) {__debugname = str; });*/\
	std::atomic_uint64_t __RunTime;/*��ʱ�õ�*/\
	static std::atomic_uint64_t _amount;/*��ʵ������*/

#else
#define GRAPH_SHARED_BASE_QOBJECT(Tp)\
	public:\
	friend class QSharedPointer<Tp>;\
	virtual QSharedPointer<Tp>Clone()const;\
	static uint64_t GetAmount(){return _amount;}\
	private:\
	Q_DISABLE_COPY(Tp)\
	static std::atomic_uint64_t _amount;/*��ʵ������*/

#endif // DEBUG




enum GuiType
{
	GuiType_Node = 0x10000 + 0x1000,
	GuiType_Vertex = 0x10000 + 0x2000,
	GuiType_Connection = 0x10000 + 0x3000,
};