#include "licq_gtk.h"

#include "pixmaps/online.xpm"
#include "pixmaps/offline.xpm"
#include "pixmaps/away.xpm"
#include "pixmaps/na.xpm"
#include "pixmaps/occ.xpm"
#include "pixmaps/dnd.xpm"
#include "pixmaps/message.xpm"

#include <ctype.h>
#include <gtk/gtk.h>

void do_colors()
{
        red = new GdkColor;
        green = new GdkColor;
        blue = new GdkColor;
 
        red->red = 30000;
        red->green = 0;
        red->blue = 0;
        red->pixel = (gulong)(255);
 
        green->red = 0;
        green->green = 30000;
        green->blue = 0;
        green->pixel = (gulong)(255*256);
 
        blue->red = 0;
        blue->green = 0;
        blue->blue = 30000;
        blue->pixel = (gulong)(255*256); 
}

void do_pixmaps()
{
        style = gtk_widget_get_style(main_window);
 
        online = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                              &style->bg[GTK_STATE_NORMAL],
                                              (gchar **)online_xpm);
 
        away = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                            &style->bg[GTK_STATE_NORMAL],
                                            (gchar **)away_xpm);
 
        na = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                          &style->bg[GTK_STATE_NORMAL],
                                          (gchar **)na_xpm);
 
        dnd = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                           &style->bg[GTK_STATE_NORMAL],
                                           (gchar **)dnd_xpm);
 
        occ = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                           &style->bg[GTK_STATE_NORMAL],
                                           (gchar **)occ_xpm); 

        offline = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                               &style->bg[GTK_STATE_NORMAL],
                                               (gchar **)offline_xpm);
 
        message = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                               &style->bg[GTK_STATE_NORMAL],
                                               (gchar **)message_xpm); 
}

void verify_numbers(GtkEditable *e, gchar *text, gint len, gint *pos, gpointer d)
{
	int i;
   	gchar *result = g_new(gchar, len);

   	for (i=0; i<len; i++)
	{
		if(!isdigit(text[i]))
			result[i] = NULL;
		else
			result[i] = text[i];
	}

	gtk_signal_handler_block_by_func(GTK_OBJECT(e),
                                         GTK_SIGNAL_FUNC(verify_numbers),
                                         d);

	gtk_editable_insert_text(GTK_EDITABLE(e), result, len, pos);

	gtk_signal_handler_unblock_by_func(GTK_OBJECT(e),
                                           GTK_SIGNAL_FUNC(verify_numbers),
                                       	   d);

	gtk_signal_emit_stop_by_name(GTK_OBJECT(e), "insert_text");

	g_free(result);
}
