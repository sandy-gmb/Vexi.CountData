#ifndef DATAVIEW_GLOBAL_H
#define DATAVIEW_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef DATAVIEW_LIB
# define DATAVIEW_EXPORT Q_DECL_EXPORT
#else
# define DATAVIEW_EXPORT Q_DECL_IMPORT
#endif

#endif // DATAVIEW_GLOBAL_H
