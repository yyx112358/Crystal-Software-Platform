#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(PLUGINSTATICTOOL_LIB)
#  define PLUGINSTATICTOOL_EXPORT Q_DECL_EXPORT
# else
#  define PLUGINSTATICTOOL_EXPORT Q_DECL_IMPORT
# endif
#else
# define PLUGINSTATICTOOL_EXPORT
#endif
