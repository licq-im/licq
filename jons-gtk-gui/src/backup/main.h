#ifndef __MAIN_H
#define __MAIN_H

#include "icqd.h"

#include <gtk/gtk.h>

#define NAME		"GTK plugin"
#define VERSION		"0.0.1"
#define STATUS		"Running"
#define USAGE		"None yet"
#define DESCRIPTION	"GTK plugin for licq"

GtkWidget *main_window;
CICQDaemon *icq_daemon;
gint _Pipe;

/* From mainwindow.h */
extern GtkWidget* main_window_new( const gchar *window_title,
				   gint height,
				   gint width );
extern void main_window_show();

/* From pipe.h */
extern void pipe_callback( gpointer data,
			   gint pipe,
			   GdkInputCondition condition );

#endif /* __MAIN_H */
