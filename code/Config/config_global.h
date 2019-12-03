#ifndef CONFIG_GLOBAL_H
#define CONFIG_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef CONFIG_LIB
# define CONFIG_EXPORT Q_DECL_EXPORT
#else
# define CONFIG_EXPORT Q_DECL_IMPORT
#endif

#endif // CONFIG_GLOBAL_H
