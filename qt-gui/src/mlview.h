#ifndef MLVIEW_H
#define MLVIEW_H

#if QT_VERSION >= 300
#include "mlview3.h"
typedef MLViewQt3 MLView;
#else
#include "mlview2.h"
typedef MLViewQt2 MLView;
#endif

#endif
