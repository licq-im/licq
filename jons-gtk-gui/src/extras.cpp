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

#include "pixmaps/online.xpm"
#include "pixmaps/offline.xpm"
#include "pixmaps/away.xpm"
#include "pixmaps/na.xpm"
#include "pixmaps/occ.xpm"
#include "pixmaps/dnd.xpm"
#include "pixmaps/message.xpm"

#include "licq_countrycodes.h"
#include "licq_events.h"
#include "licq_icqd.h"
#include "licq_languagecodes.h"

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

	online = make_pixmap(online, (gchar **)online_xpm);
	away = make_pixmap(away, (gchar **)away_xpm);
	na = make_pixmap(na, (gchar **)na_xpm);
	occ = make_pixmap(occ, (gchar **)occ_xpm);
	dnd = make_pixmap(dnd, (gchar **)dnd_xpm);
	offline = make_pixmap(offline, (gchar **)offline_xpm);
	message = make_pixmap(message, (gchar **)message_xpm);
}

struct status_icon *make_pixmap(struct status_icon *icon, gchar **file)
{
	if(icon == NULL)
		icon = g_new0(struct status_icon, 1);
		
	icon->pm = gdk_pixmap_create_from_xpm_d(main_window->window,
					    &icon->bm,
					    NULL,
					    file);
	return icon;
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

void owner_function(ICQEvent *event)
{
	/* For the main window, if it's a new registered users */
	const gchar *title = g_strdup_printf("%ld", gUserManager.OwnerUin());

	switch(event->Command())
	{
	case ICQ_CMDxSND_REGISTERxUSER:
		if(event->Result() == EVENT_SUCCESS)
		{
			wizard_message(5);
			main_window = main_window_new(title, 445, 200);
			main_window_show();
			system_status_refresh();
			dialog_close(NULL, register_window);
		}
		else
		{
			wizard_message(4);
		}
		break;
	}
}

void user_function(ICQEvent *event)
{
	GSList *temp = catcher;
	struct e_tag_data *etd;

	while(temp)
	{
		etd = (struct e_tag_data *)temp->data;

		if(etd->e_tag->Equals(event))
		{
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
	if( (etd->e_tag == NULL && event != NULL) ||
	    (etd->e_tag != NULL && !etd->e_tag->Equals(event)) )
	    	return;

	guint id;
	gchar temp[60];
	
	/* Get the id for the status bar */
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(etd->statusbar), "sta");
	
	/* Get the current text */
	strcpy(temp, etd->buf);
	
	if(event == NULL)
	{
		strcat(temp, "error");
	}

	else
	{
	/* Pop and then push the current text by the right event result */
		switch(event->Result())
		{
		case EVENT_ACKED:
		case EVENT_SUCCESS:
			strcat(temp, "done");
			break;
		case EVENT_FAILED:
			strcat(temp, "failed");
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

	gtk_statusbar_pop(GTK_STATUSBAR(etd->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(etd->statusbar), id, temp);
	
	/* Update the buffer for the e_tag_data */
	strcpy(etd->buf, "");
	strcpy(etd->buf, temp);

	/* Remove this item from the GSList */
	catcher = g_slist_remove(catcher, etd);

	/* Get the sub command and see if do more work if needed */
	switch(event->SubCommand())
	{
	case ICQ_CMDxSUB_MSG:
		finish_message(event);
		break;
	case ICQ_CMDxTCP_READxAWAYxMSG:
	case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
	case ICQ_CMDxTCP_READxNAxMSG:
	case ICQ_CMDxTCP_READxDNDxMSG:
	case ICQ_CMDxTCP_READxFFCxMSG:
		finish_away(event);
		break;
	}
}

/********************** Finishing Events *******************************/

void finish_message(ICQEvent *event)
{
	struct conversation *c =
		(struct conversation *)g_new0(struct conversation, 1);

	c = convo_find(event->Uin());

	/* If the window isn't open, there isn't anything left to do */
	if(c == NULL)
		return;

	/* Check to make sure it sent, and if it did, put the text in */
	g_strreverse(c->etag->buf);

	if(strncmp(c->etag->buf, "en", 2) == 0)
	{
		ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
		const gchar *name = g_strdup_printf("%s",
						owner->GetAlias());
		gUserManager.DropOwner();

		gtk_editable_delete_text(GTK_EDITABLE(c->entry), 0, -1);
		gtk_editable_delete_text(GTK_EDITABLE(c->spoof_uin), 0, -1);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->spoof_button),
					     FALSE);
		gtk_window_set_focus(GTK_WINDOW(c->window), c->entry);
		
		gtk_text_freeze(GTK_TEXT(c->entry));
		gtk_text_insert(GTK_TEXT(c->text), 0, blue, 0, name, -1);
		gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, c->for_user, -1);
		gtk_text_thaw(GTK_TEXT(c->entry));
	}
}

void finish_away(ICQEvent *event)
{
	struct user_away_window *uaw =
		(struct user_away_window *)g_new0(struct user_away_window, 1);

	uaw = uaw_find(event->Uin());

	/* If the window isn't open, don't bother */
	if(uaw == NULL)
		return;

	gtk_text_freeze(GTK_TEXT(uaw->text_box));
	gtk_text_insert(GTK_TEXT(uaw->text_box), 0, 0, 0,
			uaw->user->AutoResponse(), -1);
	gtk_text_thaw(GTK_TEXT(uaw->text_box));
}

/*************** Finishing Signals *****************************/

void finish_info(CICQSignal *signal)
{
	/* Only do the info.. */
	unsigned long type = signal->SubSignal();
	
	if(!(type == USER_GENERAL || type == USER_BASIC || type == USER_EXT ||
	   type == USER_MORE || type == USER_ABOUT))
		return;

	struct info_user *iu = (struct info_user *)g_new0(struct info_user, 1);

	iu = iu_find(signal->Uin());

	if(iu == NULL)
		return;

	const gchar *name = g_strdup_printf("%s %s", iu->user->GetFirstName(),
					    iu->user->GetLastName());
	const SCountry *country = GetCountryByCode(iu->user->GetCountryCode());
	const SLanguage *l1 = GetLanguageByCode(iu->user->GetLanguage(0));
	const SLanguage *l2 = GetLanguageByCode(iu->user->GetLanguage(1));
	const SLanguage *l3 = GetLanguageByCode(iu->user->GetLanguage(2));
	const gchar *zip = g_strdup_printf("%ld", iu->user->GetZipCode());

	gchar *bday;
	gchar *age;

	if(iu->user->GetAge() != 65535)
		age = g_strdup_printf("%hd", iu->user->GetAge());
	else
		age = "N/A";

	if(iu->user->GetBirthMonth() == 0 || iu->user->GetBirthDay() == 0)
		bday = "N/A";
	else
		bday = g_strdup_printf("%d/%d/%d", iu->user->GetBirthMonth(),
			iu->user->GetBirthDay(), iu->user->GetBirthYear());

	switch(type)
	{
	case USER_GENERAL:
	case USER_BASIC:
	case USER_EXT:
		gtk_entry_set_text(GTK_ENTRY(iu->alias), iu->user->GetAlias());
		gtk_entry_set_text(GTK_ENTRY(iu->name), name);
		gtk_entry_set_text(GTK_ENTRY(iu->email1),
				   iu->user->GetEmail1());
		gtk_entry_set_text(GTK_ENTRY(iu->email2),
				   iu->user->GetEmail2());
		gtk_entry_set_text(GTK_ENTRY(iu->address),
				   iu->user->GetAddress());
		gtk_entry_set_text(GTK_ENTRY(iu->city), iu->user->GetCity());
		gtk_entry_set_text(GTK_ENTRY(iu->state), iu->user->GetState());
		gtk_entry_set_text(GTK_ENTRY(iu->zip), zip);
		
		if(country == NULL)
			gtk_entry_set_text(GTK_ENTRY(iu->country),
					   "Unknown");
		else
			gtk_entry_set_text(GTK_ENTRY(iu->country),
					   country->szName);


		gtk_entry_set_text(GTK_ENTRY(iu->phone),
				   iu->user->GetPhoneNumber());
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
		if(l1 == NULL)
			gtk_entry_set_text(GTK_ENTRY(iu->lang1), "Unknown");
		else
			gtk_entry_set_text(GTK_ENTRY(iu->lang1), l1->szName);

		if(l2 == NULL)
                        gtk_entry_set_text(GTK_ENTRY(iu->lang2), "Unknown");
                else
                        gtk_entry_set_text(GTK_ENTRY(iu->lang2), l2->szName);

		if(l3 == NULL)
                        gtk_entry_set_text(GTK_ENTRY(iu->lang3), "Unknown");
                else
                        gtk_entry_set_text(GTK_ENTRY(iu->lang3), l3->szName);
		break;
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
		gtk_entry_set_text(GTK_ENTRY(iu->co_city),
				   iu->user->GetCompanyCity());
		gtk_entry_set_text(GTK_ENTRY(iu->co_state),
				   iu->user->GetCompanyState());
		break;
	case USER_ABOUT:
		gtk_text_freeze(GTK_TEXT(iu->about));
		gtk_editable_delete_text(GTK_EDITABLE(iu->about), 0, -1);
		gtk_text_insert(GTK_TEXT(iu->about), 0, 0, 0,
				iu->user->GetAbout(), -1);
		gtk_text_thaw(GTK_TEXT(iu->about));
		break;
	}
}
