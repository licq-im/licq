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

#include "licq_gtk.h"
#include "licq_file.h"

#include <gtk/gtk.h>
#include <fstream.h>

// Global variables for use in other files
bool show_offline_users;
bool show_ignored_users;
bool show_convo_timestamp;
bool recv_colors;
bool enter_sends;
bool flash_events;
char timestamp_format[50];
unsigned long auto_logon;

// The "Options" selection under the menu in the main window
void menu_options_create()
{
	GtkWidget *v_box;
	GtkWidget *notebook;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *close;
	GtkWidget *hbox;
	struct options_window *ow = g_new0(struct options_window, 1);

	// Make the window
	ow->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_show(ow->window);
	gtk_window_set_title(GTK_WINDOW(ow->window), "Licq - Options");

	// The vertical box, the main window will contain this
	v_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(ow->window), v_box);

	// The notebook that will contain all the options
	notebook = gtk_notebook_new();

	// The table that will be used in all the notebook pages
	table = gtk_table_new(5, 2, FALSE);

/*********************** FIRST TAB *********************/ 
	
	// Show ignored users in the contact list?
	ow->show_ignored = gtk_check_button_new_with_label("Show ignored users");
	gtk_table_attach(GTK_TABLE(table), ow->show_ignored, 0, 1, 0, 1,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	// Show offline users in the contact list?
	ow->show_offline = gtk_check_button_new_with_label("Show offline users");
	gtk_table_attach(GTK_TABLE(table), ow->show_offline, 1, 2, 0, 1,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);
 
	// Enter key pressed in the convo window send it?
	ow->enter_sends = gtk_check_button_new_with_label("Enter sends messages");
	gtk_table_attach(GTK_TABLE(table), ow->enter_sends, 0, 1, 1, 2,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	// Flash events
	ow->flash_events = gtk_check_button_new_with_label("Flash events");
	gtk_table_attach(GTK_TABLE(table), ow->flash_events, 1, 2, 1, 2,
		GtkAttachOptions(GTK_FILL | GTK_EXPAND), GTK_FILL, 3, 3);

	// Timestamp
	ow->show_timestamp = gtk_check_button_new_with_label("Show timestamp in messages");
	gtk_table_attach(GTK_TABLE(table), ow->show_timestamp, 0, 1, 2, 3,
		GtkAttachOptions(GTK_FILL | GTK_EXPAND),
		GTK_FILL, 3, 3);

	// Timestamp format
	hbox = gtk_hbox_new(false, 5);
	label = gtk_label_new("Timestamp Format:");
	ow->txtTimestampFormat = gtk_entry_new_with_max_length(50);
	gtk_widget_set_usize(ow->txtTimestampFormat, 80, 20);
	gtk_box_pack_start(GTK_BOX(hbox), label, false, 0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), ow->txtTimestampFormat, false, 0, 0);
	gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 2, 3,
		GtkAttachOptions(GTK_FILL | GTK_EXPAND),
		GTK_FILL, 3,3 );

	// Receive colors
	ow->chkRecvColors = gtk_check_button_new_with_label("Receive Colors");
	gtk_table_attach(GTK_TABLE(table), ow->chkRecvColors, 0, 1, 3, 4,
		GtkAttachOptions(GTK_FILL | GTK_EXPAND),
		GTK_FILL, 3, 3);

	// Put the table in the notebook
	label = gtk_label_new("General");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);

/**************** Second tab: Contact List Colors ****************/
	
	// Recreate the table
	table = gtk_table_new(5, 2, FALSE);

	// The table for the color selection in the "Contact List Colors" frame
	GtkWidget *clr_table = gtk_table_new(3, 3, FALSE);
	
	// Colors frame and attach it to the table and also a table for inside
	// the frame
	GtkWidget *color_frame = gtk_frame_new("Contact List Colors");
	gtk_table_attach(GTK_TABLE(table), color_frame, 0, 1, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 3);
	gtk_container_add(GTK_CONTAINER(color_frame), clr_table);

	// Online color label
	label = gtk_label_new("Online Color");
	gtk_table_attach(GTK_TABLE(clr_table), label, 0, 1, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 3);
	
	// Online color browse button
	int *chg_on_color = new int;
	*chg_on_color = 1;
	
	GtkWidget *online_browse = gtk_button_new_with_label("Browse");
	gtk_signal_connect(GTK_OBJECT(online_browse), "clicked",
				  GTK_SIGNAL_FUNC(show_on_color_dlg),
				  (gpointer)chg_on_color);
	gtk_table_attach(GTK_TABLE(clr_table), online_browse, 2, 3, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 3);

	// Offline color labe
	label = gtk_label_new("Offline Color");
	gtk_table_attach(GTK_TABLE(clr_table), label, 0, 1, 1, 2,
			 GTK_FILL, GTK_FILL, 3, 3);

	// Offline color browse button
	int *chg_off_color = new int;
	*chg_off_color = 2;
	
	GtkWidget *offline_browse = gtk_button_new_with_label("Browse");
	gtk_signal_connect(GTK_OBJECT(offline_browse), "clicked",
			   GTK_SIGNAL_FUNC(show_on_color_dlg),
			   (gpointer)chg_off_color);
	gtk_table_attach(GTK_TABLE(clr_table), offline_browse, 2, 3, 1, 2,
			 GTK_FILL, GTK_FILL, 3, 3);

	// Away color label
	label = gtk_label_new("Away Color");
	gtk_table_attach(GTK_TABLE(clr_table), label, 0, 1, 2, 3,
			 GTK_FILL, GTK_FILL, 3, 3);

	// Offline color browse button
	int *chg_away_color = new int;
	*chg_away_color = 3;

	GtkWidget *away_browse = gtk_button_new_with_label("Browse");
	gtk_signal_connect(GTK_OBJECT(away_browse), "clicked",
		GTK_SIGNAL_FUNC(show_on_color_dlg), (gpointer)chg_away_color);
	gtk_table_attach(GTK_TABLE(clr_table), away_browse, 2, 3, 2, 3,
		GTK_FILL, GTK_FILL, 3, 3);

	label = gtk_label_new("Contact List");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);

/********************* Third tab: Network *************************/

	table = gtk_table_new(5, 2, false);

	// ICQ Servers frame
	GtkWidget *frmServers = gtk_frame_new("ICQ Servers");
	gtk_table_attach(GTK_TABLE(table), frmServers, 0, 1, 0, 1,
		GTK_FILL, GTK_FILL, 3, 3);

	// Table inside the servers frame
	GtkWidget *tblServers = gtk_table_new(3, 2, false);
	gtk_container_add(GTK_CONTAINER(frmServers), tblServers);

	// CList of servers
	gchar *szTitles[] = { "Server", "Port" };
	ow->lstServers = gtk_clist_new_with_titles(2, szTitles);
	gtk_table_attach(GTK_TABLE(tblServers), ow->lstServers, 0, 2, 0, 1,
		GTK_FILL, GTK_FILL, 3, 3);

	// Default server port
	label = gtk_label_new("Default server port:");
	gtk_table_attach(GTK_TABLE(tblServers), label, 0, 1, 1, 2,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL), 3, 3);

	// Spin button and it's adjustment
	GtkAdjustment *adjPort =
		(GtkAdjustment *)gtk_adjustment_new(
			0, 1.0, 65535.0, 1.0, 15.0, 15.0);
	ow->spnDefPort = gtk_spin_button_new(adjPort, 25.0, 0);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(ow->spnDefPort),
		GTK_UPDATE_IF_VALID);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(ow->spnDefPort), true);
	gtk_table_attach(GTK_TABLE(tblServers), ow->spnDefPort, 1, 2, 1, 2,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL), 3, 3);

	// Firewall frame
	GtkWidget *frmFirewall = gtk_frame_new("Firewall Settings");
	gtk_table_attach(GTK_TABLE(table), frmFirewall, 1, 2, 0, 1,
		GTK_FILL, GTK_FILL, 3, 3);

	// Table inside the firewall frame
	GtkWidget *tblFirewall = gtk_table_new(3, 2, false);
	gtk_container_add(GTK_CONTAINER(frmFirewall), tblFirewall);

	ow->chkBehindFirewall = gtk_check_button_new_with_label(
		"I am behind a firewall/proxy");
	gtk_table_attach(GTK_TABLE(tblFirewall), ow->chkBehindFirewall,
		0, 1, 0, 1, GTK_FILL, GTK_FILL, 3, 3);

	ow->btnSOCKS = gtk_button_new_with_label("SOCKS5 Proxy");
	gtk_signal_connect(GTK_OBJECT(ow->btnSOCKS), "clicked",
		GTK_SIGNAL_FUNC(SOCKSClicked), 0);
	gtk_table_attach(GTK_TABLE(tblFirewall), ow->btnSOCKS, 0, 1, 1, 2,
		GtkAttachOptions(0),
		GtkAttachOptions(0), 3, 3);

	// Box for the firewall host and entry
	GtkWidget *boxHBox = gtk_hbox_new(false, 0);

	label = gtk_label_new("Firewall/Proxy Host:");
	gtk_box_pack_start(GTK_BOX(boxHBox), label, false, false, 1);

	ow->txtFirewallHost = gtk_entry_new();
	gtk_widget_set_usize(ow->txtFirewallHost, 100, 15);
	gtk_box_pack_start(GTK_BOX(boxHBox), ow->txtFirewallHost, false,
		false, 1);

	gtk_table_attach(GTK_TABLE(tblFirewall), boxHBox, 0, 2, 2, 3,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL), 1, 3);
	
	ow->chkTCPEnabled = gtk_check_button_new_with_label(
		"I can receive direct connections");
	gtk_table_attach(GTK_TABLE(tblFirewall), ow->chkTCPEnabled, 0, 1, 3, 4,
		GTK_FILL, GTK_FILL, 3, 3);	

	boxHBox = gtk_hbox_new(false, 2);
	
	// Port range
	label = gtk_label_new("Port Range:");
	gtk_box_pack_start(GTK_BOX(boxHBox), label, false, false, 0);
	
	// Adjustments
	adjPort = (GtkAdjustment *)gtk_adjustment_new(
			0, 0.0, 65535.0, 15.0, 100.0, 100.0);
	ow->spnPortLow = gtk_spin_button_new(adjPort, 25.0, 0);

	adjPort = (GtkAdjustment *)gtk_adjustment_new(
			0, 0.0, 65535.0, 15.0, 100.0, 100.0);
	ow->spnPortHigh = gtk_spin_button_new(adjPort, 25.0, 0);
	label = gtk_label_new("to");

	// Resize the spin buttons
	gtk_widget_set_usize(ow->spnPortLow, 55, 15);
	gtk_widget_set_usize(ow->spnPortHigh, 55, 15);

	gtk_box_pack_start(GTK_BOX(boxHBox), ow->spnPortLow, false, false, 0);
	gtk_box_pack_start(GTK_BOX(boxHBox), label, false, false, 0);
	gtk_box_pack_start(GTK_BOX(boxHBox), ow->spnPortHigh, false, false, 0);

	gtk_table_attach(GTK_TABLE(tblFirewall), boxHBox, 0, 2, 4, 5,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL), 3, 3);

	label = gtk_label_new("Network");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);

/********************* Fourth tab: Status *************************/

	table = gtk_table_new(4, 2, false);

	GtkWidget *frmAutoLogon = gtk_frame_new("Auto Logon");
	
	// Box for the combo box and check button
	GtkWidget *boxALBox = gtk_vbox_new(false, 5);
	
	// Combox box with the options
	GList *lItems = 0;
	lItems = g_list_append(lItems, const_cast<char *>("(None)"));
	lItems = g_list_append(lItems, const_cast<char *>("Online"));
	lItems = g_list_append(lItems, const_cast<char *>("Free For Chat"));
	lItems = g_list_append(lItems, const_cast<char *>("Away"));
	lItems = g_list_append(lItems, const_cast<char *>("Not Available"));
	lItems = g_list_append(lItems, const_cast<char *>("Occupied"));
	lItems = g_list_append(lItems, const_cast<char *>("Do Not Disturb"));

	ow->cmbAutoLogon = gtk_combo_new();
	gtk_combo_set_popdown_strings(GTK_COMBO(ow->cmbAutoLogon), lItems);

	gtk_box_pack_start(GTK_BOX(boxALBox), ow->cmbAutoLogon, false, false, 0);

	// Check button for invisible
	ow->chkInvisible = gtk_check_button_new_with_label("Invisible");
	gtk_box_pack_start(GTK_BOX(boxALBox), ow->chkInvisible, false, false, 0);

	gtk_container_add(GTK_CONTAINER(frmAutoLogon), boxALBox);

	label = gtk_label_new("Status");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frmAutoLogon, label);

/********************* END OF ALL THE TABS ************************/

	/* Put the notebook in the window */
	gtk_box_pack_start(GTK_BOX(v_box), notebook, FALSE, FALSE, 0);

	/* The close button */
	close = gtk_button_new_with_label("Done");
	gtk_box_pack_start(GTK_BOX(v_box), close, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(close), "clicked",
			   GTK_SIGNAL_FUNC(done_options), (gpointer)ow);
	
	// Set all the options now
	set_options(ow);

	gtk_widget_show_all(ow->window);
}

void set_options(struct options_window *ow)
{
	// Check boxes for contact list apperance
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->show_ignored),
		show_ignored_users);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->show_offline),
		show_offline_users);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->enter_sends),
		enter_sends);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->flash_events),
		flash_events);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->show_timestamp),
		show_convo_timestamp);
	gtk_entry_set_text(GTK_ENTRY(ow->txtTimestampFormat),
		timestamp_format);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->chkRecvColors),
		recv_colors);

	// Clist of servers

	// Default server port
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ow->spnDefPort),
		icq_daemon->getDefaultRemotePort());

	// Firewall stuff
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->chkTCPEnabled),
		icq_daemon->TCPEnabled());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ow->spnPortLow),
		icq_daemon->TCPPortsLow());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ow->spnPortHigh),
		icq_daemon->TCPPortsHigh());

	// Auto Logon
	char szStatus[15];
	if (auto_logon & ICQ_STATUS_DND)
	{
		strcpy(szStatus, "Do Not Disturb");
	}
	else if (auto_logon & ICQ_STATUS_OCCUPIED)
	{
		strcpy(szStatus, "Occupied");
	}
	else if (auto_logon & ICQ_STATUS_NA)
	{
		strcpy(szStatus, "Not Available");
	}
	else if (auto_logon & ICQ_STATUS_AWAY)
	{
		strcpy(szStatus, "Away");
	}
	else if (auto_logon & ICQ_STATUS_FREEFORCHAT)
	{
		strcpy(szStatus, "Free For Chat");
	}
	else if (auto_logon & ICQ_STATUS_ONLINE)
	{
		strcpy(szStatus, "Online");
	}
	else
	{
		strcpy(szStatus, "(None)");
	}

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(ow->cmbAutoLogon)->entry),
		szStatus);

	if (auto_logon != ICQ_STATUS_OFFLINE && auto_logon & ICQ_STATUS_FxPRIVATE)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->chkInvisible),
			true);
	}
		
}

void done_options(GtkWidget *widget, gpointer data)
{
	struct options_window *ow = (struct options_window *)data;
	show_offline_users = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(ow->show_offline));
	show_ignored_users = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(ow->show_ignored));
	show_convo_timestamp = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(ow->show_timestamp));
	enter_sends = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ow->enter_sends));
	flash_events = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
		ow->flash_events));
	gchar *temp = gtk_editable_get_chars(GTK_EDITABLE(ow->txtTimestampFormat), 0, -1);
	strcpy(timestamp_format, temp);
	g_free(temp);
	recv_colors = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
		ow->chkRecvColors));
	
	// Save the daemon options
	icq_daemon->setDefaultRemotePort(
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ow->spnDefPort)));

	icq_daemon->SetTCPPorts(
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ow->spnPortLow)),
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ow->spnPortHigh)));
	
	icq_daemon->SetTCPEnabled(
	   !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ow->chkTCPEnabled)));

	// Auto logon
	const char *szAutoLogon =
	  gtk_editable_get_chars(GTK_EDITABLE(GTK_COMBO(ow->cmbAutoLogon)->entry),
	                         0, -1);

	if (strcmp("Online", szAutoLogon) == 0)
		auto_logon = ICQ_STATUS_ONLINE;
	else if (strcmp("Away", szAutoLogon) == 0)
		auto_logon = ICQ_STATUS_AWAY;
	else if (strcmp("Not Available", szAutoLogon) == 0)
		auto_logon = ICQ_STATUS_NA;
	else if (strcmp("Occupied", szAutoLogon) == 0)
		auto_logon = ICQ_STATUS_OCCUPIED;
	else if (strcmp("Do Not Disturb", szAutoLogon) == 0)
		auto_logon = ICQ_STATUS_DND;
	else if (strcmp("Free For Chat", szAutoLogon) == 0)
		auto_logon = ICQ_STATUS_FREEFORCHAT;
	else
		auto_logon = ICQ_STATUS_OFFLINE;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ow->chkInvisible)))
	{
		auto_logon |= ICQ_STATUS_FxPRIVATE;
	}

	gtk_widget_destroy(ow->window);

	icq_daemon->SaveConf();

	// Save our options
	char filename[MAX_FILENAME_LEN];
	sprintf(filename, "%s/licq_jons-gtk-gui.conf", BASE_DIR);
	CIniFile licqConf(INI_FxERROR | INI_FxALLOWxCREATE);
	if(!licqConf.LoadFile(filename))
		return;

	licqConf.SetSection("appearance");
	licqConf.WriteNum("ColorOnline_Red", online_color->red);
	licqConf.WriteNum("ColorOnline_Green", online_color->green);
	licqConf.WriteNum("ColorOnline_Blue", online_color->blue);
	licqConf.WriteNum("ColorOnline_Pixel", online_color->pixel);
	licqConf.WriteNum("ColorOffline_Red", offline_color->red);
	licqConf.WriteNum("ColorOffline_Green", offline_color->green);
	licqConf.WriteNum("ColorOffline_Blue", offline_color->blue);
	licqConf.WriteNum("ColorOffline_Pixel", offline_color->pixel);
	licqConf.WriteNum("ColorAway_Red", away_color->red);
	licqConf.WriteNum("ColorAway_Green", away_color->green);
	licqConf.WriteNum("ColorAway_Blue", away_color->blue);
	licqConf.WriteNum("ColorAway_Pixel", away_color->pixel);
	licqConf.WriteNum("AutoLogon", auto_logon);
	licqConf.WriteBool("ShowOfflineUsers", show_offline_users);
	licqConf.WriteBool("ShowIgnoredUsres", show_ignored_users);
	licqConf.WriteBool("EnterSends", enter_sends);
	licqConf.WriteBool("FlashEvents", flash_events);
	licqConf.WriteBool("RecvColors", recv_colors);
	licqConf.WriteBool("ShowTimestamp", show_convo_timestamp);
	licqConf.WriteStr("TimestampFormat", timestamp_format);

	licqConf.FlushFile();
	licqConf.CloseFile();

	// Refresh the colors
	do_colors();

	// Refresh contact list
	contact_list_refresh();
}

// load_options() is only called if the file exists
void load_options()
{
	online_color = new GdkColor;
	offline_color = new GdkColor;
	away_color = new GdkColor;

	CIniFile licqConf;
	licqConf.LoadFile(g_strdup_printf("%s/licq_jons-gtk-gui.conf",
				          BASE_DIR));
	licqConf.SetSection("appearance");

	gLog.Info("%sLoading Jon's GTK+ GUI configuration", L_INITxSTR);

	// Online color
	licqConf.ReadNum("ColorOnline_Red", online_color->red, 8979);
	licqConf.ReadNum("ColorOnline_Green", online_color->green, 27457);
	licqConf.ReadNum("ColorOnline_Blue", online_color->blue, 63052);
	licqConf.ReadNum("ColorOnline_Pixel", online_color->pixel, 0);
	
	// Offline color
	licqConf.ReadNum("ColorOffline_Red", offline_color->red, 59080);
	licqConf.ReadNum("ColorOffline_Green", offline_color->green, 0);
	licqConf.ReadNum("ColorOffline_Blue", offline_color->blue, 1660);
	licqConf.ReadNum("ColorOffline_Pixel", offline_color->pixel, 0);

	// Away color
	licqConf.ReadNum("ColorAway_Red", away_color->red, 0);
	licqConf.ReadNum("ColorAway_Green", away_color->green, 30000);
	licqConf.ReadNum("ColorAway_Blue", away_color->blue, 0);
	licqConf.ReadNum("ColorAway_Pixel", away_color->pixel, 0);

	// General options
	licqConf.ReadBool("ShowOfflineUsers", show_offline_users, true);
	licqConf.ReadBool("ShowIgnoredUsers", show_ignored_users, false);
	licqConf.ReadBool("EnterSends", enter_sends, true);
	licqConf.ReadBool("FlashEvents", flash_events, true);
	licqConf.ReadBool("RecvColors", recv_colors, true);
	licqConf.ReadBool("ShowTimestamp", show_convo_timestamp, true);
	licqConf.ReadStr("TimestampFormat", timestamp_format, "%H:%M:%S");
	
	// Auto logon
	licqConf.ReadNum("AutoLogon", auto_logon, ICQ_STATUS_OFFLINE);
}

void show_on_color_dlg(GtkWidget *widget, gpointer data)
{
	gdouble color[4];
	int *change = (int *)data;
	
	switch(*change)
	{
	case 1:
		color[0] = (gdouble)online_color->red / 65535;
		color[1] = (gdouble)online_color->green / 65535;
		color[2] = (gdouble)online_color->blue / 65535;
		color[3] = (gdouble)online_color->pixel / 65535;
		break;
	case 2:
		color[0] = (gdouble)offline_color->red / 65535;
		color[1] = (gdouble)offline_color->green / 65535;
		color[2] = (gdouble)offline_color->blue / 65535;
		color[3] = (gdouble)offline_color->pixel / 65535;
		break;
	case 3:
		color[0] = (gdouble)away_color->red / 65535;
		color[1] = (gdouble)away_color->green / 65535;
		color[2] = (gdouble)away_color->blue / 65535;
		color[3] = (gdouble)away_color->pixel / 65535;
		break;
	}

	/* The select color dialog */
	GtkWidget *color_dialog =
		gtk_color_selection_dialog_new("Licq - Choose Color");
	/* Set the int that represents the color to change */
	gtk_object_set_user_data(GTK_OBJECT(color_dialog), (gpointer)change);

	/* Set the default color to the current online color */
	gtk_color_selection_set_color(
		GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(color_dialog)->colorsel),
				      color);

	/* Handle the ok and cancel button on the color dialog */
	gtk_signal_connect(
		GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(color_dialog)->ok_button),
		"clicked", GTK_SIGNAL_FUNC(color_dlg_ok), (gpointer)color_dialog);
	gtk_signal_connect(
		GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(color_dialog)->cancel_button),
		"clicked", GTK_SIGNAL_FUNC(color_dlg_cancel),
		(gpointer)color_dialog);

	/* Show the color dialog */
	gtk_widget_show_all(color_dialog);
}

void color_dlg_ok(GtkWidget *widget, gpointer data)
{
	GtkWidget *color_dialog = (GtkWidget *)data;

	/* What are we changing? */
	gint *change = (gint *)gtk_object_get_user_data(GTK_OBJECT(color_dialog));

	/* To store the selected color */
	double new_color[4];

	/* Now get the color that was selected */
	gtk_color_selection_get_color(
		GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(color_dialog)->colorsel),
		new_color);

	switch(*change)
	{
	case 1:
		online_color->red = (guint16)(new_color[0] * 65535.0);
		online_color->green = (guint16)(new_color[1] * 65535.0);
		online_color->blue = (guint16)(new_color[2] * 65535.0);
		online_color->pixel = (gulong)new_color[3];
		break;
	case 2:
		offline_color->red = (guint16)(new_color[0] * 65535.0);
		offline_color->green = (guint16)(new_color[1] * 65535.0);
		offline_color->blue = (guint16)(new_color[2] * 65535.0);
		offline_color->pixel = (gulong)new_color[3];
 		break;
	case 3:
		away_color->red = (guint16)(new_color[0] * 65535.0);
		away_color->green = (guint16)(new_color[1] * 65535.0);
		away_color->blue = (guint16)(new_color[2] * 65535.0);
		away_color->pixel = (gulong)new_color[3];
		break;
	}

	gtk_widget_destroy(color_dialog);
	delete change;
}

void color_dlg_cancel(GtkWidget *widget, gpointer data)
{
	GtkWidget *color_dialog = (GtkWidget *)data;
	gint *change = (gint *)gtk_object_get_user_data(GTK_OBJECT(color_dialog));
	gtk_widget_destroy(color_dialog);
	delete change;
}

void SOCKSClicked(GtkWidget *widget, gpointer data)
{
	if (icq_daemon->SocksEnabled())
	{
		const char *env = icq_daemon->SocksServer();
		if (env)
		{
			message_box("SOCKS5 support is built in but disabled.\n"
				    "To enable it, set the SOCKS5_SERVER\n"
				    "environment variable to <server>:<port>.\n");
		}
		else
		{
			gchar *message = g_strdup_printf(
				"SOCKS5 support is built in and enabled at\n"
				"\"%s\".\n", env);
			message_box(message);
			g_free(message);
		}
	}
	else
	{
		message_box("To enable SOCKS5 support, install NEC Socks or Dante\n"
			    "then configure the Licq daemon with \"--enable-socks5\".\n");
	}
}
