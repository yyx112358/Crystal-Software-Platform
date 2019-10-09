#pragma once

#include "algorithm_global.h"
#include <QVariant>
#include <QThreadPool>
#include <QFlags>

class Interface_Alg
	:public QObject
{
	Q_OBJECT
public:
	Interface_Alg(QObject*parent):QObject(parent){}

	void Init();
	void Reset();
	void Release();

	void Run();
	void Pause();
	void Stop();
protected:
	virtual void _AddChildController()=0;
	virtual void _WaitForChildController() = 0;

	QThreadPool*_threadPool = nullptr;
private:

};
