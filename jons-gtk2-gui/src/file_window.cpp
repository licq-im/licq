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
#include "licq_log.h"
#include "utilities.h"

using namespace std;

GSList *fs_list;

struct file_window
{
	/* File sending/receiving internals */
	CFileTransferManager *ftman;
	gulong uin;
	gint input_tag;
	gulong sequence;

	/* For the window */
	GtkWidget *window;
	GtkWidget *current_file_name;
	GtkWidget *total_files;
	GtkWidget *local_file_name;
	GtkWidget *batch_progress;
	GtkWidget *file_progress;
	GtkWidget *time;
	GtkWidget *eta;
	GtkWidget *status;
	GtkWidget *cancel;
};

struct file_accept
{
	GtkWidget *window;
	GtkWidget *window2;
	ICQUser *user;
	CUserEvent *e;
	GtkWidget *text;  /* This is for the refuse part... */
};

struct file_send
{
	GtkWidget *window;		// Window
	GtkWidget *description;		// File description
	GtkWidget *file_path;		// File path to send
	GtkWidget *browse;		// Browse for new file
	GtkWidget *ok;			// Send
	GtkWidget *cancel;		// Cancel before it is sent
	GtkWidget *send_normal;		// Send it normally
	GtkWidget *send_urgent;		// Send it urgently
	GtkWidget *send_list;		// Send it to their list

	/* Internals */
	gulong uin;
	struct e_tag_data *etd;
};

void create_file_window(struct file_window *fw);
struct file_send *fs_find(gulong uin);
void accept_file(GtkWidget *, struct file_accept *fa);
void refuse_file(GtkWidget *, struct file_accept *fa);
void cancel_file(GtkWidget *, struct file_window *fw);
void file_pipe_callback(gpointer, gint, GdkInputCondition);

gboolean
fa_delete(GtkWidget *, GdkEvent *, gpointer data)
{
	g_free(data);
	return FALSE;
}

void 
file_accept_window(ICQUser *user, CUserEvent *e, bool auto_accept)
{
	struct file_accept *fa = g_new0(struct file_accept, 1);
	fa->user = user;
	fa->e = e;

	if (auto_accept) {
		accept_file(0, fa);
		return;
	}

	// Make the window
	fa->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	char *title = g_strdup_printf("File From %s", user->GetAlias());
	gtk_window_set_title(GTK_WINDOW(fa->window), title);
	g_free(title);
	gtk_window_set_position(GTK_WINDOW(fa->window), GTK_WIN_POS_CENTER);

	// Make the boxes
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);

	// Description
	GtkWidget *tv = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(tv), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tv), GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(tv), FALSE);
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));
	gtk_text_buffer_set_text(tb, e->Text(), -1);
	gtk_widget_set_size_request(tv, 300, -1);
	
	GtkWidget *frame = gtk_frame_new(0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN); 
	gtk_container_add(GTK_CONTAINER(frame), tv);
	gtk_box_pack_start(GTK_BOX(v_box), frame, FALSE, FALSE, 5);

	// The buttons
	GtkWidget *accept = gtk_button_new_with_mnemonic("_Accept");
	GtkWidget *refuse = gtk_button_new_with_mnemonic("_Refuse");

	// Pack them
	GtkWidget *h_box = hbutton_box_new();
	gtk_container_add(GTK_CONTAINER(h_box), accept);
	gtk_container_add(GTK_CONTAINER(h_box), refuse);

	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(fa->window), v_box);
	
	// Connect the signals
	g_signal_connect(G_OBJECT(fa->window), "delete_event",
			   G_CALLBACK(fa_delete), fa);
	g_signal_connect(G_OBJECT(refuse), "clicked",
			   G_CALLBACK(refuse_file), fa);
	g_signal_connect(G_OBJECT(accept), "clicked",
			   G_CALLBACK(accept_file), fa);

	gtk_container_set_border_width(GTK_CONTAINER(fa->window), 10);
	gtk_widget_show_all(fa->window);
}

void
refusal_ok(GtkWidget *widget, struct file_accept *fa)
{
	CEventFile *f = (CEventFile *)fa->e;

	string reason(textview_get_chars(fa->text));
	if (reason.empty()) // user gave no reason
		reason = "No reason given.";

	icq_daemon->icqFileTransferRefuse(fa->user->Uin(),
		reason.c_str(), fa->e->Sequence(), 
		f->MessageID(),
		f->IsDirect());

	// once refused close all windows
	window_close(0, fa->window2);
	window_close(0, fa->window);
	g_free(fa);
}

void
refuse_file(GtkWidget *widget, struct file_accept *fa)
{
	// Create a window to get a reason for not accepting it
	fa->window2 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(fa->window2), "Refuse File Request");
	
	// The box for the items in the window
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(fa->window2), v_box);

	// The text for the refusal
	fa->text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(fa->text), TRUE);
	gtk_widget_set_size_request(fa->text, 300, 50);

	GtkWidget *frame = gtk_frame_new("Reason:");
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN); 
	gtk_container_add(GTK_CONTAINER(frame), fa->text);
	gtk_box_pack_start(GTK_BOX(v_box), frame, FALSE, FALSE, 0);

	// Buttons
	GtkWidget *h_box = hbutton_box_new();
	GtkWidget *ok = gtk_button_new_from_stock(GTK_STOCK_OK);
	GtkWidget *cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_container_add(GTK_CONTAINER(h_box), ok);
	gtk_container_add(GTK_CONTAINER(h_box), cancel);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);

	// Connect all the signals
	g_signal_connect(G_OBJECT(ok), "clicked",
			   G_CALLBACK(refusal_ok), fa);
	g_signal_connect(G_OBJECT(cancel), "clicked",
			   G_CALLBACK(window_close), fa->window2);
	
	gtk_container_set_border_width(GTK_CONTAINER(fa->window2), 10);
	gtk_widget_show_all(fa->window2);
}

void
save_file(struct file_accept *fa, char *filename)
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
	char *p = strrchr(filename, '/');
	if (p != NULL)
		*p = '\0';
	
	fw->ftman->ReceiveFiles(filename);

	fw->input_tag = gtk_input_add_full(fw->ftman->Pipe(), GDK_INPUT_READ,
			file_pipe_callback, NULL, fw, NULL);

	// Actually accept the file
	icq_daemon->icqFileTransferAccept(fw->uin,
			fw->ftman->LocalPort(),
			fw->sequence,
			f->MessageID(),
			f->IsDirect());
}

void
accept_file(GtkWidget *widget, struct file_accept *fa)
{
	GtkWidget *file_selector = 
			gtk_file_selection_new("Please select destination directory");
	char *dest_dir = g_strdup_printf("%s/", getenv("HOME"));
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selector), dest_dir);
	int response = gtk_dialog_run(GTK_DIALOG(file_selector));
	if (response == GTK_RESPONSE_OK) {
		char *file_name = g_strdup(
				gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector)));
		gtk_widget_destroy(GTK_WIDGET(file_selector));
		if (fa->window)
			window_close(0, fa->window);
		save_file(fa, file_name);
		g_free(fa);
	}
	else
		gtk_widget_destroy(GTK_WIDGET(file_selector));
}

GtkWidget *
ro_entry_new(int width_chars, const char *sample = NULL)
{
	GtkWidget *entry = gtk_entry_new();
	//gtk_widget_set_size_request(fw->current_file_name, 200, 20);
	if (sample == NULL)
		gtk_entry_set_width_chars(GTK_ENTRY(entry), width_chars);
	else {
		PangoLayout *layout = gtk_entry_get_layout(GTK_ENTRY(entry));
		pango_layout_set_text(layout, sample, -1);
		int wd, ht;
		pango_layout_get_pixel_size(layout, &wd, &ht);
		gtk_widget_set_size_request(entry, wd + 20, -1);
		pango_layout_set_text(layout, "", -1);
	}
	gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	
	return entry;
}

void
create_file_window(struct file_window *fw)
{
	// The window
	fw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	char *title = g_strdup_printf("Licq - File Transfer (%ld)", fw->uin);
	gtk_window_set_title(GTK_WINDOW(fw->window), title);
	g_free(title);

	// Create the table and add it to the window
	GtkWidget *table = gtk_table_new(7, 2, FALSE);
	gtk_container_add(GTK_CONTAINER(fw->window), table);

	// Current file label
	gtk_table_attach(GTK_TABLE(table),
			gtk_label_new("Current File:"), 
			0, 1, 0, 1,
			GTK_FILL, GTK_FILL, 3, 3);

	// Current file entry box
	fw->current_file_name = ro_entry_new(50);
	gtk_table_attach(GTK_TABLE(table),
			fw->current_file_name, 
			1, 2, 0, 1,
			GTK_FILL, GTK_FILL, 3, 3);

	// Total number of files entry box
	fw->total_files = ro_entry_new(0, "99 / 99");
	gtk_table_attach(GTK_TABLE(table),
			fw->total_files, 
			2, 3, 0, 1,
			GTK_FILL, GTK_FILL, 3, 3);

	// Local file name label
	gtk_table_attach(GTK_TABLE(table), gtk_label_new("File Name:"), 
			0, 1, 1, 2,
			GTK_FILL, GTK_FILL, 3, 3);

	// Local file name entry box
	fw->local_file_name = ro_entry_new(50);

	// Attach it
	gtk_table_attach(GTK_TABLE(table), 
			fw->local_file_name,
			1, 3, 1, 2,
			GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			3, 3);
			   
	// Current file label
	gtk_table_attach(GTK_TABLE(table), gtk_label_new("File:"), 
			0, 1, 2, 3,
			GTK_FILL, GTK_FILL, 3, 3);

	// Progress bar for current file
	fw->file_progress = gtk_progress_bar_new();
	gtk_table_attach(GTK_TABLE(table), 
			fw->file_progress,
			1, 3, 2, 3,
			GTK_FILL, GTK_FILL, 3, 3);

	// Batch label
	gtk_table_attach(GTK_TABLE(table), 
			gtk_label_new("Batch:"), 
			0, 1, 3, 4,
			GTK_FILL, GTK_FILL, 3, 3);

	// Batch progress bar
	fw->batch_progress = gtk_progress_bar_new();

	// Batch size entry box
	gtk_table_attach(GTK_TABLE(table), 
			fw->batch_progress,
			1, 3, 3, 4,
			GTK_FILL, GTK_FILL, 3, 3);

	// This box will contain the time (how long it's been receiving),
	// ETA, and BPS
	GtkWidget *h_box = gtk_hbox_new(FALSE, 5);
	
	// The time label and entry box
	gtk_box_pack_start(GTK_BOX(h_box), gtk_label_new("Time:"), FALSE, FALSE, 5);
	fw->time = ro_entry_new(0, "00:00:00");
	gtk_box_pack_start(GTK_BOX(h_box), fw->time, FALSE, FALSE, 0);

	// The ETA label and entry box
	gtk_box_pack_start(GTK_BOX(h_box), gtk_label_new("ETA:"), FALSE, FALSE, 5);
	fw->eta = ro_entry_new(0, "00:00:00");
	gtk_box_pack_start(GTK_BOX(h_box), fw->eta, FALSE, FALSE, 0);
	
	// The BPS label and entry box
	//gtk_box_pack_start(GTK_BOX(h_box), gtk_label_new("BPS:"), FALSE, FALSE, 5);
	//fw->bps = ro_entry_new(0, "000.0 Bytes/s");
	//gtk_box_pack_start(GTK_BOX(h_box), fw->bps, FALSE, FALSE, 0);

	// Add the box to the table
	gtk_table_attach(GTK_TABLE(table), 
			h_box, 
			1, 3, 4, 5,
			GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			3, 3);

	// Cancel button
  fw->cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
  GtkWidget *bbox = hbutton_box_new();
  gtk_container_add(GTK_CONTAINER(bbox), fw->cancel);
  
	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	fw->status = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(hbox), fw->status, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(hbox), bbox, FALSE, FALSE, 0);
	
	gtk_table_attach(GTK_TABLE(table), 
			hbox, 
			0, 3, 5, 6,
			GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			3, 3);

	g_signal_connect(G_OBJECT(fw->cancel), "clicked",
			  G_CALLBACK(cancel_file), fw);
	
	gtk_widget_show_all(fw->window);
}

void 
cancel_file(GtkWidget *widget, struct file_window *fw)
{
	const gchar *label_text = gtk_button_get_label(GTK_BUTTON(fw->cancel));

	if (strcasecmp(label_text, "Cancel") == 0)
		// Cancel the transfer
		fw->ftman->CloseFileTransfer();

	gtk_input_remove(fw->input_tag);

	// Close this window
	window_close(0, fw->window);
}

string
to_str_free(char *p)
{
	string s(p);
	g_free(p);
	return s;
}

std::string
encode_file_size(unsigned long size)
{
	gchar unit[6];
	
	if (size >= (1024 * 1024)) {
		size /= (1024*1024) / 10;
		strcpy(unit, "MB");
	}
	else if (size >= 1024) {
		size /= (1024 / 10);
		strcpy(unit, "KB");
	}
	else if (size != 1) {
		size *= 10;
		strcpy(unit, "Bytes");
	}
	else {
		size *= 10;
		strcpy(unit, "Byte");
	}

	return 
		to_str_free(g_strdup_printf("%ld.%ld %s", (size / 10), (size % 10), unit));
}

void 
update_file_info(struct file_window *fw)
{
	// Time
	time_t elapsed = time(0) - fw->ftman->StartTime();

	if (fw->ftman->FileSize() == 0 || fw->ftman->BatchSize() == 0)
		return;

	char *txt;
	if (elapsed != 0) {
		txt = g_strdup_printf("%02ld:%02ld:%02ld",
				elapsed / 3600, (elapsed % 3600) / 60, (elapsed % 60));
		gtk_entry_set_text(GTK_ENTRY(fw->time), txt);
		g_free(txt);
	}
	
	// File progress
	if (elapsed != 0)
		txt = g_strdup_printf("%s/%s, %s/s",
				encode_file_size(fw->ftman->FilePos()).c_str(),
				encode_file_size(fw->ftman->FileSize()).c_str(),
				encode_file_size(fw->ftman->BytesTransfered() / elapsed).c_str());
	else
		txt = g_strdup_printf("%s/%s",
				encode_file_size(fw->ftman->FilePos()).c_str(),
				encode_file_size(fw->ftman->FileSize()).c_str());
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(fw->file_progress), txt);
	g_free(txt);

	// Update progress percent
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(fw->file_progress), 
			((gfloat)fw->ftman->FilePos() / fw->ftman->FileSize()));

	// Batch progress
	if (elapsed != 0)
		txt = g_strdup_printf("%s/%s, %s/s",
				encode_file_size(fw->ftman->BatchPos()).c_str(),
				encode_file_size(fw->ftman->BatchSize()).c_str(),
				encode_file_size(fw->ftman->BatchPos() / elapsed).c_str());
	else
		txt = g_strdup_printf("%s/%s",
				encode_file_size(fw->ftman->BatchPos()).c_str(),
				encode_file_size(fw->ftman->BatchSize()).c_str());
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(fw->batch_progress), txt);
	g_free(txt);

	// Get a percentage, cast it to a float and update the batch progress bar
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(fw->batch_progress), 
			((gfloat)fw->ftman->BatchPos() / fw->ftman->BatchSize()));

	// ETA
	if (elapsed != 0) {
		long bytes_left = fw->ftman->BatchSize() - fw->ftman->BatchPos();
		time_t eta = (time_t)(bytes_left / (fw->ftman->BatchPos() / elapsed));
		txt = g_strdup_printf("%02ld:%02ld:%02ld",
							eta / 3600, (eta % 3600) / 60, (eta % 60));
		gtk_entry_set_text(GTK_ENTRY(fw->eta), txt);
		g_free(txt);
	}
}

void
transfer_finish(file_window *fw, const char *status_txt)
{
	gtk_button_set_label(GTK_BUTTON(fw->cancel), GTK_STOCK_CLOSE);
	status_change(fw->status, "sta", status_txt);
  fw->ftman->CloseFileTransfer();
}

void
file_pipe_callback(gpointer data, gint pipe, GdkInputCondition cond)
{
	struct file_window *fw = (struct file_window *)data;

	// Read out the pending event
	gchar buf[32];
	read(fw->ftman->Pipe(), buf, 32);

	CFileTransferEvent *e = 0;
	
	while ((e = fw->ftman->PopFileTransferEvent()) != 0) {
		switch (e->Command())	{
		  case FT_STARTxBATCH:
			{
	  		// File 1 out of x files
				char *txt = g_strdup_printf("1 / %d", fw->ftman->BatchFiles());
				gtk_entry_set_text(GTK_ENTRY(fw->total_files), txt);
				g_free(txt);
				
				status_change(fw->status, "sta", "Starting the batch...");
				break;
			}
			
			case FT_CONFIRMxFILE:
				fw->ftman->StartReceivingFile(NULL);
				break;
			
		  case FT_STARTxFILE:
			{
		  	// File x out of y files
		  	char *txt = g_strdup_printf("%d / %d", 
						fw->ftman->CurrentFile(),
					  fw->ftman->BatchFiles());
				gtk_entry_set_text(GTK_ENTRY(fw->total_files), txt);
				g_free(txt);

				// Current file name being transfered
				gtk_entry_set_text(GTK_ENTRY(fw->current_file_name),
						fw->ftman->FileName());
				// Local filename that it's being saved to
				gtk_entry_set_text(GTK_ENTRY(fw->local_file_name),
						fw->ftman->PathName());
				
				if (fw->ftman->Direction() == D_RECEIVER)
					status_change(fw->status, "sta", "Receiving...");
				else
					status_change(fw->status, "sta", "Sending...");
				update_file_info(fw);

				break;
			}
			
		  case FT_UPDATE:
		  	// Update the info 
				update_file_info(fw);
				break;

		  case FT_DONExFILE:
		  	update_file_info(fw);
				break;

		  case FT_DONExBATCH:
		  	update_file_info(fw);
				transfer_finish(fw, "Batch complete");
				break;

      case FT_ERRORxBIND:
		  	transfer_finish(fw, 
						"Unable to bind to a port. See Network Log for details.");
        break;

      case FT_ERRORxCONNECT:
		  	transfer_finish(fw, 
        		"Unable to reach remote host. See Network Log for details.");
        break;

      case FT_ERRORxRESOURCES:
		  	transfer_finish(fw, 
        		"Unable to create a thread. See Network Log for details.");
        break;

		  case FT_ERRORxCLOSED:
		  	transfer_finish(fw, "Remote side disconnected");
				break;

		  case FT_ERRORxFILE:
			{
				char *msg = g_strdup_printf("File I/0 error on %s", 
						fw->ftman->PathName());
		  	transfer_finish(fw, msg);
				g_free(msg);
				break;
		  }
			
		  case FT_ERRORxHANDSHAKE:
		  	transfer_finish(fw, "Handshake error");
				break;
		}

		delete e;
	}
}

void
destroy_fs(GtkWidget *widget, struct file_send *fs)
{
	// Get rid of the file send list and destory the window
	fs_list = g_slist_remove(fs_list, fs);
	g_free(fs);
}

void
fs_ok_click(GtkWidget *widget, struct file_send *fs)
{
	string fname(entry_get_chars(fs->file_path));
	if (fname.empty()) {
		message_box("File Transfer:\nYou must specify a file to send.");
		return;
	}
	
	fs->etd = g_new0(struct e_tag_data, 1);
	catcher = g_slist_append(catcher, fs->etd);

	ConstFileList fl;
	fl.push_back(fname.c_str());

	fs->etd->e_tag = icq_daemon->icqFileTransfer(
			fs->uin, 
			fname.c_str(),
			textview_get_chars(fs->description).c_str(),
			fl, ICQ_TCPxMSG_NORMAL, false);
}

void
fs_browse_click(GtkWidget *widget, struct file_send *fs)
{
	char *txt = g_strdup_printf("Select file(s) to send to %ld", fs->uin);
	GtkWidget *filesel = gtk_file_selection_new(txt);
	g_free(txt);
	
	int response = gtk_dialog_run(GTK_DIALOG(filesel));
	if (response == GTK_RESPONSE_OK) {
		const char *fname = 
				gtk_file_selection_get_filename(GTK_FILE_SELECTION(filesel));
		gtk_entry_set_text(GTK_ENTRY(fs->file_path), fname);
		if (fname != NULL && fname[0] != '\0')
			gtk_widget_set_sensitive(fs->ok, TRUE);
	}

	gtk_widget_destroy(filesel);
}

// Function to send a file from the contact list
void
list_request_file(GtkWidget *widget, ICQUser *user)
{
	// Does it already exist?
	struct file_send *fs = fs_find(user->Uin());
	
	// Don't make it if it already exists
	if (fs != 0)
		return;
		
	fs = g_new0(struct file_send, 1);
	fs->uin = user->Uin();
	
	// Add this to the linked list
	fs_list = g_slist_append(fs_list, fs);

	// Make the window
	fs->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	char *title = g_strdup_printf("Send file to %s", user->GetAlias());
	gtk_window_set_title(GTK_WINDOW(fs->window), title);
	g_free(title);

	// The destroy signal for the window
	g_signal_connect(G_OBJECT(fs->window), "destroy",
			   G_CALLBACK(destroy_fs), fs);

	// The box that will hold all the widgets for this window
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(fs->window), v_box);

	// Description in a frame
	GtkWidget *frame = gtk_frame_new("Description:");
	fs->description = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(fs->description), TRUE);
	gtk_widget_set_size_request(fs->description, 100, 75);
	gtk_container_add(GTK_CONTAINER(frame), fs->description);
	gtk_box_pack_start(GTK_BOX(v_box), frame, FALSE, FALSE, 0);

	// HBox with a "File:" label, file path entry box and a browse button
	GtkWidget *h_box = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), gtk_label_new("File:"), FALSE, FALSE, 0);
	fs->file_path = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(fs->file_path), 40);
	gtk_box_pack_start(GTK_BOX(h_box), fs->file_path, FALSE, FALSE, 0);
	fs->browse = gtk_button_new_with_mnemonic("  _Browse  ");
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
	h_box = hbutton_box_new();
	fs->ok = gtk_button_new_from_stock(GTK_STOCK_OK);
	fs->cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_container_add(GTK_CONTAINER(h_box), fs->ok);
	gtk_container_add(GTK_CONTAINER(h_box), fs->cancel);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);

	// Connect the signals for the browse, ok, and cancel buttons
	g_signal_connect(G_OBJECT(fs->browse), "clicked",
			   G_CALLBACK(fs_browse_click), fs);
	g_signal_connect(G_OBJECT(fs->ok), "clicked",
			   G_CALLBACK(fs_ok_click), fs);
	g_signal_connect(G_OBJECT(fs->cancel), "clicked",
			   G_CALLBACK(window_close), fs->window);
	
	// since we haven't selected any files yet...
	gtk_widget_set_sensitive(fs->ok, FALSE);
	
	// Show the window now
	gtk_container_set_border_width(GTK_CONTAINER(fs->window), 10);
	gtk_widget_show_all(fs->window);
}

// Used for the finishing event in extras.cpp
struct file_send *
fs_find(gulong uin)
{
	GSList *temp_fs_list = fs_list;

	while (temp_fs_list) {
		struct file_send *fs = (struct file_send *)temp_fs_list->data;
		if (fs->uin == uin)
			return fs;

		temp_fs_list = temp_fs_list->next;
	}

	// It wasn't found, return 0
	return 0;
}

// See if the file was accepted or not
void
file_start_send(ICQEvent *event)
{
	CExtendedAck *ea = event->ExtendedAck();
	CUserEvent *ue = event->UserEvent();
	
	if (ea == 0 || ue == 0) {
		gLog.Error("%sInternal error: file_start_send(): chat or file"
			   " request acknowledgement without extended "
			   "result.\n", L_ERRORxSTR);
		return;
	}

	if (!ea->Accepted()) {
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
	fw->input_tag = gtk_input_add_full(fw->ftman->Pipe(), GDK_INPUT_READ,
			file_pipe_callback, NULL, fw, NULL);
	fw->ftman->SetUpdatesEnabled(1);
  CEventFile *f = (CEventFile *)event->UserEvent();
  fw->ftman->SendFiles(f->FileList(), ea->Port());
	
	return;
}

void
finish_file(ICQEvent *event)
{
	struct file_send *fs = fs_find(event->Uin());

	if (fs == 0)
		return;
		
	window_close(0, fs->window);
	file_start_send(event);
}

