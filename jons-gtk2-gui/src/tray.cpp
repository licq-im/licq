/* Licq - ICQ client
 * tray.cpp copied and modified from 
 * GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>

#include "licq_gtk.h"
#include "eggtrayicon.h"

#include <iostream>

extern GtkWidget *main_window;

int tray_clicked(GtkWidget *widget, GdkEventButton *event, gpointer)
{
  g_return_val_if_fail (event != NULL, false);
	static gint winX = 1, winY = 1;

  bool visible = GTK_WIDGET_VISIBLE (GTK_WIDGET (main_window));

  if (visible) {
		gtk_window_get_position(GTK_WINDOW(main_window), &winX, &winY);
    gtk_widget_hide(main_window);
  }
  else {
    gtk_widget_show(main_window);
		gtk_window_move(GTK_WINDOW(main_window), winX, winY);
  }

  return false;
}

static void licq_build_tray(GtkContainer *tray_icon)
{
  GtkWidget *image;

	image = gtk_image_new_from_pixbuf(online);
  GtkWidget *eventbox = gtk_event_box_new();
  
  gtk_widget_set_events(GTK_WIDGET(eventbox), 
			 gtk_widget_get_events(eventbox) 
			 | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);
  
  g_signal_connect(G_OBJECT(eventbox), "button_press_event",
		    G_CALLBACK(tray_clicked), 0);
  
  gtk_widget_show(image);
  gtk_widget_show(eventbox);
  
  /* add the status to the plug */
  g_object_set_data(G_OBJECT(tray_icon), "image", image);
  g_object_set_data(G_OBJECT(tray_icon), "icon", online);
  g_object_set_data(G_OBJECT(tray_icon), "flash", GINT_TO_POINTER(1));
  gtk_container_add(GTK_CONTAINER(eventbox), image);
  gtk_container_add(tray_icon, eventbox);
}

void licq_tray_show(GObject *tray);
void licq_tray_hide(GObject *tray);

/* The functions */
GObject *licq_init_tray()
{
  EggTrayIcon *tray_icon;

  tray_icon = egg_tray_icon_new("Licq Tray Icon");

  licq_build_tray(GTK_CONTAINER(tray_icon));
  
  licq_tray_show(G_OBJECT(tray_icon));

  return G_OBJECT(tray_icon);
}

void licq_tray_set_content(GObject *tray, GdkPixbuf *icon, int flash_on)
{
  gpointer image = g_object_get_data(tray, "image");
	if (icon != NULL) {
  	gtk_image_set_from_pixbuf(GTK_IMAGE(image), icon);
	  g_object_set_data(G_OBJECT(tray), "icon", icon);
	}
	else {
		if (flash_on) {
			GdkPixbuf *saved_icon = (GdkPixbuf *)g_object_get_data(tray, "icon");
  		gtk_image_set_from_pixbuf(GTK_IMAGE(image), saved_icon);
			g_object_set_data(G_OBJECT(tray), "flash", GINT_TO_POINTER(1));
		}
		else {
	  	gtk_image_set_from_pixbuf(GTK_IMAGE(image), blank_icon);
			g_object_set_data(G_OBJECT(tray), "flash", GINT_TO_POINTER(0));
		}
	}
}

void licq_tray_show(GObject *tray)
{
  gtk_widget_show(GTK_WIDGET(tray));
}


void licq_tray_hide(GObject *tray)
{
  gtk_widget_hide(GTK_WIDGET(tray));
}

gint licq_tray_flash(GObject *tray)
{
  gpointer data;

  data = g_object_get_data(tray, "flash");

  if (GPOINTER_TO_INT(data) == 1) {
    licq_tray_set_content(tray, NULL, 0);
  } else {
    licq_tray_set_content(tray, NULL, 1);
  }

  return TRUE;
}

guint tray_flash_timeout_id = 0;

void
licq_tray_start_flashing()
{
	if (tray_flash_timeout_id == 0)
		tray_flash_timeout_id = gtk_timeout_add(1000,
				(GtkFunction)licq_tray_flash, 
				trayicon);
}

void
licq_tray_stop_flashing()
{
	if (tray_flash_timeout_id != 0) {
		gtk_timeout_remove(tray_flash_timeout_id);
		tray_flash_timeout_id = 0;
		// make sure the icon is now shown
    licq_tray_set_content(trayicon, NULL, 1);
	}
}
