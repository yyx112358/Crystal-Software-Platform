#pragma once
#include <QSharedPointer>
//允许从this获得自身的QSharedPointer
//因为QEnableSharedFromThis不能访问内部的QWeakPointer，而只有QEnableSharedFromThis才能在QSharedPointer::create()当中就初始化weakPointer，因此不得不添加这个
#define GRAPH_ENABLE_SHARED(Tp) \
public:\
	friend class QSharedPointer<Tp>;\
	inline QSharedPointer<Tp>StrongRef() { return sharedFromThis(); }\
	inline QSharedPointer<const Tp>StrongRef()const { return sharedFromThis(); }\
	inline QWeakPointer<Tp>WeakRef() { return _weakRef; }\
	inline QWeakPointer<const Tp>WeakRef()const { return _weakRef; }\
	void SetSelfPointer() { if (sharedFromThis().isNull() == false) _weakRef = sharedFromThis(); }\
private:\
	mutable QWeakPointer<Tp>_weakRef;/*自身的weakRef，用于代替this传递自身指针*/
