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

enum GuiType
{
	GuiType_Node = 0x10000 + 0x1000,
	GuiType_Vertex = 0x10000 + 0x2000,
	GuiType_Connection = 0x10000 + 0x3000,
};