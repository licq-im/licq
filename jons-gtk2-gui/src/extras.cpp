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

#include "pixmaps/online.xpm"
#include "pixmaps/offline.xpm"
#include "pixmaps/away.xpm"
#include "pixmaps/na.xpm"
#include "pixmaps/occ.xpm"
#include "pixmaps/dnd.xpm"
#include "pixmaps/ffc.xpm"
#include "pixmaps/invisible.xpm"
#include "pixmaps/message.xpm"
#include "pixmaps/file.xpm"
#include "pixmaps/chat.xpm"
#include "pixmaps/url.xpm"
#include "pixmaps/secure_on.xpm"
#include "pixmaps/birthday.xpm"
#include "pixmaps/securebday.xpm"
#include "pixmaps/blank.xpm"

#include "licq_countrycodes.h"
#include "licq_events.h"
#include "licq_icqd.h"
#include "licq_languagecodes.h"

#include <ctype.h>
#include <gtk/gtk.h>

#include <iostream>
using namespace std;

void do_colors()
{
  red = new GdkColor;
  blue = new GdkColor;

	red->red = 60000;
  red->green = 0;
  red->blue = 0;
  red->pixel = (gulong)(255);

  blue->red = 0;
  blue->green = 0;
	blue->blue = 60000;
  blue->pixel = (gulong)(255*256); 
}

void do_pixmaps()
{
	online = gdk_pixbuf_new_from_xpm_data((const char **)(online_xpm));
	away = gdk_pixbuf_new_from_xpm_data((const char **)(away_xpm));
	na = gdk_pixbuf_new_from_xpm_data((const char **)(na_xpm));
	occ = gdk_pixbuf_new_from_xpm_data((const char **)(occ_xpm));
	dnd = gdk_pixbuf_new_from_xpm_data((const char **)(dnd_xpm));
	offline = gdk_pixbuf_new_from_xpm_data((const char **)(offline_xpm));
	ffc = gdk_pixbuf_new_from_xpm_data((const char **)(ffc_xpm));
	invisible = gdk_pixbuf_new_from_xpm_data((const char **)(invisible_xpm));
	message_icon = gdk_pixbuf_new_from_xpm_data((const char **)(message_xpm));
	url_icon = gdk_pixbuf_new_from_xpm_data((const char **)(url_xpm));
	file_icon = gdk_pixbuf_new_from_xpm_data((const char **)(file_xpm));
	chat_icon = gdk_pixbuf_new_from_xpm_data((const char **)(chat_xpm));
	secure_icon = gdk_pixbuf_new_from_xpm_data((const char **)(secure_on_xpm));
	birthday_icon = gdk_pixbuf_new_from_xpm_data((const char **)(birthday_xpm));
	securebday_icon = gdk_pixbuf_new_from_xpm_data((const char **)(securebday_xpm));
	blank_icon = gdk_pixbuf_new_from_xpm_data((const char **)(blank_xpm));
}

/* Used to create a message box ... will be improved down the road */
void message_box(const char *message)
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *ok;

	/* Create the dialog */
	dialog = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 3);

	/* The label */
	label = gtk_label_new(message);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);

	/* The ok button */
	ok = gtk_button_new_from_stock(GTK_STOCK_OK); 
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
			  ok);

	/* Close the window on the OK button */
	g_signal_connect(G_OBJECT(ok), "clicked",
			   G_CALLBACK(dialog_close), dialog);
	
	/* Show the message box*/
	gtk_widget_show_all(dialog);
}

void verify_numbers(GtkEditable *e, gchar *text, gint len, gint *pos, 
		gpointer d)
{
	int i;
	gchar *result = g_new(gchar, len);

  for (i = 0; i < len; i++)
	{
		if(!isdigit(text[i]))
			result[i] = '\0';
		else
			result[i] = text[i];
	}

	g_signal_handlers_block_by_func(G_OBJECT(e), (gpointer)verify_numbers, d);
	gtk_editable_insert_text(GTK_EDITABLE(e), result, len, pos);

	g_signal_handlers_unblock_by_func(G_OBJECT(e), (gpointer)verify_numbers, d);

	g_signal_stop_emission_by_name(G_OBJECT(e), "insert_text");
	
	g_free(result);
}

void owner_function(ICQEvent *event)
{
	/* For the main window, if it's a new registered users */
	char *title = g_strdup_printf("%ld", gUserManager.OwnerUin());

	if (event->Command() == ICQ_CMDxSND_REGISTERxUSER) {
		if(event->Result() == EVENT_SUCCESS) {
			wizard_message(5);
			main_window = main_window_new(title);
			main_window_show();
			system_status_refresh();
			dialog_close(0, register_window);
		}
		else
			wizard_message(4);
	}
	g_free(title);
}

void user_function(ICQEvent *event)
{
	GSList *temp = catcher;
	struct e_tag_data *etd;

	while (temp) {
		etd = (struct e_tag_data *)temp->data;

		if (event->Equals(etd->e_tag)) {
			finish_event(etd, event);
			return;
		}

		/* Not this one, go on to the next */
		temp = temp->next;
	}

	return;
}

void finish_event(struct e_tag_data *etd, ICQEvent *event)
{
	/* Make sure we have the right event and event tag */
	if( (etd->e_tag == 0 && event != 0) ||
	    (etd->e_tag != 0 && !event->Equals(etd->e_tag)) )
	  return;

	guint id = 0;
	gchar temp[60];
	
	/* Get the id for the status bar, if the statusbar exists */
	if (etd->statusbar)
		id = gtk_statusbar_get_context_id(GTK_STATUSBAR(etd->statusbar),
			"sta");

	/* Get the current text */
	strcpy(temp, etd->buf);
	
	if (event == 0)
		strcat(temp, "error");
	else if (event->SubCommand() == ICQ_CMDxSUB_SECURExOPEN ||
					 event->SubCommand() == ICQ_CMDxSUB_SECURExCLOSE) {
		catcher = g_slist_remove(catcher, etd);
		finish_secure(event);
		return;
	}
	else {
	/* Pop and then push the current text by the right event result */
		switch(event->Result()) {
			case EVENT_ACKED:
			case EVENT_SUCCESS:
				strcat(temp, "done");
				break;
			case EVENT_FAILED:
				strcat(temp, "failed");
				if (event->SubCommand() == ICQ_CMDxSND_RANDOMxSEARCH)
					message_box("No random chat user found!");
				break;
			case EVENT_TIMEDOUT:
				strcat(temp, "timed out");
				break;
			case EVENT_ERROR:
				strcat(temp, "error");
				break;
			default:
				strcat(temp, "unknown");
				break;
		}
	}

	if (etd->statusbar) {
		gtk_statusbar_pop(GTK_STATUSBAR(etd->statusbar), id);
		gtk_statusbar_push(GTK_STATUSBAR(etd->statusbar), id, temp);
	}

	/* Update the buffer for the e_tag_data */
	strcpy(etd->buf, "");
	strcpy(etd->buf, temp);

	/* Remove this item from the GSList */
	catcher = g_slist_remove(catcher, etd);

	/* Get the sub command and see if do more work if needed */
	switch(event->SubCommand()) {
		case ICQ_CMDxSUB_MSG:
			finish_message(event);
			break;
		case ICQ_CMDxSUB_CHAT:
			finish_chat(event);
			break;
		case ICQ_CMDxSUB_FILE:
			finish_file(event);
			break;
		case ICQ_CMDxTCP_READxAWAYxMSG:
		case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
		case ICQ_CMDxTCP_READxNAxMSG:
		case ICQ_CMDxTCP_READxDNDxMSG:
		case ICQ_CMDxTCP_READxFFCxMSG:
			finish_away(event);
			break;
		default:
			cerr << event->SubCommand() << endl;
	} 

	// No sub command for this..
	if(event->Command() == ICQ_CMDxSND_RANDOMxSEARCH)
		finish_random(event);
}

/********************** Finishing Events *******************************/

void finish_chat(ICQEvent *event)
{
	struct request_chat *rc = g_new0(struct request_chat, 1);

	rc = rc_find(event->Uin());

	if(rc == 0)
		return;
		
	close_request_chat(rc);
	chat_start_as_client(event);
}

void finish_file(ICQEvent *event)
{
	struct file_send *fs = g_new0(struct file_send, 1);

	fs = fs_find(event->Uin());

	if(fs == 0)
		return;
		
//	close_file_send(fs);
	gtk_widget_destroy(fs->window);
	file_start_send(event);
}

void finish_away(ICQEvent *event)
{
	struct user_away_window *uaw = g_new0(struct user_away_window, 1);

	uaw = uaw_find(event->Uin());

	/* If the window isn't open, don't bother */
	if(uaw == 0)
		return;

	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(uaw->text_box));
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(tb, &iter);
	gtk_text_buffer_insert(tb, &iter, uaw->user->AutoResponse(), -1);
}

void finish_random(ICQEvent *event)
{
	// They can search again!
	gtk_widget_set_sensitive(rcw->search, true);

	// Show the person's info window
	ICQUser *u = gUserManager.FetchUser(event->SearchAck()->Uin(), LOCK_R);
	list_info_user(0, u);
	gUserManager.DropUser(u);
}

void finish_secure(ICQEvent *event)
{
	struct key_request *kr = kr_find(event->Uin());

	// Window isn't open.. cya
	if(kr == 0)
		return;

	char result[41];
	
	switch(event->Result())
	{
		case EVENT_FAILED:
			strncpy(result,
				"Remote client does not support OpenSSL.", 41);
			break;

		case EVENT_ERROR:
			strncpy(result, "Could not connect to remote client.",
				41);
			break;

		case EVENT_SUCCESS:
			if(kr->open)
				strncpy(result, "Secure channel established.",
					41);
			else
				strncpy(result, "Secure channel closed.", 41);
			break;
		case EVENT_ACKED:
		case EVENT_TIMEDOUT:
		case EVENT_CANCELLED:
		default:
			break;
	}

	gtk_label_set_text(GTK_LABEL(kr->label_status), result);
	if (event->Result() == EVENT_SUCCESS)
		gtk_timeout_add_full(500, key_request_close_window, NULL, kr, NULL);
}

			
/*************** Finishing Signals *****************************/

void finish_info(CICQSignal *signal)
{
	/* Only do the info.. */
	unsigned long type = signal->SubSignal();
	
	if(!(type == USER_GENERAL || type == USER_BASIC || type == USER_EXT ||
	   type == USER_MORE || type == USER_ABOUT))
		return;

	struct info_user *iu = g_new0(struct info_user, 1);

	iu = iu_find(signal->Uin());

	if(iu == 0)
		return;

	const SCountry *country = GetCountryByCode(iu->user->GetCountryCode());
//	const SLanguage *l1 = GetLanguageByCode(iu->user->GetLanguage(0));
//	const SLanguage *l2 = GetLanguageByCode(iu->user->GetLanguage(1));
//	const SLanguage *l3 = GetLanguageByCode(iu->user->GetLanguage(2));

	gchar bday[11];
	gchar age[6];

	if(iu->user->GetAge() != 65535)
		sprintf(age, "%hd", iu->user->GetAge());
	else
		strcpy(age, "N/A");

	if(iu->user->GetBirthMonth() == 0 || iu->user->GetBirthDay() == 0)
		strcpy(bday, "N/A");
	else
		sprintf(bday, "%d/%d/%d", iu->user->GetBirthMonth(),
			iu->user->GetBirthDay(), iu->user->GetBirthYear());

	switch(type)
	{
	case USER_GENERAL:
	case USER_BASIC:
	case USER_EXT:
		gtk_entry_set_text(GTK_ENTRY(iu->alias), iu->user->GetAlias());
		gtk_entry_set_text(GTK_ENTRY(iu->fname),
				   iu->user->GetFirstName());
		gtk_entry_set_text(GTK_ENTRY(iu->lname),
				   iu->user->GetLastName());
		gtk_entry_set_text(GTK_ENTRY(iu->email1),
				   iu->user->GetEmailPrimary());
		gtk_entry_set_text(GTK_ENTRY(iu->email2),
				   iu->user->GetEmailSecondary());
		gtk_entry_set_text(GTK_ENTRY(iu->oldemail),
				   iu->user->GetEmailOld());
		gtk_entry_set_text(GTK_ENTRY(iu->address),
				   iu->user->GetAddress());
		gtk_entry_set_text(GTK_ENTRY(iu->city), iu->user->GetCity());
		gtk_entry_set_text(GTK_ENTRY(iu->state), iu->user->GetState());
		gtk_entry_set_text(GTK_ENTRY(iu->zip), iu->user->GetZipCode());
		
		if(country == 0)
			gtk_entry_set_text(GTK_ENTRY(iu->country),
					   "Unspecified");
		else
			gtk_entry_set_text(GTK_ENTRY(iu->country),
					   country->szName);

		gtk_entry_set_text(GTK_ENTRY(iu->phone),
				   iu->user->GetPhoneNumber());
		gtk_entry_set_text(GTK_ENTRY(iu->cellphone),
				   iu->user->GetCellularNumber());
		gtk_entry_set_text(GTK_ENTRY(iu->faxnumber),
				   iu->user->GetFaxNumber());
		break;
	case USER_MORE:
		if(iu->user->GetGender() == 1)
			gtk_entry_set_text(GTK_ENTRY(iu->gender), "Female");
		else if(iu->user->GetGender() == 2)
			gtk_entry_set_text(GTK_ENTRY(iu->gender), "Male");
		else 
			gtk_entry_set_text(GTK_ENTRY(iu->gender), "Unspecified");

		gtk_entry_set_text(GTK_ENTRY(iu->age), age);

		gtk_entry_set_text(GTK_ENTRY(iu->bday), bday);

		gtk_entry_set_text(GTK_ENTRY(iu->homepage),
				   iu->user->GetHomepage());

		for(unsigned short i = 0; i < 3; i++) {
		const SLanguage *l = GetLanguageByCode(iu->user->GetLanguage(i));
		if(l == 0)
			gtk_entry_set_text(GTK_ENTRY(iu->lang[i]), "Unknown");
		else
			gtk_entry_set_text(GTK_ENTRY(iu->lang[i]), l->szName);
		}
/*
		if(l2 == 0)
                        gtk_entry_set_text(GTK_ENTRY(iu->lang2), "Unknown");
                else
                        gtk_entry_set_text(GTK_ENTRY(iu->lang2), l2->szName);

		if(l3 == 0)
                        gtk_entry_set_text(GTK_ENTRY(iu->lang3), "Unknown");
                else
                        gtk_entry_set_text(GTK_ENTRY(iu->lang3), l3->szName);
	*/	break;
	case USER_WORK:
		gtk_entry_set_text(GTK_ENTRY(iu->company),
				   iu->user->GetCompanyName());
		gtk_entry_set_text(GTK_ENTRY(iu->dept),
				   iu->user->GetCompanyDepartment());
		gtk_entry_set_text(GTK_ENTRY(iu->pos),
				   iu->user->GetCompanyPosition());
		gtk_entry_set_text(GTK_ENTRY(iu->co_homepage),
				   iu->user->GetCompanyHomepage());
		gtk_entry_set_text(GTK_ENTRY(iu->co_address),
				   iu->user->GetCompanyAddress());
		gtk_entry_set_text(GTK_ENTRY(iu->co_phone),
				   iu->user->GetCompanyPhoneNumber());
		gtk_entry_set_text(GTK_ENTRY(iu->co_fax),
				   iu->user->GetCompanyFaxNumber());
		gtk_entry_set_text(GTK_ENTRY(iu->co_city),
				   iu->user->GetCompanyCity());
		gtk_entry_set_text(GTK_ENTRY(iu->co_state),
				   iu->user->GetCompanyState());
		break;
	case USER_ABOUT:
		GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(iu->about));
		gtk_text_buffer_set_text(tb, iu->user->GetAbout(), -1);
		break;
	}
}
