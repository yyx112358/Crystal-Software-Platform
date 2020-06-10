#pragma once

#include <QVariant>

class Interface_ParamWatcher
{
public:
	virtual bool Reset() = 0;
	virtual bool AppendParam(QVariant param) = 0;
	virtual bool AppendParam(QList<QVariant>params) = 0;
	virtual QVariant GetParam(int idx)const = 0;
	virtual QList<QVariant> GetParam()const = 0;

	virtual int type()const = 0;
	virtual bool IsSupportType(int type)const = 0;

	virtual void SetIsSave(bool isSave, int maximum = 2147483647) = 0;
};