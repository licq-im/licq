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
#include "licq_log.h"
#include "utilities.h"

#include <gtk/gtk.h>
#include <fstream>

struct options_window
{
	GtkWidget *window;
	GtkWidget *show_ignored;
	GtkWidget *show_offline;
	GtkWidget *show_timestamp;
	GtkWidget *txtTimestampFormat;
	GtkWidget *enter_sends;
	GtkWidget *flash_events;
	GtkWidget *chkRecvColors;
	GtkWidget *chkRememberWindowPos;

	// Network section
	GtkWidget *icqServer;
	GtkWidget *icqServerPort;
	GtkWidget *chkBehindFirewall;
	GtkWidget *btnSOCKS;
	GtkWidget *txtFirewallHost;
	GtkWidget *chkTCPEnabled;
	GtkWidget *spnPortLow;
	GtkWidget *spnPortHigh;

	// Status section
	GtkWidget *cmbAutoLogon;
	GtkWidget *chkInvisible;
  
	// Timeouts
  GtkWidget *auto_away_time;
  GtkWidget *auto_na_time;
  GtkWidget *auto_offline_time;
};

void done_options(GtkWidget *, struct options_window *);
//void SOCKSClicked(GtkWidget *, gpointer);

// Global variables for use in other files
bool show_offline_users;
bool show_ignored_users;
bool show_convo_timestamp;
bool recv_colors;
bool enter_sends;
bool flash_events;
char timestamp_format[50];
unsigned long auto_logon;
bool remember_window_pos;
unsigned long auto_away_time;
unsigned long auto_na_time;
unsigned long auto_offline_time;

// Global variables for window position and size
short int windowX;
short int windowY;
short int windowH;
short int windowW;

void set_options(struct options_window *ow);

void
run_color_dialog(GtkWidget *color_button, GdkColor *color)
{
  GtkWidget *dialog = gtk_color_selection_dialog_new("Choose color");
  GtkColorSelection *csel = 
      GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)->colorsel);
  gtk_color_selection_set_current_color(csel, color);
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_color_selection_get_current_color(csel, color);
    gtk_widget_modify_bg(color_button, GTK_STATE_NORMAL, color);
    gtk_widget_modify_bg(color_button, GTK_STATE_ACTIVE, color);
    gtk_widget_modify_bg(color_button, GTK_STATE_PRELIGHT, color);
    gtk_widget_modify_bg(color_button, GTK_STATE_SELECTED, color);
    gtk_widget_modify_bg(color_button, GTK_STATE_INSENSITIVE, color);
  }
  gtk_widget_destroy(dialog);
}

GtkWidget *
color_button_new(const char *label, GdkColor *color)
{
	GtkWidget *color_button = gtk_button_new_with_label(label);
  gtk_widget_modify_bg(color_button, GTK_STATE_NORMAL, color);
  gtk_widget_modify_bg(color_button, GTK_STATE_ACTIVE, color);
  gtk_widget_modify_bg(color_button, GTK_STATE_PRELIGHT, color);
  gtk_widget_modify_bg(color_button, GTK_STATE_SELECTED, color);
  gtk_widget_modify_bg(color_button, GTK_STATE_INSENSITIVE, color);
  g_signal_connect(G_OBJECT(color_button), "clicked",
      G_CALLBACK(run_color_dialog), color);
  return color_button;
}

// The "Options" selection under the menu in the main window
void
menu_options_create()
{
	static struct options_window *ow = 0;
  
  if (ow != NULL) {
    gtk_window_present(GTK_WINDOW(ow->window));
    return;
  }
  
	GtkWidget *hbox;
	ow = g_new0(struct options_window, 1);

	// Make the window
	ow->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_show(ow->window);
	gtk_window_set_title(GTK_WINDOW(ow->window), "Licq - Options");
	gtk_container_set_border_width(GTK_CONTAINER(ow->window), 10);

	// The vertical box, the main window will contain this
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(ow->window), v_box);

	// The notebook that will contain all the options
	GtkWidget *notebook = gtk_notebook_new();

	// The table that will be used in all the notebook pages
  GtkWidget *general_vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(general_vbox), 5);
	GtkWidget *table = gtk_table_new(5, 2, FALSE);
  gtk_box_pack_start(GTK_BOX(general_vbox), table, FALSE, FALSE, 5);

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
	ow->show_timestamp = 
      gtk_check_button_new_with_label("Show timestamp in messages");
	gtk_table_attach(GTK_TABLE(table), ow->show_timestamp, 0, 1, 2, 3,
		  GtkAttachOptions(GTK_FILL | GTK_EXPAND),
		  GTK_FILL, 3, 3);

	// Timestamp format
	hbox = gtk_hbox_new(false, 5);
	GtkWidget *label = gtk_label_new("Timestamp format:");
	ow->txtTimestampFormat = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(ow->txtTimestampFormat), 50);
  gtk_entry_set_width_chars(GTK_ENTRY(ow->txtTimestampFormat), 15);
	gtk_box_pack_start(GTK_BOX(hbox), label, false, 0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), ow->txtTimestampFormat, false, 0, 0);
	gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 2, 3,
		  GtkAttachOptions(GTK_FILL | GTK_EXPAND),
		  GTK_FILL, 3, 3);

	// Receive colors
	ow->chkRecvColors = gtk_check_button_new_with_label("Receive Colors");
	gtk_table_attach(GTK_TABLE(table), ow->chkRecvColors, 0, 1, 3, 4,
		  GtkAttachOptions(GTK_FILL | GTK_EXPAND),
		  GTK_FILL, 3, 3);

	// Remember window position
	ow->chkRememberWindowPos = 
      gtk_check_button_new_with_label("Save Window Position");
	gtk_table_attach(GTK_TABLE(table), ow->chkRememberWindowPos, 1, 2, 3, 4,
		  GtkAttachOptions(GTK_FILL | GTK_EXPAND),
		  GTK_FILL, 3, 3);

	// The table for the color selection in the "Contact List Colors" frame
	GtkWidget *color_frame = gtk_frame_new("Contact list colors");
	gtk_container_set_border_width(GTK_CONTAINER(color_frame), 10);
  gtk_box_pack_start(GTK_BOX(general_vbox), color_frame, FALSE, FALSE, 5);
	GtkWidget *color_box = hbutton_box_new(15, GTK_BUTTONBOX_SPREAD);
	gtk_container_set_border_width(GTK_CONTAINER(color_box), 10);
  gtk_container_add(GTK_CONTAINER(color_frame), color_box);
	
	gtk_container_add(GTK_CONTAINER(color_box), 
      color_button_new("Online", online_color));
	gtk_container_add(GTK_CONTAINER(color_box), 
      color_button_new("Offline", offline_color));
	gtk_container_add(GTK_CONTAINER(color_box),
      color_button_new("Away", away_color));

	// Put the table in the notebook
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), general_vbox,
      gtk_label_new("General"));

/********************* Third tab: Network *************************/

	GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	// ICQ Server frame
	GtkWidget *frmServers = gtk_frame_new("Server settings");
	gtk_box_pack_start(GTK_BOX(vbox), frmServers, FALSE, FALSE, 0);

	// Table inside the servers frame
	GtkWidget *tblServers = gtk_table_new(2, 2, FALSE);
	gtk_container_add(GTK_CONTAINER(frmServers), tblServers);

  label = gtk_label_new("ICQ server:");
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tblServers), label, 0, 1, 0, 1,
		  GTK_FILL, GTK_FILL, 3, 3);
  ow->icqServer = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(ow->icqServer), 20);
  gtk_table_attach(GTK_TABLE(tblServers), ow->icqServer, 1, 2, 0, 1,
  		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
	  	GtkAttachOptions(GTK_EXPAND | GTK_FILL), 3, 3);
  
	// Default server port
	label = gtk_label_new("ICQ server port:");
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tblServers), label, 0, 1, 1, 2,
		  GTK_FILL, GTK_FILL, 3, 3);
  
	// Spin button and it's adjustment
	ow->icqServerPort = gtk_spin_button_new_with_range(1, 65535, 1);

	gtk_table_attach(GTK_TABLE(tblServers), ow->icqServerPort, 1, 2, 1, 2,
		  GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		  GtkAttachOptions(GTK_EXPAND | GTK_FILL), 3, 3);

	// Firewall frame
	GtkWidget *frmFirewall = gtk_frame_new("Firewall settings");
	gtk_box_pack_start(GTK_BOX(vbox), frmFirewall, FALSE, FALSE, 0);

	// Table inside the firewall frame
	GtkWidget *tblFirewall = gtk_table_new(4, 2, false);
	gtk_container_add(GTK_CONTAINER(frmFirewall), tblFirewall);

	ow->chkBehindFirewall = gtk_check_button_new_with_label(
		  "I am behind a firewall/proxy");
	gtk_table_attach(GTK_TABLE(tblFirewall), ow->chkBehindFirewall,
		  0, 2, 0, 1, GTK_FILL, GTK_FILL, 3, 3);

	/*
  ow->btnSOCKS = gtk_button_new_with_mnemonic("SOCKS5 _Proxy");
	//gtk_signal_connect(GTK_OBJECT(ow->btnSOCKS), "clicked",
	//	GTK_SIGNAL_FUNC(SOCKSClicked), 0);
	gtk_table_attach(GTK_TABLE(tblFirewall), ow->btnSOCKS, 0, 1, 1, 2,
		  GtkAttachOptions(0),
		  GtkAttachOptions(0), 3, 3);
  */
  
	label = gtk_label_new("Firewall/proxy host:");
	gtk_table_attach(GTK_TABLE(tblFirewall), label,
		  0, 1, 1, 2, GTK_FILL, GTK_FILL, 3, 3);
	//gtk_box_pack_start(GTK_BOX(boxHBox), label, false, false, 1);

	ow->txtFirewallHost = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(ow->txtFirewallHost), 20);
	gtk_table_attach(GTK_TABLE(tblFirewall), ow->txtFirewallHost,
		  1, 2, 1, 2, 
		  GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		  GtkAttachOptions(GTK_EXPAND | GTK_FILL),
      3, 3);
	
	ow->chkTCPEnabled = gtk_check_button_new_with_label(
		  "I can receive direct connections");
	gtk_table_attach(GTK_TABLE(tblFirewall), ow->chkTCPEnabled, 0, 2, 2, 3,
		  GTK_FILL, GTK_FILL, 3, 3);	

	// Port range
	label = gtk_label_new("Port range:");
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tblFirewall), label,
		  0, 1, 3, 4, GTK_FILL, GTK_FILL, 3, 3);
	
	// Spinbuttons
	GtkWidget *boxHBox = gtk_hbox_new(false, 2);
	ow->spnPortLow = gtk_spin_button_new_with_range(0, 65535, 1);
	label = gtk_label_new("to");
	ow->spnPortHigh = gtk_spin_button_new_with_range(0, 65535, 1);

	// Add spin buttons to the box
	gtk_box_pack_start(GTK_BOX(boxHBox), ow->spnPortLow, false, false, 0);
	gtk_box_pack_start(GTK_BOX(boxHBox), label, false, false, 0);
	gtk_box_pack_start(GTK_BOX(boxHBox), ow->spnPortHigh, false, false, 0);

	gtk_table_attach(GTK_TABLE(tblFirewall), boxHBox, 1, 2, 3, 4,
		  GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		  GtkAttachOptions(GTK_EXPAND | GTK_FILL), 3, 3);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox,
      gtk_label_new("Network"));

/********************* Fourth tab: Status *************************/

  GtkWidget *status_vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_set_border_width(GTK_CONTAINER(status_vbox), 5);
	
	GtkWidget *frmAutoLogon = gtk_frame_new("Auto Logon");
  gtk_box_pack_start(GTK_BOX(status_vbox), frmAutoLogon, FALSE, FALSE, 0);

	// Box for the combo box and check button
	GtkWidget *boxALBox = gtk_vbox_new(false, 5);
	gtk_container_add(GTK_CONTAINER(frmAutoLogon), boxALBox);

  ow->cmbAutoLogon = gtk_option_menu_new();
	gtk_box_pack_start(GTK_BOX(boxALBox), ow->cmbAutoLogon, false, false, 0);

  GtkWidget *menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(ow->cmbAutoLogon), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
      gtk_menu_item_new_with_label("Offline"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
      gtk_menu_item_new_with_label("Online"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
      gtk_menu_item_new_with_label("Free For Chat"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
      gtk_menu_item_new_with_label("Away"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
      gtk_menu_item_new_with_label("Not Available"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
      gtk_menu_item_new_with_label("Occupied"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
      gtk_menu_item_new_with_label("Do Not Disturb"));

	// Check button for invisible
	ow->chkInvisible = gtk_check_button_new_with_label("Invisible");
	gtk_box_pack_start(GTK_BOX(boxALBox), ow->chkInvisible, false, false, 5);

	GtkWidget *frame = gtk_frame_new("Auto timeouts (seconds)");
  gtk_box_pack_start(GTK_BOX(status_vbox), frame, FALSE, FALSE, 0);
  GtkWidget *tbl = gtk_table_new(3, 2, FALSE);
  gtk_container_add(GTK_CONTAINER(frame), tbl);
  
  gtk_table_attach(GTK_TABLE(tbl), gtk_label_new("Away timeout:"),
      0, 1, 0, 1, GTK_FILL, GTK_FILL, 3, 3);
	ow->auto_away_time = gtk_spin_button_new_with_range(0, 24 * 60 * 60, 1);
  gtk_table_attach(GTK_TABLE(tbl), ow->auto_away_time,
      1, 2, 0, 1, GTK_FILL, GTK_FILL, 3, 3);

  gtk_table_attach(GTK_TABLE(tbl), gtk_label_new("NA timeout:"),
      0, 1, 1, 2, GTK_FILL, GTK_FILL, 3, 3);
	ow->auto_na_time = gtk_spin_button_new_with_range(0, 24 * 60 * 60, 1);
  gtk_table_attach(GTK_TABLE(tbl), ow->auto_na_time,
      1, 2, 1, 2, GTK_FILL, GTK_FILL, 3, 3);

  gtk_table_attach(GTK_TABLE(tbl), gtk_label_new("Offline timeout:"),
      0, 1, 2, 3, GTK_FILL, GTK_FILL, 3, 3);
	ow->auto_offline_time = gtk_spin_button_new_with_range(0, 24 * 60 * 60, 1);
  gtk_table_attach(GTK_TABLE(tbl), ow->auto_offline_time,
      1, 2, 2, 3, GTK_FILL, GTK_FILL, 3, 3);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), status_vbox,
      gtk_label_new("Status"));

/********************* END OF ALL THE TABS ************************/

	/* Put the notebook in the window */
	gtk_box_pack_start(GTK_BOX(v_box), notebook, FALSE, FALSE, 0);

	/* The close button */
	GtkWidget *close = gtk_button_new_with_mnemonic("_Apply");
	GtkWidget *cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
  GtkWidget *bbox = hbutton_box_new();
  gtk_container_add(GTK_CONTAINER(bbox), close);
  gtk_container_add(GTK_CONTAINER(bbox), cancel);
	gtk_box_pack_start(GTK_BOX(v_box), bbox, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(close), "clicked",
			   G_CALLBACK(done_options), ow);
	g_signal_connect(G_OBJECT(cancel), "clicked",
			   G_CALLBACK(window_close), ow->window);
	g_signal_connect(G_OBJECT(ow->window), "destroy",
			   G_CALLBACK(destroy_cb), &ow);
	
	// Set all the options now
	set_options(ow);

	gtk_widget_show_all(ow->window);
}

void
set_options(struct options_window *ow)
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
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->chkRememberWindowPos),
		  remember_window_pos);

  // ICQ Server and port
  gtk_entry_set_text(GTK_ENTRY(ow->icqServer),
      icq_daemon->ICQServer());
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(ow->icqServerPort),
      icq_daemon->ICQServerPort());
  
	// Firewall stuff
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->chkTCPEnabled),
		  icq_daemon->TCPEnabled());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ow->spnPortLow),
		  icq_daemon->TCPPortsLow());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ow->spnPortHigh),
		  icq_daemon->TCPPortsHigh());

	// Auto Logon
	if (auto_logon & ICQ_STATUS_OFFLINE)
  	gtk_option_menu_set_history(GTK_OPTION_MENU(ow->cmbAutoLogon), 0);
	else if (auto_logon & ICQ_STATUS_ONLINE)
  	gtk_option_menu_set_history(GTK_OPTION_MENU(ow->cmbAutoLogon), 1);
	else if (auto_logon & ICQ_STATUS_FREEFORCHAT)
  	gtk_option_menu_set_history(GTK_OPTION_MENU(ow->cmbAutoLogon), 2);
	else if (auto_logon & ICQ_STATUS_AWAY)
  	gtk_option_menu_set_history(GTK_OPTION_MENU(ow->cmbAutoLogon), 3);
	else if (auto_logon & ICQ_STATUS_NA)
  	gtk_option_menu_set_history(GTK_OPTION_MENU(ow->cmbAutoLogon), 4);
	else if (auto_logon & ICQ_STATUS_OCCUPIED)
  	gtk_option_menu_set_history(GTK_OPTION_MENU(ow->cmbAutoLogon), 5);
	else if (auto_logon & ICQ_STATUS_DND)
  	gtk_option_menu_set_history(GTK_OPTION_MENU(ow->cmbAutoLogon), 6);

	if (auto_logon != ICQ_STATUS_OFFLINE && auto_logon & ICQ_STATUS_FxPRIVATE)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->chkInvisible), TRUE);

  gtk_spin_button_set_value(GTK_SPIN_BUTTON(ow->auto_away_time),
      auto_away_time);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(ow->auto_na_time),
      auto_na_time);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(ow->auto_offline_time), 
      auto_offline_time);
}

void
done_options(GtkWidget *widget, struct options_window *ow)
{
	show_offline_users = gtk_toggle_button_get_active(
		  GTK_TOGGLE_BUTTON(ow->show_offline));
	show_ignored_users = gtk_toggle_button_get_active(
		  GTK_TOGGLE_BUTTON(ow->show_ignored));
	show_convo_timestamp = gtk_toggle_button_get_active(
		  GTK_TOGGLE_BUTTON(ow->show_timestamp));
	enter_sends = gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(ow->enter_sends));
	flash_events = gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(ow->flash_events));
	strcpy(timestamp_format, entry_get_chars(ow->txtTimestampFormat).c_str());
	recv_colors = gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(ow->chkRecvColors));
	remember_window_pos = gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(ow->chkRememberWindowPos));
	
	// Save the daemon options
  if (!entry_get_chars(ow->icqServer).empty())
     icq_daemon->SetICQServer(entry_get_chars(ow->icqServer).c_str());
	icq_daemon->SetICQServerPort(
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ow->icqServerPort)));

	icq_daemon->SetTCPPorts(
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ow->spnPortLow)),
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ow->spnPortHigh)));
	
	icq_daemon->SetTCPEnabled(
  	  !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ow->chkTCPEnabled)));

	// Auto logon
	switch (gtk_option_menu_get_history(GTK_OPTION_MENU(ow->cmbAutoLogon))) {
    case 1:	auto_logon = ICQ_STATUS_ONLINE; break;
		case 2: auto_logon = ICQ_STATUS_FREEFORCHAT; break;
		case 3: auto_logon = ICQ_STATUS_AWAY; break;
		case 4: auto_logon = ICQ_STATUS_NA; break;
		case 5: auto_logon = ICQ_STATUS_OCCUPIED; break;
		case 6: auto_logon = ICQ_STATUS_DND; break;
		default: auto_logon = ICQ_STATUS_OFFLINE; break;
  }

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ow->chkInvisible)))
		auto_logon |= ICQ_STATUS_FxPRIVATE;

  auto_away_time = 
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ow->auto_away_time));
  auto_na_time = 
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ow->auto_na_time));
  auto_offline_time = 
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ow->auto_offline_time));

	window_close(NULL, ow->window);

	icq_daemon->SaveConf();

	// Save our options
	char filename[MAX_FILENAME_LEN];
	snprintf(filename, MAX_FILENAME_LEN, "%s/%s", BASE_DIR, config_file());
	CIniFile licqConf(INI_FxERROR | INI_FxALLOWxCREATE);
	if (!licqConf.LoadFile(filename))
		return;

	licqConf.SetSection("appearance");
	licqConf.WriteNum("ColorOnline_Red", online_color->red);
	licqConf.WriteNum("ColorOnline_Green", online_color->green);
	licqConf.WriteNum("ColorOnline_Blue", online_color->blue);
	licqConf.WriteNum("ColorOnline_Pixel", 
			static_cast<unsigned long>(online_color->pixel));
	licqConf.WriteNum("ColorOffline_Red", offline_color->red);
	licqConf.WriteNum("ColorOffline_Green", offline_color->green);
	licqConf.WriteNum("ColorOffline_Blue", offline_color->blue);
	licqConf.WriteNum("ColorOffline_Pixel", 
			static_cast<unsigned long>(offline_color->pixel));
	licqConf.WriteNum("ColorAway_Red", away_color->red);
	licqConf.WriteNum("ColorAway_Green", away_color->green);
	licqConf.WriteNum("ColorAway_Blue", away_color->blue);
	licqConf.WriteNum("ColorAway_Pixel", 
			static_cast<unsigned long>(away_color->pixel));
	licqConf.WriteNum("AutoLogon", auto_logon);
	licqConf.WriteBool("ShowOfflineUsers", show_offline_users);
	licqConf.WriteBool("ShowIgnoredUsres", show_ignored_users);
	licqConf.WriteBool("EnterSends", enter_sends);
	licqConf.WriteBool("FlashEvents", flash_events);
	licqConf.WriteBool("RecvColors", recv_colors);
	licqConf.WriteBool("ShowTimestamp", show_convo_timestamp);
	licqConf.WriteBool("RememberWindowPos", remember_window_pos);
	licqConf.WriteStr("TimestampFormat", timestamp_format);

	licqConf.WriteNum("AutoAway", auto_away_time);
	licqConf.WriteNum("AutoNA", auto_na_time);
	licqConf.WriteNum("AutoOffline", auto_offline_time);

	licqConf.FlushFile();
	licqConf.CloseFile();

	// Refresh contact list
	contact_list_refresh();
}

void
save_window_pos()
{
	char filename[MAX_FILENAME_LEN];
	snprintf(filename, MAX_FILENAME_LEN, "%s/%s", BASE_DIR, config_file());
	CIniFile licqConf(INI_FxERROR | INI_FxALLOWxCREATE);
	if (!licqConf.LoadFile(filename))
		return;
	gint x, y, w, h;

	gtk_window_get_position(GTK_WINDOW(main_window), &x, &y);
	gtk_window_get_size(GTK_WINDOW(main_window), &w, &h);

	licqConf.SetSection("appearance");

	licqConf.WriteNum("windowX", (short int)x);
	licqConf.WriteNum("windowY", (short int)y);
	licqConf.WriteNum("windowW", (short int)w);
	licqConf.WriteNum("windowH", (short int)h);
	
	licqConf.FlushFile();
	licqConf.CloseFile();
}

// load_options() is only called if the file exists
void 
load_options()
{
	online_color = new GdkColor;
	offline_color = new GdkColor;
	away_color = new GdkColor;

	char filename[MAX_FILENAME_LEN];
	snprintf(filename, MAX_FILENAME_LEN, "%s/%s", BASE_DIR, config_file());

	CIniFile licqConf;
	licqConf.LoadFile(filename);
	licqConf.SetSection("appearance");

	gLog.Info("%sLoading Jon's GTK2 GUI configuration", L_INITxSTR);

	// Online color
	licqConf.ReadNum("ColorOnline_Red", online_color->red, 8979);
	licqConf.ReadNum("ColorOnline_Green", online_color->green, 27457);
	licqConf.ReadNum("ColorOnline_Blue", online_color->blue, 63052);
	unsigned long l;
	licqConf.ReadNum("ColorOnline_Pixel", l, 0); online_color->pixel = l;
	
	// Offline color
	licqConf.ReadNum("ColorOffline_Red", offline_color->red, 59080);
	licqConf.ReadNum("ColorOffline_Green", offline_color->green, 0);
	licqConf.ReadNum("ColorOffline_Blue", offline_color->blue, 1660);
	licqConf.ReadNum("ColorOffline_Pixel", l, 0); offline_color->pixel = l;

	// Away color
	licqConf.ReadNum("ColorAway_Red", away_color->red, 0);
	licqConf.ReadNum("ColorAway_Green", away_color->green, 30000);
	licqConf.ReadNum("ColorAway_Blue", away_color->blue, 0);
	licqConf.ReadNum("ColorAway_Pixel", l, 0); away_color->pixel = l;

	// General options
	licqConf.ReadBool("ShowOfflineUsers", show_offline_users, true);
	licqConf.ReadBool("ShowIgnoredUsers", show_ignored_users, false);
	licqConf.ReadBool("EnterSends", enter_sends, true);
	licqConf.ReadBool("FlashEvents", flash_events, true);
	licqConf.ReadBool("RecvColors", recv_colors, true);
	licqConf.ReadBool("ShowTimestamp", show_convo_timestamp, true);
	licqConf.ReadStr("TimestampFormat", timestamp_format, "%H:%M:%S");
	licqConf.ReadBool("RememberWindowPos", remember_window_pos, true);

	// Window size & position
	licqConf.ReadNum("windowX", windowX, 0);
	licqConf.ReadNum("windowY", windowY, 0);
	licqConf.ReadNum("windowH", windowH, 245);
	licqConf.ReadNum("windowW", windowW, 160);
	
	// Auto logon
	licqConf.ReadNum("AutoLogon", auto_logon, ICQ_STATUS_OFFLINE);
	
	// Auto away, na, offline
	licqConf.ReadNum("AutoAway", auto_away_time, 10);
	licqConf.ReadNum("AutoNA", auto_na_time, 0);
	licqConf.ReadNum("AutoOffline", auto_offline_time, 0);
}

/*
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
*/
