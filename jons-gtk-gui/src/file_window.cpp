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

#include "licq_gtk.h"

#include "licq_icqd.h"
#include "licq_packets.h"
#include "licq_message.h"
#include "licq_user.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define STATE_RECVxHANDSHAKE		1
#define STATE_RECVxCLIENTxINIT		2
#define STATE_RECVxFILExINFO		3
#define	STATE_RECVxFILE			4
#define STATE_RECVxSERVERxINIT		5
#define STATE_RECVxSTART		6
#define STATE_SENDxFILE			7

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
	fa->e = (CEventFile *)e;
	g_print("Sequence: %lu\n", fa->e->Sequence());

	/* Make the window */
	fa->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(fa->window), title);
	gtk_window_set_position(GTK_WINDOW(fa->window), GTK_WIN_POS_CENTER);

	/* Make the boxes */
	v_box = gtk_vbox_new(FALSE, 5);
	h_box = gtk_hbox_new(FALSE, 5);

	/* The label */
	const gchar *text = g_strdup_printf("File: %s (%ld bytes)",
					    ((CEventFile *)e)->Filename(),
					    ((CEventFile *)e)->FileSize());
	label = gtk_label_new(text);

	/* Pack the label */
	gtk_box_pack_start(GTK_BOX(v_box), label, FALSE, FALSE, 10);

	/* The buttons */
	accept = gtk_button_new_with_label("Accept");
	refuse = gtk_button_new_with_label("Refuse");

	/* Pack them */
	gtk_box_pack_start(GTK_BOX(h_box), accept, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), refuse, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 10);

	/* Connect the signals */
	gtk_signal_connect(GTK_OBJECT(fa->window), "destroy",
			   GTK_SIGNAL_FUNC(dialog_close), fa->window);
	gtk_signal_connect(GTK_OBJECT(refuse), "clicked",
			   GTK_SIGNAL_FUNC(refuse_file), fa);
	gtk_signal_connect(GTK_OBJECT(accept), "clicked",
			   GTK_SIGNAL_FUNC(accept_file), fa);

	gtk_container_add(GTK_CONTAINER(fa->window), v_box);
	gtk_widget_show_all(fa->window);
}

void accept_file(GtkWidget *widget, struct file_accept *fa)
{
	/* Close the unnecesarry open window */
	dialog_close(NULL, fa->window);

	/* Get the place to save it as */
	fa->file_selection = gtk_file_selection_new("Licq - Save File");

	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fa->file_selection)->ok_button),
			   "clicked", GTK_SIGNAL_FUNC(save_file), fa);

	/* Make sure that the box is closed when the user selects a button */
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fa->file_selection)->ok_button),
			   "clicked", GTK_SIGNAL_FUNC(dialog_close),
			   (gpointer)fa->file_selection);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fa->file_selection)->cancel_button),
			   "clicked", GTK_SIGNAL_FUNC(dialog_close),
			   (gpointer)fa->file_selection);

	gtk_widget_show(fa->file_selection);
}

void save_file(GtkWidget *widget, struct file_accept *fa)
{
	gchar *file_name = g_strdup_printf("%s",
				 gtk_file_selection_get_filename(
                      		 GTK_FILE_SELECTION(fa->file_selection)));
	int flags = O_WRONLY;
	struct stat buffer;
	struct file_window *fw;

#if 0
	/* Check to see if the file exists already */
	if( stat(file_name, &buffer) == 0)
	{
		/* If the incoming file is <= than the current file, truncate?*/
		if((unsigned long)buffer.st_size >= fa->e->FileSize())
		{
			flags |= O_TRUNC;
		}

		else
		{
			flags |= O_APPEND;
		}
	}
	
	/* Create it, it doesn't exist */
	else
		flags |= O_CREAT;
#endif
	/* Temporary for my purpose only...  MAKE SURE THE FILE DOES NOT EXIST
	 * as I'm doing this to TEST only.. */
	flags |= O_CREAT;

	int file_desc = open(file_name, flags, 00664); 

	if(file_desc < 0)
		g_print("Error opening file\n");

	fw = g_new0(struct file_window, 1);
	fw->uin = fa->user->Uin();
	fw->sequence = fa->e->Sequence();
	fw->file_name = file_name,
	fw->flags = flags;
	fw->file_desc = file_desc;
	
	file_start_as_server(fw);	
}

void file_start_as_server(struct file_window *fw)
{
	if(icq_daemon->StartTCPServer(&fw->socket_file_server) == -1)
	{
		g_print("No more ports available\n");
		return;
	}
	
//	fw->socket_file_server = temp;
//	fw->port = temp.LocalPort();
	fw->port = fw->socket_file_server.LocalPort();
	
	fw->input_tag = gdk_input_add(fw->socket_file_server.Descriptor(),
				      GDK_INPUT_READ,
		      		      file_recv_connection,
				      (gpointer)fw);

	sleep(1);

	icq_daemon->icqFileTransferAccept(fw->uin, fw->port, fw->sequence);
}

void file_recv_connection(gpointer g_fw, gint descriptor,
			  GdkInputCondition condition)
{	g_print("file_recv_connection\n");
	
	struct file_window *fw = (struct file_window *)g_fw;
	fw->socket_file_server.RecvConnection(fw->socket_file);

	/* We don't need this anymore */
	gdk_input_remove(fw->input_tag);

	fw->state = STATE_RECVxHANDSHAKE;
	fw->input_tag = gdk_input_add(fw->socket_file.Descriptor(),
				      GDK_INPUT_READ,
				      file_state_server,
				      (gpointer)fw);
}

void file_state_server(gpointer g_fw, gint descriptor,
		       GdkInputCondition condition)
{	g_print("file_state_server\n");
	gchar buf[128];
	struct file_window *fw = (struct file_window *)g_fw;
	fw->file_pos = 0;	
	/* Get the handshake packet */
	if(!fw->socket_file.RecvPacket())
	{
		//file_cancel(NULL, fw);
		if(fw->socket_file.Error() == 0)
			g_print("Remove user disconnected.\n");
		else
			gLog.Error("%sFile transfer receive error - lost remoe end\n%s%s\n",
			L_ERRORxSTR, L_BLANKxSTR, fw->socket_file.ErrorStr(buf, 128));
		return;
	}

	if(!fw->socket_file.RecvBufferFull())
		return;

	switch(fw->state)
	{
	case STATE_RECVxHANDSHAKE:
	{
			guint8 handshake;
			fw->socket_file.RecvBuffer() >> handshake;
			if(handshake != ICQ_CMDxTCP_HANDSHAKE)
			{
				gLog.Error("%sReceive error - bad handshake (%04X).\n",
				   L_ERRORxSTR, handshake);
			
				//file_cancel(NULL, fw);
				return;
			}
			fw->state = STATE_RECVxCLIENTxINIT;
			break;
	}
	case STATE_RECVxCLIENTxINIT:
	{
			/* Process the init packet */
			char junk;
			gulong junk_long;
			guint16 remote_name_len;
			fw->socket_file.RecvBuffer() >> junk;
			if(junk != 0x00)
			{
				char *buf;
				gLog.Error("%sError receiving data: invalid client init packet:\n%s%s\n",
				L_ERRORxSTR, L_BLANKxSTR,
				fw->socket_file.RecvBuffer().print(buf));
				delete [] buf;
				//file_cancel(NULL, fw);
				return;
			}

			fw->socket_file.RecvBuffer() >> junk_long
						     >> fw->total_files
						     >> fw->batch_size
						     >> junk_long
						     >> remote_name_len;
			fw->remote_name = new gchar[remote_name_len];

			for(int i = 0; i < remote_name_len; i++)
				fw->socket_file.RecvBuffer() >> fw->remote_name[i];
			
			/* Send a response */
			CPFile_InitServer p(fw->local_name);
			fw->socket_file.SendPacket(p.getBuffer());

			fw->state = STATE_RECVxFILExINFO;
			break;
	}
	case STATE_RECVxFILExINFO:
	{
			/* Process file packet */
			guint16 len;
			gchar junk;
			fw->socket_file.RecvBuffer() >> junk;
			if(junk == 0x05)
			{
				/* Setting speed not implemented */
				break;
			}
			if(junk != 0x02)
			{
				gchar *buf;
				gLog.Error("%sError receiving data: invalid file info packet:\n%s%s\n",
				L_ERRORxSTR, L_BLANKxSTR,
				fw->socket_file.RecvBuffer().print(buf));
				delete [] buf;
				//file_cancel(NULL, fw);
				return;
			}
			fw->socket_file.RecvBuffer() >> junk >> len;
			for(int j = 0; j < len; j++)
			{	
				fw->socket_file.RecvBuffer() >>
							fw->remote_file_name[j];
			}
			fw->socket_file.RecvBuffer() >> len;
			fw->socket_file.RecvBuffer() >> junk;
			fw->socket_file.RecvBuffer() >> fw->remote_file_size;

			/* Send response THIS WILL CREATE A NEW FILE ONLY */
			CPFile_Start p(0);
			fw->socket_file.SendPacket(p.getBuffer());

			/* Don't need this anymore */
			gdk_input_remove(fw->input_tag);

			fw->input_tag = gdk_input_add(fw->socket_file.Descriptor(),
						      GDK_INPUT_READ,
						      file_recv_file,
						      (gpointer)fw);
			fw->state = STATE_RECVxFILE;
			break;
	}
	}

	fw->socket_file.ClearRecvBuffer();
}

void file_recv_file(gpointer g_fw, gint descriptor,
		    GdkInputCondition condition)
{	g_print("file_recv_file\n");
	gchar buf[128];

	struct file_window *fw = (struct file_window *)g_fw;	
	if(!fw->socket_file.RecvPacket())
	{
		//file_cancel(NULL, fw);
		if(fw->socket_file.Error() == 0)
			gLog.Error("%sFile receive error, remote end disconnected.\n",
			L_ERRORxSTR);
		else
			gLog.Error("%sFile receive error:\n%s%s\n", L_ERRORxSTR,
				   L_BLANKxSTR,fw->socket_file.ErrorStr(buf, 128));
		return;
	}

	/* If this is the first call to this function */
	if(fw->file_pos == 0)
	{
		fw->start_time = time(NULL);
//		fw->batch_pos += fw->file_pos;
	}

	/* Write the new data to the file and empty the buffer */
	CBuffer &b = fw->socket_file.RecvBuffer();
	gchar test;
	b >> test;
	if(test != 0x06)
	{
		if(test != 0x05)
		{
			gLog.Unknown("%sFile receive error, invalid data(%c).  Ignoring packet.\n",
			L_UNKNOWNxSTR, test);
		}
		fw->socket_file.ClearRecvBuffer();
		return;
	}
	
	errno = 0;
	size_t bytes_written = write(fw->file_desc, b.getDataPosRead(),
				     b.getDataSize() - 1);

	if(bytes_written != b.getDataSize() - 1)
	{
		gLog.Error("%sFile write error:\n%s%s.\n", L_ERRORxSTR,
			   L_BLANKxSTR,
			   errno == 0 ? "Disk full (?)" : strerror(errno));
		//file_cancel(NULL, fw);
		return;
	}

	fw->file_pos += bytes_written;

	fw->socket_file.ClearRecvBuffer();

	gint bytes_left = fw->remote_file_size - fw->file_pos;
	if(bytes_left > 0)
	{
		/* More data to receive */
		return;
	}


}

void refuse_file(GtkWidget *widget, struct file_accept *fa)
{
	/* Let's get rid of the file accept window */
	dialog_close(NULL, fa->window);

	/*
	 * Make a new window to refuse the file.  We have the file_accept 
         * structure with everything we need.  So don't make a new structure
	 * for this part. 
	 */

	GtkWidget *v_box;
	GtkWidget *scroll;
	GtkWidget *ok;
	const gchar *title =
		g_strdup_printf("File refusal to %s", fa->user->GetAlias());

	/* Make the window */
	fa->window2 = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(fa->window2), title);
	gtk_window_set_position(GTK_WINDOW(fa->window2), GTK_WIN_POS_CENTER);

	/* The box */
	v_box = gtk_vbox_new(FALSE, 0);

	/* The text box in the scrolled window */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);

	fa->textbox = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(fa->textbox), TRUE);
	gtk_text_set_word_wrap(GTK_TEXT(fa->textbox), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(fa->textbox), TRUE);

	gtk_container_add(GTK_CONTAINER(scroll), fa->textbox);
	gtk_widget_set_usize(scroll, 150, 150);

	/* Pack it */
	gtk_box_pack_start(GTK_BOX(v_box), scroll, FALSE, FALSE, 5);

	/* The button */
	ok = gtk_button_new_with_label("OK");

	/* Pack it */
	gtk_box_pack_start(GTK_BOX(v_box), ok, TRUE, TRUE, 5);

	/* Connect the signals */
	gtk_signal_connect(GTK_OBJECT(fa->window2), "destroy",
			   GTK_SIGNAL_FUNC(dialog_close), fa->window2);
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(refuse_ok), fa);


	/* Show the window now */
	gtk_container_add(GTK_CONTAINER(fa->window2), v_box);
	gtk_widget_show_all(fa->window2);
}

void refuse_ok(GtkWidget *widget, struct file_accept *fa)
{
	icq_daemon->icqFileTransferRefuse(fa->user->Uin(),
		(const char *)gtk_editable_get_chars(GTK_EDITABLE(fa->textbox), 0, -1),
		fa->e->Sequence());

	dialog_close(NULL, fa->window2);
}
