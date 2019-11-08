#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(PLUGINTOOL_LIB)
#  define PLUGINTOOL_EXPORT Q_DECL_EXPORT
# else
#  define PLUGINTOOL_EXPORT Q_DECL_IMPORT
# endif
#else
# define PLUGINTOOL_EXPORT
#endif
