/*
 * Licq GTK GUI Plugin
 *
 * Copyright (C) 2000, Jon Keating <jonkeating@norcom2000.com>
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

#include <stdlib.h>	// for char *getenv(const char *);
#include "licq_gtk.h"

#include "licq_icqd.h"
#include "licq_packets.h"
#include "licq_message.h"
#include "licq_filetransfer.h"
#include "licq_user.h"

GSList *fs_list;

void file_accept_window(ICQUser *user, CUserEvent *e)
{
	GtkWidget *accept;
	GtkWidget *refuse;
	GtkWidget *v_box;
	GtkWidget *h_box;
	GtkWidget *label;
	struct file_accept *fa = (struct file_accept *)g_new0(struct file_accept, 1);
	const gchar *title = g_strdup_printf("File From %s", user->GetAlias());

	fa->user = user;
	fa->e = e;


	// Make the window
	fa->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(fa->window), title);
	gtk_window_set_position(GTK_WINDOW(fa->window), GTK_WIN_POS_CENTER);

	// Make the boxes
	v_box = gtk_vbox_new(FALSE, 5);
	h_box = gtk_hbox_new(FALSE, 5);

	// The label
	const gchar *text = g_strdup_printf("File: %s (%ld bytes)",
					    ((CEventFile *)e)->Filename(),
					    ((CEventFile *)e)->FileSize());
	label = gtk_label_new(text);

	// Pack the label
	gtk_box_pack_start(GTK_BOX(v_box), label, FALSE, FALSE, 10);

	// The buttons
	accept = gtk_button_new_with_label("Accept");
	refuse = gtk_button_new_with_label("Refuse");

	// Pack them
	gtk_box_pack_start(GTK_BOX(h_box), accept, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), refuse, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 10);

	// Connect the signals
	gtk_signal_connect(GTK_OBJECT(fa->window), "destroy",
			   GTK_SIGNAL_FUNC(dialog_close), fa->window);
	gtk_signal_connect(GTK_OBJECT(refuse), "clicked",
			   GTK_SIGNAL_FUNC(refuse_file), fa);
	gtk_signal_connect(GTK_OBJECT(accept), "clicked",
			   GTK_SIGNAL_FUNC(accept_file), (gpointer)fa);
			 

	gtk_container_add(GTK_CONTAINER(fa->window), v_box);
	gtk_widget_show_all(fa->window);
}

void refuse_file(GtkWidget *widget, gpointer _fa)
{
	struct file_accept *fa = (struct file_accept *)_fa;

	// Close the unnecessary open window
	dialog_close(NULL, fa->window);

	// Create a window to get a reason for not accepting it
	
	// The window
	fa->window2 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(fa->window2), "File Refusal");
	
	// The box for the items in the window
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(fa->window2), v_box);

	// The text for the refusal
	fa->text = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(fa->text), TRUE);
	gtk_box_pack_start(GTK_BOX(v_box), fa->text, FALSE, FALSE, 0);

	// The ok button
	GtkWidget *ok = gtk_button_new_with_label("OK");
	gtk_box_pack_start(GTK_BOX(v_box), ok, FALSE, FALSE, 0);

	// Connect all the signals
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(refusal_ok), (gpointer)fa);
	gtk_signal_connect(GTK_OBJECT(fa->window2), "destroy",
			   GTK_SIGNAL_FUNC(refusal_ok), (gpointer)fa);

	gtk_widget_show_all(fa->window2);
}

void refusal_ok(GtkWidget *widget, gpointer _fa)
{
	struct file_accept *fa = (struct file_accept *)_fa;
	const char *reason = gtk_editable_get_chars(GTK_EDITABLE(fa->text),
				0, -1);

	// The user gave a reason
	if((strcmp(reason, "") != 0))
	{
		icq_daemon->icqFileTransferRefuse(fa->user->Uin(),
			reason, fa->e->Sequence());
	}
	
	// Use a default reason
	else
	{
		icq_daemon->icqFileTransferRefuse(fa->user->Uin(),
			"No reason given.", fa->e->Sequence());
	}

	dialog_close(NULL, fa->window2);
}

void accept_file(GtkWidget *widget, gpointer _fa)
{
	struct file_accept *fa = (struct file_accept *)_fa;

	// Close the unnecessary open window
	dialog_close(NULL, fa->window);

	save_file(fa);
}

void save_file(struct file_accept *fa)
{
	struct file_window *fw;
	
	fw = g_new0(struct file_window, 1);
	fw->uin = fa->user->Uin();
	fw->sequence = fa->e->Sequence();

	// Make the rest of the window and show it
	create_file_window(fw);

	// Create the file transfer manager and connect it's pipe
	fw->ftman = new CFileTransferManager(icq_daemon, fw->uin);
	fw->ftman->SetUpdatesEnabled(1);

	// Get the HOME environment variable
	const char *home = getenv("HOME");
	fw->ftman->ReceiveFiles(home);

	fw->input_tag = gdk_input_add(fw->ftman->Pipe(), GDK_INPUT_READ,
				      file_pipe_callback, (gpointer)fw);

	// Actually accept the file
	icq_daemon->icqFileTransferAccept(fw->uin,
					  fw->ftman->LocalPort(), fw->sequence);
}

void create_file_window(struct file_window *fw)
{
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *h_box;

	// The box for the entry widgets
	h_box = gtk_hbox_new(FALSE, 5);

	// The window
	fw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(fw->window),
			     g_strdup_printf("Licq - File Transfer (%ld)",
			     		     fw->uin));

	// Create the table and add it to the window
	table = gtk_table_new(7, 2, FALSE);
	gtk_container_add(GTK_CONTAINER(fw->window), table);

	// Current file label
	label = gtk_label_new("Current File:");
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 3);

	// Current file entry box
	fw->current_file_name = gtk_entry_new();
	gtk_widget_set_usize(fw->current_file_name, 200, 20);
	gtk_box_pack_start(GTK_BOX(h_box), fw->current_file_name, FALSE,
			   FALSE, 0);

	// Total number of files entry box
	fw->total_files = gtk_entry_new();
	gtk_widget_set_usize(fw->total_files, 50, 20);
	gtk_box_pack_start(GTK_BOX(h_box), fw->total_files, FALSE, FALSE, 0);

	// Attach the box with the current file namd and total files
	// to this table.  This will be the format for all of the entry
	// boxes to follow in this window.
	gtk_table_attach(GTK_TABLE(table), h_box, 1, 2, 0, 1,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 3, 3);

	// Local file name label
	label = gtk_label_new("File Name:");
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
			 GTK_FILL, GTK_FILL, 3, 3);

	// New box
	h_box = gtk_hbox_new(FALSE, 5);
	
	// Local file name entry box
	fw->local_file_name = gtk_entry_new();
	gtk_widget_set_usize(fw->local_file_name, 255, 20);
	gtk_box_pack_start(GTK_BOX(h_box), fw->local_file_name, FALSE,
			   FALSE, 0);

	// Attach it
	gtk_table_attach(GTK_TABLE(table), h_box, 1, 2, 1, 2,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 3, 3);
			   
	// Current file label
	label = gtk_label_new("File:");
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
			 GTK_FILL, GTK_FILL, 3, 3);

	// New box
	h_box = gtk_hbox_new(FALSE, 5);

	// Progress bar for current file
	fw->progress = gtk_progress_bar_new();
	gtk_progress_set_show_text(GTK_PROGRESS(fw->progress), TRUE);
	gtk_progress_set_text_alignment(GTK_PROGRESS(fw->progress), 0.5, 0.5);
	gtk_progress_set_format_string(GTK_PROGRESS(fw->progress), "%p%%");
	gtk_widget_set_usize(fw->progress, 160, 20);
	gtk_box_pack_start(GTK_BOX(h_box), fw->progress, FALSE, FALSE, 0);

	// Current file size
	fw->file_size = gtk_entry_new();
	gtk_widget_set_usize(fw->file_size, 90, 20);
	gtk_box_pack_start(GTK_BOX(h_box), fw->file_size, FALSE, FALSE, 0);

	// Attach it
	gtk_table_attach(GTK_TABLE(table), h_box, 1, 2, 2, 3,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 3, 3);

	// Batch label
	label = gtk_label_new("Batch:");
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
			 GTK_FILL, GTK_FILL, 3, 3);

	// New box
	h_box = gtk_hbox_new(FALSE, 5);
	
	// Batch progress bar
	fw->batch_progress = gtk_progress_bar_new();
	gtk_progress_set_show_text(GTK_PROGRESS(fw->batch_progress), TRUE);
	gtk_progress_set_text_alignment(GTK_PROGRESS(fw->batch_progress),
					0.5, 0.5);
	gtk_progress_set_format_string(GTK_PROGRESS(fw->batch_progress),
				       "%p%%");
	gtk_widget_set_usize(fw->batch_progress, 160, 20);
	gtk_box_pack_start(GTK_BOX(h_box), fw->batch_progress, FALSE,
			   FALSE, 0);

	// Batch size entry box
	fw->batch_size = gtk_entry_new();
	gtk_widget_set_usize(fw->batch_size, 90, 20);
	gtk_box_pack_start(GTK_BOX(h_box), fw->batch_size, FALSE, FALSE, 0);

	// Attach it
	gtk_table_attach(GTK_TABLE(table), h_box, 1, 2, 3, 4,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 3, 3);

	// This box will contain the time (how long it's been receiving),
	// ETA, and BPS
	h_box = gtk_hbox_new(FALSE, 5);
	
	// The time label and entry box
	label = gtk_label_new("Time:");
	fw->time = gtk_entry_new();
	gtk_widget_set_usize(fw->time, 65, 20);
	gtk_widget_set_sensitive(fw->time, FALSE);

	// Pack them
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), fw->time, FALSE, FALSE, 0);

	// The ETA label and entry box
	label = gtk_label_new("ETA:");
	fw->eta = gtk_entry_new();
	gtk_widget_set_usize(fw->eta, 65, 20);
	gtk_widget_set_sensitive(fw->eta, FALSE);

	// Pack them
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), fw->eta, FALSE, FALSE, 0);
	
	// The BPS label and entry box
	label = gtk_label_new("BPS:");
	fw->bps = gtk_entry_new();
	gtk_widget_set_usize(fw->bps, 65, 20);
	gtk_widget_set_sensitive(fw->bps, FALSE);

	// Pack them
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), fw->bps, FALSE, FALSE, 0);


	// Add the box to the table
	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 4, 5,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 3, 3);

	// Cancel button
	fw->cancel = gtk_button_new_with_label("Cancel");
	gtk_table_attach(GTK_TABLE(table), fw->cancel, 1, 2, 5, 6,
			 GTK_SHRINK, GTK_SHRINK, 3, 3);
	gtk_signal_connect(GTK_OBJECT(fw->cancel), "clicked",
			  GTK_SIGNAL_FUNC(cancel_file), (gpointer)fw);

	gtk_widget_show_all(fw->window);
}

void cancel_file(GtkWidget *widget, gpointer _fw)
{
	struct file_window *fw = (struct file_window *)_fw;

	// Cancel the transfer
	fw->ftman->CloseFileTransfer();
	gdk_input_remove(fw->input_tag);

	// Close this window
	dialog_close(NULL, fw->window);
}

void file_pipe_callback(gpointer data, gint pipe, GdkInputCondition cond)
{
	struct file_window *fw = (struct file_window *)data;

	// Read out the pending event
	gchar buf[32];
	read(fw->ftman->Pipe(), buf, 32);

	CFileTransferEvent *e = NULL;

	while((e = fw->ftman->PopFileTransferEvent()) != NULL)
	{
		switch(e->Command())
		{
		  case FT_STARTxBATCH:
		  {
		  	// File 1 out of x files
			gtk_entry_set_text(GTK_ENTRY(fw->total_files),
					   g_strdup_printf("1 / %d",
					   fw->ftman->BatchFiles()));
			// Total size of the batch
			gtk_entry_set_text(GTK_ENTRY(fw->batch_size),
					   encode_file_size(fw->ftman->BatchSize()));
			break;
		  }
		  
		  case FT_STARTxFILE:
		  {
		  	// File x out of y files
		  	gtk_entry_set_text(GTK_ENTRY(fw->total_files),
					   g_strdup_printf("%d / %d",
					     fw->ftman->CurrentFile(),
					     fw->ftman->BatchFiles()));
			// Current file name being transfered
			gtk_entry_set_text(GTK_ENTRY(fw->current_file_name),
					   fw->ftman->FileName());
			// Local filename that it's being saved to
			gtk_entry_set_text(GTK_ENTRY(fw->local_file_name),
					   fw->ftman->PathName());
			// Current file size
			gtk_entry_set_text(GTK_ENTRY(fw->file_size),
					   encode_file_size(fw->ftman->FileSize()));
			break;
		  }

		  case FT_UPDATE:
		  {
		  	// Update the info 
			update_file_info(fw);
			break;
		  }

		  case FT_DONExFILE:
		  {
		  	update_file_info(fw);
			g_print("File done\n");
			break;
		  }

		  case FT_DONExBATCH:
		  {
		  	g_print("Batch Done\n");
			fw->ftman->CloseFileTransfer();
			break;
		  }

		  case FT_ERRORxCLOSED:
		  {
		  	g_print("Remote side disconnected\n");
			fw->ftman->CloseFileTransfer();
			break;
		  }

		  case FT_ERRORxFILE:
		  {
		  	g_print("File I/O Error\n");
			fw->ftman->CloseFileTransfer();
			break;
		  }
		  
		  case FT_ERRORxHANDSHAKE:
		  {
		  	g_print("Handshake error\n");
			fw->ftman->CloseFileTransfer();
			break;
		  }
		}

		delete e;
	}
}

void update_file_info(struct file_window *fw)
{
	// Current File info

	// Time
	time_t Time = time(NULL) - fw->ftman->StartTime();
	gtk_entry_set_text(GTK_ENTRY(fw->time), g_strdup_printf("%02ld:%02ld:%02ld",
								Time / 3600,
								(Time % 3600) / 60,
								(Time % 60)));

	if(Time == 0 || fw->ftman->BytesTransfered() == 0)
	{
		gtk_entry_set_text(GTK_ENTRY(fw->bps), "---");
		gtk_entry_set_text(GTK_ENTRY(fw->eta), "---");
		return;
	}

	// BPS
	gtk_entry_set_text(GTK_ENTRY(fw->bps),
			   g_strdup_printf("%s/s",
			       encode_file_size(fw->ftman->BytesTransfered() / Time)));

	// ETA
	int bytes_left = fw->ftman->FileSize() - fw->ftman->FilePos();
	time_t eta = (time_t)(bytes_left / fw->ftman->BytesTransfered() / Time);
	gtk_entry_set_text(GTK_ENTRY(fw->eta), g_strdup_printf("%02ld:%02ld:%02ld",
						eta / 3600, (eta % 3600) / 60, (eta % 60)));

	// Get a percentage, cast it to a float, and update the progress bar
	gfloat percent = ((gfloat)fw->ftman->FilePos() / fw->ftman->FileSize());
	gtk_progress_set_percentage(GTK_PROGRESS(fw->progress), percent);

	// Batch File info

	// Batch size
	gtk_entry_set_text(GTK_ENTRY(fw->batch_size),
			   g_strdup_printf("%s/%s", encode_file_size(fw->ftman->BatchPos()),
			   		   encode_file_size(fw->ftman->BatchSize())));

	// Get a percentage, cast it to a float and update the batch progress bar
	gfloat batch_percent = ((gfloat)fw->ftman->BatchPos() / fw->ftman->BatchSize());
	gtk_progress_set_percentage(GTK_PROGRESS(fw->batch_progress), batch_percent);
}

gchar *encode_file_size(unsigned long size)
{
	gchar *unit;
	
	if(size >= (1024 * 1024))
	{
		size /= (1024*1024) / 10;
		unit = "MB";
	}
	else if(size >= 1024)
	{
		size /= (1024 / 10);
		unit = "KB";
	}
	else if(size != 1)
	{
		size *= 10;
		unit = "Bytes";
	}
	else
	{
		size *= 10;
		unit = "Byte";
	}

	return g_strdup_printf("%ld.%ld %s", (size / 10), (size % 10), unit);
}

// Function to send a file from the contact list
void list_request_file(GtkWidget *widget, ICQUser *user)
{
	struct file_send *fs;
	
	// Does it already exist?
	fs = fs_find(user->Uin());
	
	// Don't make it if it already exists
	if(fs != NULL)
		return;
		
	fs = g_new0(struct file_send, 1);
	fs->uin = user->Uin();
	fs->file_select = gtk_file_selection_new(g_strdup_printf("Select file to"				" send to %s", user->GetAlias()));

	// Add this to the linked list
	fs_list = g_slist_append(fs_list, fs);

	// Make the window
	fs->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(fs->window), g_strdup_printf(
			     "Send file to %s", user->GetAlias()));

	// The box that will hold all the widgets for this window
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(fs->window), v_box);

	// Description text box
	fs->description = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(fs->description), TRUE);
	gtk_box_pack_start(GTK_BOX(v_box), fs->description, FALSE, FALSE, 0);

	// HBox with a file path entry box and a browse button
	GtkWidget *h_box = gtk_hbox_new(FALSE, 5);
	fs->file_path = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(h_box), fs->file_path, FALSE, FALSE, 0);
	fs->browse = gtk_button_new_with_label("Browse");
	gtk_box_pack_start(GTK_BOX(h_box), fs->browse, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);

	// Send normal, urgent, to list, in an hbox
	h_box = gtk_hbox_new(FALSE, 5);
	fs->send_normal = gtk_radio_button_new_with_label(NULL, "Send Normal");
	fs->send_urgent = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(fs->send_normal), "Send Urgent");
	fs->send_list = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(fs->send_normal), "Send to List");

	gtk_box_pack_start(GTK_BOX(h_box), fs->send_normal, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), fs->send_urgent, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), fs->send_list, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);

	// The hbox with the ok and cancel buttons
	h_box = gtk_hbox_new(FALSE, 5);
	fs->ok = gtk_button_new_with_label("OK");
	fs->cancel = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(h_box), fs->ok, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), fs->cancel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);

	// Connect the signals for the browse, ok, and cancel buttons
	gtk_signal_connect(GTK_OBJECT(fs->browse), "clicked",
			   GTK_SIGNAL_FUNC(fs_browse_click), (gpointer)fs);
	gtk_signal_connect(GTK_OBJECT(fs->ok), "clicked",
			   GTK_SIGNAL_FUNC(fs_ok_click), (gpointer)fs);
	gtk_signal_connect(GTK_OBJECT(fs->cancel), "clicked",
			   GTK_SIGNAL_FUNC(fs_cancel_click), (gpointer)fs);

	// Show the window now
	gtk_widget_show_all(fs->window);

	// Make sure the window is closed when a user selects a button
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs->file_select)->ok_button),
				  "clicked", GTK_SIGNAL_FUNC(file_select_ok),
				  (gpointer)fs);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs->file_select)->cancel_button),
				"clicked", GTK_SIGNAL_FUNC(file_select_cancel),
				(gpointer)fs);

	// Show the file selection now
	gtk_widget_show_all(fs->file_select);
}

void fs_browse_click(GtkWidget *widget, gpointer _fs)
{
	struct file_send *fs = (struct file_send *)_fs;
	gtk_widget_show_all(fs->file_select);
}

void fs_cancel_click(GtkWidget *widget, gpointer _fs)
{
	struct file_send *fs = (struct file_send *)_fs;

	// Get rid of the file send list and destory the window
	fs_list = g_slist_remove(fs_list, fs);
	gtk_widget_destroy(fs->window);
}

void file_select_ok(GtkWidget *widget, gpointer _fs)
{
	struct file_send *fs = (struct file_send *)_fs;
	gtk_widget_hide_all(fs->file_select);
	gtk_entry_set_text(GTK_ENTRY(fs->file_path),
		gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs->file_select)));
}

void file_select_cancel(GtkWidget *widget, gpointer _fs)
{
	struct file_send *fs = (struct file_send *)_fs;
	gtk_widget_hide_all(fs->file_select);
}

void fs_ok_click(GtkWidget *widget, gpointer _fs)
{
	struct file_send *fs = (struct file_send *)_fs;
	const char *file_name = gtk_file_selection_get_filename(
					GTK_FILE_SELECTION(fs->file_select));

	if(strcmp(file_name, "") == 0)
	{
		g_print("You must specify a file to send\n");
		return;
	}

	// Only make this if we're really going to send it
	fs->etd = g_new0(struct e_tag_data, 1);

	// We expect to catch the event back
	catcher = g_slist_append(catcher, fs->etd);

	fs->etd->e_tag = icq_daemon->icqFileTransfer(fs->uin, file_name,
				gtk_editable_get_chars(GTK_EDITABLE(
					fs->description), 0, -1),
				ICQ_TCPxMSG_NORMAL);
}

// Used for the finishing event in extras.cpp
struct file_send *fs_find(gulong uin)
{
	struct file_send *fs;
	GSList *temp_fs_list = fs_list;

	while(temp_fs_list)
	{
		fs = (struct file_send *)temp_fs_list->data;
		if(fs->uin == uin)
			return fs;

		temp_fs_list = temp_fs_list->next;
	}

	// It wasn't found, return NULL
	return NULL;
}

// See if the file was accepted or not
void file_start_send(ICQEvent *event)
{
	CExtendedAck *ea = event->ExtendedAck();
	CUserEvent *ue = event->UserEvent();
	
	if(ea == NULL || ue == NULL)
	{
		gLog.Error("%sInternal error: file_start_send(): chat or file"
			   " request acknowledgement without extended "
			   "result.\n", L_ERRORxSTR);
		return;
	}

	if(!ea->Accepted())
	{
		return;
	}

	// It is sending now
	struct file_window *fw = g_new0(file_window, 1);
	fw->uin = event->Uin();
	fw->sequence = ue->Sequence();

	create_file_window(fw);

	// Create the file transfer manager and connect it's pipe
	fw->ftman = new CFileTransferManager(icq_daemon, fw->uin);
	fw->ftman->SetUpdatesEnabled(1);
	fw->input_tag = gdk_input_add(fw->ftman->Pipe(), GDK_INPUT_READ,
	                              file_pipe_callback, (gpointer)fw);

        CEventFile *f = (CEventFile *)event->UserEvent();
        ConstFileList fl;
        fl.push_back(f->Filename());
        if(!fw->ftman->SendFiles(fl, ea->Port()))
		return;

}

