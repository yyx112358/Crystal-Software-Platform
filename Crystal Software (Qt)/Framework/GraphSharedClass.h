#pragma once
#include <QSharedPointer>
//�����this��������QSharedPointer
//��ΪQEnableSharedFromThis���ܷ����ڲ���QWeakPointer����ֻ��QEnableSharedFromThis������QSharedPointer::create()���оͳ�ʼ��weakPointer����˲��ò�������
#define GRAPH_ENABLE_SHARED(Tp) \
public:\
	friend class QSharedPointer<Tp>;\
	inline QSharedPointer<Tp>StrongRef() { return sharedFromThis(); }\
	inline QSharedPointer<const Tp>StrongRef()const { return sharedFromThis(); }\
	inline QWeakPointer<Tp>WeakRef() { return _weakRef; }\
	inline QWeakPointer<const Tp>WeakRef()const { return _weakRef; }\
	void SetSelfPointer() { if (sharedFromThis().isNull() == false) _weakRef = sharedFromThis(); }\
private:\
	mutable QWeakPointer<Tp>_weakRef;/*�����weakRef�����ڴ���this��������ָ��*/
