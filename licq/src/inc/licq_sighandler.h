#ifndef LICQSEGV_H
#define LICQSEGV_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <signal.h>

void licq_segv_handler(void (*f)(int /*, siginfo_t *, void **/));

void signal_handler_eventThread(int /*, siginfo_t *, void * */);
void signal_handler_chatThread(int /*, siginfo_t *, void * */);
void signal_handler_pingThread(int /*, siginfo_t *, void * */);
void signal_handler_managerThread(int /*, siginfo_t *, void * */);
void signal_handler_monitorThread(int /*, siginfo_t *, void * */);

void licq_handle_sigsegv(const char * /*, siginfo_t *, void * */);

#ifdef __cplusplus
}
#endif

#endif


