/*
 * Licq GTK GUI Plugin
 *
 * Copyright (C) 2000, Jon Keating <jon@licq.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "config.h"
#include "licq_gtk.h"
#include "licq_log.h"

unsigned short
get_owner_status()
{
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
	unsigned short status = owner->Status();
	gUserManager.DropOwner();
	
	return status;
}

extern "C" {

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef USE_SCRNSAVER
#include <X11/extensions/scrnsaver.h>
#endif

}

#include <gtk/gtk.h>

#include <fstream>
#include <iostream>
using namespace std;

GtkWidget *contact_list_new();
GtkWidget *status_bar_new();
GtkWidget *system_status_new();
GObject *licq_init_tray();

GtkWidget *vertical_box;
GtkWidget *contact_list;
GObject *trayicon;
GtkWidget *main_window;

gint flash_icons(gpointer);

gboolean main_window_delete_event(GtkWidget *mainwindow, gpointer data)
{
	save_window_pos();
	gtk_main_quit();

	return FALSE;
}

gboolean auto_away(gpointer data);
guint auto_away_id;

GtkWidget* main_window_new(const gchar* window_title)
{
	gtk_timeout_add(1000, flash_icons, 0);

	/* Here's a good place to start the option defaults */
	char filename[MAX_FILENAME_LEN];
	snprintf(filename, MAX_FILENAME_LEN, "%s/%s", BASE_DIR, config_file());
	fstream file(filename, ios::in | ios::out);

	if(file)
	{
		file.close();
		load_options();
	}
	else
	{
		file << "[appearance]\n";
		file.close();
		load_options();
	}

	GtkWidget *scroll_bar;

	/* Create the main window */
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	/* set the position if that option is enabled */
	if (remember_window_pos)
		gtk_window_move(GTK_WINDOW(main_window), windowX, windowY);

	/* Set the title */
	gtk_window_set_title(GTK_WINDOW(main_window), window_title);
	
	/* Make the window fully resizable */
	gtk_window_set_resizable(GTK_WINDOW(main_window), TRUE);
	gtk_window_set_default_size(GTK_WINDOW(main_window), windowW, windowH);

	/* Call main_window_delete_event when the delete_event is called */
	g_signal_connect(G_OBJECT(main_window), "delete_event",
			   G_CALLBACK(main_window_delete_event), 0);
	
	// After the main window has been created, but before the contact
	// list or anything is shown, we need to make the colors and pixmaps
	do_colors();
	do_pixmaps();

	/* Add the vertical box in */
	vertical_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(main_window), vertical_box);
	gtk_widget_show(vertical_box);

	/* Add in the menu */
	menu_create();

	/* Add a scroll bar for the contact list */
	scroll_bar = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_bar),
				       //GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);

	/* Add in the contact list */
	contact_list = contact_list_new();
	
	gtk_container_add(GTK_CONTAINER(scroll_bar), contact_list);
	gtk_box_pack_start(GTK_BOX(vertical_box), scroll_bar, 
			   TRUE, TRUE, 0); 
	//contact_list_refresh();

	/* Add the system status bar and pack it into the vbox */
	gtk_box_pack_start(GTK_BOX(vertical_box), system_status_new(),
			FALSE, FALSE, 0);

	/* Now add the mode status bar in */
	gtk_box_pack_start(GTK_BOX(vertical_box), status_bar_new(), 
			FALSE, FALSE, 0);

	/* Refresh the system status */
	//	system_status_refresh();

	/* Show the widgets */
	gtk_widget_show(scroll_bar);
	gtk_widget_show(contact_list);

	// Auto logon here
	if (auto_logon != ICQ_STATUS_OFFLINE)
		icq_daemon->icqLogon(auto_logon);

	trayicon = licq_init_tray();
	
	auto_away_id = gtk_timeout_add(10000, auto_away, 0);
	
	return main_window;
}

void main_window_show()
{
	gtk_widget_show(main_window);
	contact_list_refresh();
	system_status_refresh();
	status_bar_refresh();
}

void changeStatus(int id)
{
  unsigned long newStatus = ICQ_STATUS_OFFLINE;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  if (id == ICQ_STATUS_OFFLINE)
  {
    gUserManager.DropOwner();
    icq_daemon->icqLogoff();
    return;
  }
  else if (id == (int)ICQ_STATUS_FxPRIVATE) // toggle invisible status
  {
    /*
		mnuStatus->setItemChecked(ICQ_STATUS_FxPRIVATE,
                              !mnuStatus->isItemChecked(ICQ_STATUS_FxPRIVATE));
    */
		if (o->StatusOffline())
    {
      gUserManager.DropOwner();
      return;
    }
    /*
		if (mnuStatus->isItemChecked(ICQ_STATUS_FxPRIVATE))
       newStatus = o->StatusFull() | ICQ_STATUS_FxPRIVATE;
    else
       newStatus = o->StatusFull() & (~ICQ_STATUS_FxPRIVATE);
		*/
  }
  else
  {
    newStatus = id;
  }

  // we may have been offline and gone online with invisible toggled
  /*
	if (mnuStatus->isItemChecked(ICQ_STATUS_FxPRIVATE))
     newStatus |= ICQ_STATUS_FxPRIVATE;
	*/
	
  // disable combo box, flip pixmap...
  //lblStatus->setEnabled(false);

  // call the right function
  bool b = o->StatusOffline();
  gUserManager.DropOwner();
  if (b)
    icq_daemon->icqLogon(newStatus);
  else
    icq_daemon->icqSetStatus(newStatus);
}

// this bit is copied straight out of qt-gui
gboolean auto_away(gpointer)
{
#ifdef USE_SCRNSAVER
  static XScreenSaverInfo *mit_info = NULL;
  static bool bAutoAway = false;
  static bool bAutoNA = false;
  static bool bAutoOffline = false;
	static Display *display = 0;

	unsigned short status = get_owner_status();
	
  if (mit_info == NULL) {
		if (display == NULL)
			display = XOpenDisplay(gdk_get_display());
    int event_base, error_base;
    if(XScreenSaverQueryExtension(display, &event_base, &error_base)) {
      mit_info = XScreenSaverAllocInfo ();
    }
    else {
			gLog.Warn("%sNo XScreenSaver extension found on current XServer, disabling auto-away.\n",
                L_WARNxSTR);
      return FALSE;
    }
  }

  if (!XScreenSaverQueryInfo(display, DefaultRootWindow(display), mit_info)) {
    gLog.Warn("%sXScreenSaverQueryInfo failed, disabling auto-away.\n",
              L_WARNxSTR);
    return FALSE;
  }
  unsigned long idle_time = mit_info->idle;

  // Check no one changed the status behind our back
	if ( (bAutoOffline && status != ICQ_STATUS_OFFLINE) ||
       (bAutoNA && status != ICQ_STATUS_NA && !bAutoOffline) ||
       (bAutoAway && status != ICQ_STATUS_AWAY && !bAutoNA && !bAutoOffline) )
  {
    bAutoOffline = false;
    bAutoNA = false;
    bAutoAway = false;
    return TRUE;
  }
	
//  gLog.Info("offl %d, n/a %d, away %d idlt %d\n",
//            bAutoOffline, bAutoNA, bAutoAway, idleTime);

	if (auto_offline_time > 0 && idle_time > auto_offline_time * 60000)
  {
    if (status == ICQ_STATUS_ONLINE || status == ICQ_STATUS_AWAY || 
				status == ICQ_STATUS_NA)
    {
      changeStatus(ICQ_STATUS_OFFLINE);
      bAutoOffline = true;
      bAutoAway = (status == ICQ_STATUS_ONLINE || bAutoAway);
      bAutoNA = ((status == ICQ_STATUS_AWAY && bAutoAway) || bAutoNA);
    }
  }
  else if ((auto_na_time > 0) && idle_time > auto_na_time * 60000)
  {
    if (status == ICQ_STATUS_ONLINE || status == ICQ_STATUS_AWAY)
    {
      /*
			if (autoNAMess) {
       SARList &sar = gSARManager.Fetch(SAR_NA);
       ICQUser *u = gUserManager.FetchOwner(LOCK_W);
       u->SetAutoResponse(QString(sar[autoAwayMess-1]->AutoResponse()).local8Bit());
       gUserManager.DropOwner();
       gSARManager.Drop();
      }
			*/
			
      changeStatus(ICQ_STATUS_NA);
      bAutoNA = true;
      bAutoAway = (status == ICQ_STATUS_ONLINE || bAutoAway);
    }
  }
  else if (auto_away_time > 0 && idle_time > auto_away_time * 60000)
  {
    if (status == ICQ_STATUS_ONLINE)
    {
			/*
			if (autoAwayMess) {
       SARList &sar = gSARManager.Fetch(SAR_AWAY);
       ICQUser *u = gUserManager.FetchOwner(LOCK_W);
       u->SetAutoResponse("Gone fishin'!");
       gUserManager.DropOwner();
       gSARManager.Drop();
      }
			*/
      changeStatus(ICQ_STATUS_AWAY);
      bAutoAway = true;
    }
  }
  else
  {
    if (bAutoOffline)
    {
      if (bAutoNA && bAutoAway)
      {
        changeStatus(ICQ_STATUS_ONLINE);
        bAutoOffline = bAutoNA = bAutoAway = false;
      }
      else if (bAutoNA)
      {
        changeStatus(ICQ_STATUS_AWAY);
        bAutoNA = bAutoOffline = false;
      }
      else
      {
        changeStatus(ICQ_STATUS_NA);
        bAutoOffline = false;
      }
    }
    else if (bAutoNA)
    {
      if (bAutoAway)
      {
        changeStatus(ICQ_STATUS_ONLINE);
        bAutoNA = bAutoAway = false;
      }
      else
      {
        changeStatus(ICQ_STATUS_AWAY);
        bAutoNA = false;
      }
    }
    else if (bAutoAway)
    {
      changeStatus(ICQ_STATUS_ONLINE);
      bAutoAway = false;
    }
  }

	return TRUE;
#endif // USE_SCRNSAVER
}
