#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(ALGORITHM_LIB)
#  define ALGORITHM_EXPORT Q_DECL_EXPORT
# else
#  define ALGORITHM_EXPORT Q_DECL_IMPORT
# endif
#else
# define ALGORITHM_EXPORT
#endif

#ifdef _DEBUG
#include <vld.h>
#endif // _DEBUG