#pragma once
#include "global.h"
#include <QMainWindow>

extern "C" 
{
	FRAMEWORK_EXPORT QMainWindow*  GetEntry();//获取程序入口点（实际上就是创建一个Controller）
}
