#ifndef TIMEFIX_H
#define TIMEFIX_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif //HAVE_SYS_TIME_H
#endif //TIME_WITH_SYS_TIME

#endif
