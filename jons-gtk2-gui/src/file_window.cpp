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

#include <stdlib.h>	// for char *getenv(const char *);
#include "licq_gtk.h"

#include "licq_icqd.h"
#include "licq_message.h"
#include "licq_filetransfer.h"
#include "licq_user.h"

GSList *fs_list;

void file_accept_window(ICQUser *user, CUserEvent *e, bool auto_accept)
{
	struct file_accept *fa = (struct file_accept *)g_new0(struct file_accept, 1);
	fa->user = user;
	fa->e = e;

	if(auto_accept)
	{
		accept_file(0, (gpointer)fa);
		return;
	}

	GtkWidget *accept;
	GtkWidget *refuse;
	GtkWidget *v_box;
	GtkWidget *h_box;
	GtkWidget *label;
	const gchar *title = g_strdup_printf("File From %s", user->GetAlias());

	// Make the window
	fa->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
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
	accept = gtk_button_new_with_mnemonic("_Accept");
	refuse = gtk_button_new_with_mnemonic("_Refuse");

	// Pack them
	gtk_box_pack_start(GTK_BOX(h_box), accept, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), refuse, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 10);

	// Connect the signals
	g_signal_connect(G_OBJECT(fa->window), "destroy",
			   G_CALLBACK(dialog_close), fa->window);
	g_signal_connect(G_OBJECT(refuse), "clicked",
			   G_CALLBACK(refuse_file), (gpointer)fa);
	g_signal_connect(G_OBJECT(accept), "clicked",
			   G_CALLBACK(accept_file), (gpointer)fa);

	gtk_container_add(GTK_CONTAINER(fa->window), v_box);
	gtk_widget_show_all(fa->window);
}

void refuse_file(GtkWidget *widget, gpointer _fa)
{
	struct file_accept *fa = (struct file_accept *)_fa;

	// Close the unnecessary open window
	dialog_close(0, fa->window);

	// Create a window to get a reason for not accepting it
	
	// The window
	fa->window2 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(fa->window2), "File Refusal");
	
	// The box for the items in the window
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(fa->window2), v_box);

	// The text for the refusal
	fa->text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(fa->text), TRUE);
	gtk_box_pack_start(GTK_BOX(v_box), fa->text, FALSE, FALSE, 0);

	// The ok button
	GtkWidget *ok = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_box_pack_start(GTK_BOX(v_box), ok, FALSE, FALSE, 0);

	// Connect all the signals
	g_signal_connect(G_OBJECT(ok), "clicked",
			   G_CALLBACK(refusal_ok), (gpointer)fa);
	g_signal_connect(G_OBJECT(fa->window2), "destroy",
			   G_CALLBACK(refusal_ok), (gpointer)fa);
	
	gtk_widget_show_all(fa->window2);
}

void refusal_ok(GtkWidget *widget, gpointer _fa)
{
	struct file_accept *fa = (struct file_accept *)_fa;
	const char *reason = gtk_editable_get_chars(GTK_EDITABLE(fa->text),
				0, -1);
	CEventFile *f = (CEventFile *)fa->e;
	

	// The user gave a reason
	if((strcmp(reason, "") != 0))
	{
		icq_daemon->icqFileTransferRefuse(fa->user->Uin(),
			reason, fa->e->Sequence(), 
			f->MessageID(),
			f->IsDirect());
	}
	
	// Use a default reason
	else
	{
		icq_daemon->icqFileTransferRefuse(fa->user->Uin(),
			"No reason given.", fa->e->Sequence(),
			f->MessageID(),
			f->IsDirect()
			);
	}

	dialog_close(0, fa->window2);
}

void accept_file(GtkWidget *widget, gpointer _fa)
{
	struct file_accept *fa = (struct file_accept *)_fa;

	// Close the unnecessary open window
	if(fa->window)
		dialog_close(0, fa->window);

	save_file(fa);
}

void save_file(struct file_accept *fa)
{
	struct file_window *fw;
	CEventFile *f = (CEventFile *)fa->e;
	
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

	fw->input_tag = gtk_input_add_full(fw->ftman->Pipe(), GDK_INPUT_READ,
				      file_pipe_callback, NULL, (gpointer)fw, NULL);

	// Actually accept the file
	icq_daemon->icqFileTransferAccept(fw->uin,
					  fw->ftman->LocalPort(),
					  fw->sequence,
					  f->MessageID(),
					  f->IsDirect());
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
	gtk_widget_set_size_request(fw->current_file_name, 200, 20);
	gtk_box_pack_start(GTK_BOX(h_box), fw->current_file_name, FALSE,
			   FALSE, 0);

	// Total number of files entry box
	fw->total_files = gtk_entry_new();
	gtk_widget_set_size_request(fw->total_files, 50, 20);
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
	gtk_widget_set_size_request(fw->local_file_name, 255, 20);
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
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(fw->progress), "%p%%");
	//gtk_progress_set_text_alignment(GTK_PROGRESS_BAR(fw->progress), 0.5, 0.5);
	//gtk_progress_set_format_string(GTK_PROGRESS_BAR(fw->progress), "%p%%");
	gtk_widget_set_size_request(fw->progress, 160, 20);
	gtk_box_pack_start(GTK_BOX(h_box), fw->progress, FALSE, FALSE, 0);

	// Current file size
	fw->file_size = gtk_entry_new();
	gtk_widget_set_size_request(fw->file_size, 90, 20);
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
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(fw->batch_progress), "%p%%");
	gtk_widget_set_size_request(fw->batch_progress, 160, 20);
	gtk_box_pack_start(GTK_BOX(h_box), fw->batch_progress, FALSE,
			   FALSE, 0);

	// Batch size entry box
	fw->batch_size = gtk_entry_new();
	gtk_widget_set_size_request(fw->batch_size, 90, 20);
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
	gtk_widget_set_size_request(fw->time, 65, 20);
	gtk_widget_set_sensitive(fw->time, FALSE);

	// Pack them
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), fw->time, FALSE, FALSE, 0);

	// The ETA label and entry box
	label = gtk_label_new("ETA:");
	fw->eta = gtk_entry_new();
	gtk_widget_set_size_request(fw->eta, 65, 20);
	gtk_widget_set_sensitive(fw->eta, FALSE);

	// Pack them
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), fw->eta, FALSE, FALSE, 0);
	
	// The BPS label and entry box
	label = gtk_label_new("BPS:");
	fw->bps = gtk_entry_new();
	gtk_widget_set_size_request(fw->bps, 65, 20);
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
	fw->cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_table_attach(GTK_TABLE(table), fw->cancel, 1, 2, 5, 6,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 3, 3);
	g_signal_connect(G_OBJECT(fw->cancel), "clicked",
			  G_CALLBACK(cancel_file), (gpointer)fw);
	
	gtk_widget_show_all(fw->window);
}

void cancel_file(GtkWidget *widget, gpointer _fw)
{
	struct file_window *fw = (struct file_window *)_fw;

	const gchar *label_text = gtk_button_get_label(GTK_BUTTON(fw->cancel));

	if(strcasecmp(label_text, "Cancel") == 0)
	{
		// Cancel the transfer
		fw->ftman->CloseFileTransfer();
	}

	gtk_input_remove(fw->input_tag);

	// Close this window
	dialog_close(0, fw->window);
}

void file_pipe_callback(gpointer data, gint pipe, GdkInputCondition cond)
{
	struct file_window *fw = (struct file_window *)data;

	// Read out the pending event
	gchar buf[32];
	read(fw->ftman->Pipe(), buf, 32);

	CFileTransferEvent *e = 0;

	while((e = fw->ftman->PopFileTransferEvent()) != 0)
	{
		switch(e->Command())
		{
      case FT_ERRORxBIND:
      {
        message_box("Unable to bind to a port.\nSee Network Log for "
                    "details.");
        fw->ftman->CloseFileTransfer();
        break;
      }

      case FT_ERRORxCONNECT:
      {
        message_box("Unable to reach remote host.\nSee Network Log for "
                    "details.");
        fw->ftman->CloseFileTransfer();
        break;
      }

      case FT_ERRORxRESOURCES:
      {
        message_box("Unable to create a thread.\nSee Network Log for "
                    "details.");
        fw->ftman->CloseFileTransfer();
        break;
      }

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
			break;
		  }

		  case FT_DONExBATCH:
		  {
		  	gtk_label_set_text(GTK_LABEL(fw->cancel), "Close");
		  	message_box("File Transfer:\nBatch Done");
			fw->ftman->CloseFileTransfer();
			break;
		  }

		  case FT_ERRORxCLOSED:
		  {
		  	message_box("File Transfer:\nRemote side disconnected");
			fw->ftman->CloseFileTransfer();
			break;
		  }

		  case FT_ERRORxFILE:
		  {
		  	message_box("File Transfer:\nFile I/0 Error");
			fw->ftman->CloseFileTransfer();
			break;
		  }
		  
		  case FT_ERRORxHANDSHAKE:
		  {
		  	message_box("File Transfer:\nHandshake error");
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
	time_t Time = time(0) - fw->ftman->StartTime();
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
	time_t eta = (time_t)(bytes_left / (fw->ftman->BytesTransfered() / Time));
	gtk_entry_set_text(GTK_ENTRY(fw->eta), g_strdup_printf("%02ld:%02ld:%02ld",
						eta / 3600, (eta % 3600) / 60, (eta % 60)));

	// Get a percentage, cast it to a float, and update the progress bar
	gfloat percent = ((gfloat)fw->ftman->FilePos() / fw->ftman->FileSize());
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(fw->progress), percent);

	// Batch File info

	// Batch size
	gtk_entry_set_text(GTK_ENTRY(fw->batch_size),
			   g_strdup_printf("%s/%s", encode_file_size(fw->ftman->BatchPos()),
			   		   encode_file_size(fw->ftman->BatchSize())));

	// Get a percentage, cast it to a float and update the batch progress bar
	gfloat batch_percent = ((gfloat)fw->ftman->BatchPos() / fw->ftman->BatchSize());
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(fw->batch_progress), 
			batch_percent);
}

gchar *encode_file_size(unsigned long size)
{
	gchar unit[6];
	
	if(size >= (1024 * 1024))
	{
		size /= (1024*1024) / 10;
		strcpy(unit, "MB");
	}
	else if(size >= 1024)
	{
		size /= (1024 / 10);
		strcpy(unit, "KB");
	}
	else if(size != 1)
	{
		size *= 10;
		strcpy(unit, "Bytes");
	}
	else
	{
		size *= 10;
		strcpy(unit, "Byte");
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
	if(fs != 0)
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

	// The destroy signal for the window
	g_signal_connect(G_OBJECT(fs->window), "destroy",
			   G_CALLBACK(fs_cancel_click), (gpointer)fs);

	// The box that will hold all the widgets for this window
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(fs->window), v_box);

	// VBox with the description label and text box
	GtkWidget *desc_v_box = gtk_vbox_new(FALSE, 0);
	GtkWidget *label = gtk_label_new("Description:");
	gtk_box_pack_start(GTK_BOX(desc_v_box), label, FALSE, FALSE, 0);
	fs->description = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(fs->description), TRUE);
	gtk_widget_set_size_request(fs->description, 100, 75);
	gtk_box_pack_start(GTK_BOX(desc_v_box), fs->description, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), desc_v_box, FALSE, FALSE, 0);

	// HBox with a "File:" label, file path entry box and a browse button
	GtkWidget *h_box = gtk_hbox_new(FALSE, 5);
	label = gtk_label_new("File:");
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 0);
	fs->file_path = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(h_box), fs->file_path, FALSE, FALSE, 0);
	fs->browse = gtk_button_new_with_mnemonic("_Browse");
	gtk_box_pack_start(GTK_BOX(h_box), fs->browse, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);

	// Send normal, urgent, to list, in an hbox
	h_box = gtk_hbox_new(FALSE, 5);
	fs->send_normal = gtk_radio_button_new_with_label(0, "Send Normal");
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
	fs->ok = gtk_button_new_from_stock(GTK_STOCK_OK);
	fs->cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_start(GTK_BOX(h_box), fs->ok, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), fs->cancel, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);

	// Connect the signals for the browse, ok, and cancel buttons
	g_signal_connect(G_OBJECT(fs->browse), "clicked",
			   G_CALLBACK(fs_browse_click), (gpointer)fs);
	g_signal_connect(G_OBJECT(fs->ok), "clicked",
			   G_CALLBACK(fs_ok_click), (gpointer)fs);
	g_signal_connect(G_OBJECT(fs->cancel), "clicked",
			   G_CALLBACK(fs_cancel_click), (gpointer)fs);
	
	// Show the window now
	gtk_widget_show_all(fs->window);

	// Make sure the window is closed when a user selects a button
	g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(fs->file_select)->ok_button),
				  "clicked", G_CALLBACK(file_select_ok),
				  (gpointer)fs);
	g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(fs->file_select)->cancel_button),
				"clicked", G_CALLBACK(file_select_cancel),
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
		message_box("File Transfer:\nYou must specify a file to send.");
		return;
	}

	// Only make this if we're really going to send it
	fs->etd = g_new0(struct e_tag_data, 1);

	// We expect to catch the event back
	catcher = g_slist_append(catcher, fs->etd);

	fs->etd->e_tag = icq_daemon->icqFileTransfer(fs->uin, file_name,
				gtk_editable_get_chars(GTK_EDITABLE(
					fs->description), 0, -1),
				ICQ_TCPxMSG_NORMAL, false);
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

	// It wasn't found, return 0
	return 0;
}

// See if the file was accepted or not
void file_start_send(ICQEvent *event)
{
	CExtendedAck *ea = event->ExtendedAck();
	CUserEvent *ue = event->UserEvent();
	
	if(ea == 0 || ue == 0)
	{
		gLog.Error("%sInternal error: file_start_send(): chat or file"
			   " request acknowledgement without extended "
			   "result.\n", L_ERRORxSTR);
		return;
	}

	if(!ea->Accepted())
	{
		ICQUser *u = gUserManager.FetchUser(event->Uin(), LOCK_R);
		const char *mes = g_strdup_printf("File Transfer with %s "
			"Refused:\n%s", u->GetAlias(), ea->Response());
		gUserManager.DropUser(u);
		message_box(mes);
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
	fw->input_tag = gtk_input_add_full(fw->ftman->Pipe(), GDK_INPUT_READ,
	                              file_pipe_callback, NULL, (gpointer)fw, NULL);
  CEventFile *f = (CEventFile *)event->UserEvent();
  ConstFileList fl;
  fl.push_back(f->Filename());
  fw->ftman->SendFiles(fl, ea->Port());
	
	return;
}

