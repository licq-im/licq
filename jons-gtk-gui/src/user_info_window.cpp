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

#include "licq_user.h"
#include "licq_icqd.h"
#include "licq_countrycodes.h"
#include "licq_languagecodes.h"

#include <gtk/gtk.h>
#include <stdlib.h>

GSList *iu_list;

void list_info_user(GtkWidget *window, ICQUser *user)
{
	gboolean is_o = FALSE;

	// Check to see if it's for the owner 
	if(user == 0)
	{
		user = gUserManager.FetchOwner(LOCK_R);
		is_o = TRUE;
	}

	struct info_user *iu = iu_find(user->Uin());

	if(iu != 0)
	{
		gdk_window_show(iu->window->window);
		gdk_window_raise(iu->window->window);
		return;
	}

	iu = iu_new(user);

	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *v_box;
	GtkWidget *v_scroll;
	GtkWidget *save;
	GtkWidget *close;
	GtkWidget *statusbar;
	const gchar *title = g_strdup_printf("Info for %s", user->GetAlias());
	const gchar *uin = g_strdup_printf("%ld", user->Uin());
	gchar real_ip[32];
	const gulong _realip = user->RealIp();
	gchar buf[32];
	strcpy(real_ip, inet_ntoa_r(*(struct in_addr *)&_realip, buf));
	
	/* Take care of the e_tag_data stuff */
	iu->etag = g_new0(struct e_tag_data, 1);

	iu->user = user;

	/* Make the window */
	iu->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(iu->window), title);
	gtk_window_set_position(GTK_WINDOW(iu->window), GTK_WIN_POS_CENTER);

	/* Make the scroll window for the about box */
	v_scroll = gtk_scrolled_window_new(0, 0);
	gtk_widget_set_usize(v_scroll, 175, 115);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(v_scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);

	/* Create new boxes */
	v_box = gtk_vbox_new(FALSE, 5);

	/* The notebook */
	iu->notebook = gtk_notebook_new();

//------ START THE GENERAL TAB ---------------------------

	GtkWidget *general_table = gtk_table_new(6, 4, false);

	// Alias (always editable)
	do_label_and_entry(iu->alias, user->GetAlias(), "Alias:",
		general_table, 0, 0, true);

	// UIN (never editable)
	do_label_and_entry(entry, uin, "UIN:", general_table, 
		2, 0, false);

	// Full Name
	do_label_and_entry(iu->fname, user->GetFirstName(), "First Name:",
		general_table, 0, 1, is_o);
	
	// Last Name
	do_label_and_entry(iu->lname, user->GetLastName(), "Last Name:",
		general_table, 2, 1, is_o);
	
	// IP (never editable)
	do_label_and_entry(entry, user->IpPortStr(buf), "IP:", 
		general_table, 0, 2, false);

	// Real IP (never editable)
	do_label_and_entry(entry, real_ip, "Real IP:", general_table,
		2, 2, false);
	
	// Primary e-mail
	do_label_and_entry(iu->email1, user->GetEmailPrimary(),
		"Primary E-Mail:", general_table, 0, 3, is_o);

	// Secondary e-mail
	do_label_and_entry(iu->email2, user->GetEmailSecondary(),
		"Secondary E-mail:", general_table, 2, 3, is_o);

	// Old e-mail
	do_label_and_entry(iu->oldemail, user->GetEmailOld(),
		"Old E-mail:", general_table, 0, 4, is_o);

	// Hide e-mail is needed anyways.. use an if statement to decide
	// where it goes..
	iu->hide_email = gtk_check_button_new_with_label(
		"Hide E-mail Address");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(iu->hide_email),
		user->GetHideEmail());
		
	// If it's not the owner, show last seen
	if(!is_o)
	{
		gchar online[28];
		
		if(!iu->user->StatusOffline())
			strcpy(online, "Now");
		else if(iu->user->LastOnline() == 0)
			strcpy(online, "Unknown");
		else
		{
			time_t last = iu->user->LastOnline();
			strcpy(online, ctime(&last));
		}
	
		do_label_and_entry(entry, online, "Last Seen:", general_table,
			2, 4, FALSE);

		iu->need_auth = gtk_check_button_new_with_label(
			"Need Authorization To Add");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
			iu->need_auth), iu->user->GetAuthorization());
		gtk_widget_set_sensitive(iu->need_auth, false);
		gtk_table_attach(GTK_TABLE(general_table), iu->need_auth, 0, 2,
			5, 6, GTK_FILL, GTK_FILL, 3, 1);

		gtk_widget_set_sensitive(iu->hide_email, false);
		gtk_table_attach(GTK_TABLE(general_table), iu->hide_email, 2, 4,
			5, 6,
			GtkAttachOptions(0),
			GtkAttachOptions(0), 3, 1);
	}
	// It's the owner
	else
	{
		gtk_table_attach(GTK_TABLE(general_table), iu->hide_email,
			0, 2, 5, 6, GTK_FILL, GTK_FILL, 3, 1);
	}

//------- END GENERAL TAB ----------------------------------

//------- START ADDRESS TAB --------------------------------

	// Address info table
	GtkWidget *address_table = gtk_table_new(4, 2, false);

	// Address
	do_label_and_entry(iu->address, user->GetAddress(), "Address:",
		address_table, 0, 0, is_o);

	// City
	do_label_and_entry(iu->city, user->GetCity(), "City:", 
		address_table, 2, 0, is_o);

	// State
	do_label_and_entry(iu->state, user->GetState(), "State:",
		address_table, 0, 1, is_o);

	// Zip
	do_label_and_entry(iu->zip, user->GetZipCode(), "Zip:",
		address_table, 2, 1, is_o);

	// Country
	SCountry *sc = (SCountry *)GetCountryByCode(user->GetCountryCode());
	gchar country[32];

	if(sc == 0)
		strcpy(country, "Unspecified");

	else
		strcpy(country, sc->szName);

	if(!is_o)
	{
		do_label_and_entry(iu->country, country, "Country:",
			address_table, 0, 2, FALSE);
	}
	else
	{
		iu->o_country = gtk_combo_new();
		GList *country_strings = 0;

		country_strings = g_list_append(country_strings,
			const_cast<char *>("Unspecified"));
		for(unsigned short j = 0; j < NUM_COUNTRIES; j++)
			country_strings = g_list_append(country_strings,
				const_cast<char *>(GetCountryByIndex(j)->szName));

		gtk_combo_set_popdown_strings(GTK_COMBO(iu->o_country),
			country_strings);
		gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(iu->o_country)->entry),
			false);
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(iu->o_country)->entry),
			country);

		label = gtk_label_new("Country:");
		gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
		
		gtk_table_attach(GTK_TABLE(address_table), label, 0, 1, 2, 3,
			GtkAttachOptions(GTK_EXPAND | GTK_FILL),
			GTK_FILL, 3, 1);
		gtk_table_attach(GTK_TABLE(address_table), iu->o_country,
			1, 2, 2, 3,
			GtkAttachOptions(GTK_EXPAND | GTK_FILL),
			GTK_FILL, 3, 1);
	}
		
	// Phone number
	do_label_and_entry(iu->phone, user->GetPhoneNumber(), "Phone Number:",
		address_table, 2, 2, is_o);

	// Cellular number
	do_label_and_entry(iu->cellphone, user->GetCellularNumber(),
		"Cellular Number:", address_table, 0, 3, is_o);

	// Fax number
	do_label_and_entry(iu->faxnumber, user->GetFaxNumber(), "Fax Number:",
		address_table, 2, 3, is_o);

//------- END ADDRESS TAB ---------------------------------------- 

//------- START MORE TAB -----------------------------------------

	GtkWidget *more_table = gtk_table_new(3, 2, false);

	// Age
	gchar age[6];
	if(user->GetAge() != 65535)
		sprintf(age, "%hd", user->GetAge());
	else
		strcpy(age, "N/A");
	
	do_label_and_entry(iu->age, age, "Age:", more_table,
		0, 0, is_o);

	// Gender
	if(!is_o)
	{
		gchar gender[12];
		if(user->GetGender() == 1)
			strcpy(gender, "Female");
		else if(user->GetGender() == 2)
			strcpy(gender, "Male");
		else
			strcpy(gender, "Unspecified");

		do_label_and_entry(iu->gender, gender, "Gender:", more_table,
			2, 0, FALSE);
	}
	else
	{
		iu->o_gender = gtk_combo_new();
		GList *gender_strings = 0;
		gender_strings = g_list_append(gender_strings, const_cast<char *>("(None)"));
		gender_strings = g_list_append(gender_strings, const_cast<char *>("Female"));
		gender_strings = g_list_append(gender_strings, const_cast<char *>("Male"));
		gtk_combo_set_popdown_strings(GTK_COMBO(iu->o_gender),
			gender_strings);
		gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(iu->o_gender)->entry),
			false);

		label = gtk_label_new("Gender:");
		gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
		
		gtk_table_attach(GTK_TABLE(more_table), label, 2, 3, 0, 1,
			GtkAttachOptions(GTK_EXPAND | GTK_FILL),
			GTK_FILL, 3, 1);
		gtk_table_attach(GTK_TABLE(more_table), iu->o_gender,
			3, 4, 0, 1,
			GtkAttachOptions(GTK_EXPAND | GTK_FILL),
			GTK_FILL, 3, 1);
	}

	// Homepage
	do_label_and_entry(iu->homepage, user->GetHomepage(), "Homepage:",
		 more_table, 0, 1, is_o);

	// Birthday
	if(!is_o)
	{
		gchar bday[11];

		if(user->GetBirthMonth() == 0 || user->GetBirthDay() == 0)
			strcpy(bday, "N/A");

		else
			sprintf(bday, "%d/%d/%d", user->GetBirthMonth(),
				user->GetBirthDay(), user->GetBirthYear());

		do_label_and_entry(iu->bday, bday, "Birthday:", more_table,
			2, 1, FALSE);
	}
	else
	{
		label = gtk_label_new("Birthday:");
		gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
		gtk_table_attach(GTK_TABLE(more_table), label, 2, 3, 1, 2,
			GtkAttachOptions(GTK_EXPAND | GTK_FILL),
			GTK_FILL, 3, 1);

		GtkWidget *h_box = gtk_hbox_new(false, 0);

		label = gtk_label_new("Month:");
		GtkAdjustment *month_adj =
			(GtkAdjustment *)gtk_adjustment_new(
				user->GetBirthMonth(), 1.0, 12.0,
				1.0, 3.0, 3.0);
		iu->o_bmonth = gtk_spin_button_new(month_adj, 1.0, 0);
		gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(iu->o_bmonth),
			GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(iu->o_bmonth),
			true);
		
		gtk_box_pack_start(GTK_BOX(h_box), label, false, false, 3);
		gtk_box_pack_start(GTK_BOX(h_box), iu->o_bmonth, false, false,
			3);

		label = gtk_label_new("Day:");
		GtkAdjustment *day_adj =
			(GtkAdjustment *)gtk_adjustment_new(
				user->GetBirthDay(), 1.0, 31.0, 1.0, 5.0, 5.0);
		iu->o_bday = gtk_spin_button_new(day_adj, 1.0, 0);
		gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(iu->o_bday),
			GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(iu->o_bday), true);

		gtk_box_pack_start(GTK_BOX(h_box), label, false, false, 3);
		gtk_box_pack_start(GTK_BOX(h_box), iu->o_bday, false, false, 3);

		label = gtk_label_new("Year:");
		GtkAdjustment *year_adj =
			(GtkAdjustment *)gtk_adjustment_new(
				user->GetBirthYear(), 1900.0,
				2020.0, 1.0, 10.0, 10.0);
		iu->o_byear = gtk_spin_button_new(year_adj, 1.0, 0);
		gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(iu->o_byear),
			GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(iu->o_byear), true);

		gtk_box_pack_start(GTK_BOX(h_box), label, false, false, 3);
		gtk_box_pack_start(GTK_BOX(h_box), iu->o_byear, false, false,
			3);

		gtk_table_attach(GTK_TABLE(more_table), h_box, 3, 4, 1, 2,
			GtkAttachOptions(GTK_EXPAND | GTK_FILL),
			GTK_FILL, 3, 1);
	}

	// Languages
	GtkWidget *h_box = gtk_hbox_new(FALSE, 5);
	
	if(!is_o)
	{
		for(unsigned short i = 0; i < 3; i++)
		{
			const SLanguage *lang =
				GetLanguageByCode(user->GetLanguage(i));
			
		  	label =
				gtk_label_new(
					g_strdup_printf("Language %d:", i + 1));
			if (lang)
			  do_entry(iu->lang[i], lang->szName, FALSE);
			else
			  do_entry(iu->lang[i], "", false);

			gtk_widget_set_usize(iu->lang[i], 75, 20);
			gtk_box_pack_start(GTK_BOX(h_box), label, FALSE,
				FALSE, 5);
			gtk_box_pack_start(GTK_BOX(h_box), iu->lang[i],
				FALSE, FALSE, 2);
		}
	}
	else
	{
		GList *lang_strings = 0;
		
		for(unsigned short j = 0; j < NUM_LANGUAGES; j++)
			lang_strings = g_list_append(lang_strings,
				const_cast<char *>(GetLanguageByIndex(j)->szName));

		for(unsigned short i = 0; i < 3; i++)
		{
			label = gtk_label_new(
				g_strdup_printf("Language %d:", i + 1));

			iu->o_lang[i] = gtk_combo_new();
			gtk_widget_set_usize(iu->o_lang[i], 100, 20);
			gtk_combo_set_popdown_strings(GTK_COMBO(iu->o_lang[i]),
				lang_strings);
			gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(
				iu->o_lang[i])->entry), false);

			// Set the language now
			const SLanguage *lang = GetLanguageByCode(
				user->GetLanguage(i));
			if(lang == 0)
				gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(
					iu->o_lang[i])->entry), "Unspecified");
			else
				gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(
					iu->o_lang[i])->entry), lang->szName);

			gtk_box_pack_start(GTK_BOX(h_box), label, false, false,
				5);
			gtk_box_pack_start(GTK_BOX(h_box), iu->o_lang[i],
				false, false, 5);
		}
	}

	gtk_table_attach(GTK_TABLE(more_table), h_box, 0, 4, 2, 3,
		GTK_FILL, GTK_FILL, 3, 1);

//------ END MORE TAB -----------------------------------

//------ START WORK TAB ---------------------------------

	GtkWidget *work_table = gtk_table_new(5, 4, false);

	// Company Name
	do_label_and_entry(iu->company, user->GetCompanyName(), "Company Name:",
		work_table, 0, 0, is_o);

	// Department
	do_label_and_entry(iu->dept, user->GetCompanyDepartment(), "Department:",
		work_table, 2, 0, is_o);

	// Position
	do_label_and_entry(iu->pos, user->GetCompanyPosition(), "Position:",
		work_table, 0, 2, is_o);

	// Homepage
	do_label_and_entry(iu->co_homepage, user->GetCompanyHomepage(),
		"Homepage:", work_table, 2, 2, is_o);

	// Address 
	do_label_and_entry(iu->co_address, user->GetCompanyAddress(),
		"Address:", work_table, 0, 3, is_o);

	// City
	do_label_and_entry(iu->co_city, user->GetCompanyCity(),
		"City:", work_table, 2, 3, is_o);

	// State
	do_label_and_entry(iu->co_state, user->GetCompanyState(), "State:",
		work_table, 0, 4, is_o);

	// Phone Number
	do_label_and_entry(iu->co_phone, user->GetCompanyPhoneNumber(),
		"Phone Number:", work_table, 0, 5, is_o);

	// Fax Number
	do_label_and_entry(iu->co_fax, user->GetCompanyFaxNumber(),
		"Fax Number:", work_table, 2, 5, is_o);

	
//------- END WORK TAB --------------------------------------

//------- START ABOUT TAB -----------------------------------

	GtkWidget *about_box = gtk_vbox_new(FALSE, 5);
	iu->about = gtk_text_new(0, 0);
	gtk_text_set_word_wrap(GTK_TEXT(iu->about), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(iu->about), TRUE);
	gtk_text_set_editable(GTK_TEXT(iu->about), is_o);
	gtk_text_freeze(GTK_TEXT(iu->about));
	gtk_text_insert(GTK_TEXT(iu->about), 0, 0, 0, user->GetAbout(), -1);
	gtk_text_thaw(GTK_TEXT(iu->about));
	gtk_container_add(GTK_CONTAINER(v_scroll), iu->about);

	/* Pack the about box */
	gtk_box_pack_start(GTK_BOX(about_box), v_scroll, FALSE, FALSE, 5);

//------ END ABOUT TAB -------------------------------------

	/* Add everything to the notebook */
	label = gtk_label_new("General");
	gtk_notebook_append_page(GTK_NOTEBOOK(iu->notebook), general_table,
		label);
	label = gtk_label_new("Address");
	gtk_notebook_append_page(GTK_NOTEBOOK(iu->notebook), address_table,
		label);
	label = gtk_label_new("More");
	gtk_notebook_append_page(GTK_NOTEBOOK(iu->notebook), more_table, label);
	label = gtk_label_new("Work");
	gtk_notebook_append_page(GTK_NOTEBOOK(iu->notebook), work_table, label);
	label = gtk_label_new("About");
	gtk_notebook_append_page(GTK_NOTEBOOK(iu->notebook), about_box, label);

	/* The buttons */
	save = gtk_button_new_with_label("Save");
	iu->update = gtk_button_new_with_label("Update");
	iu->cancel = gtk_button_new_with_label("Cancel");
	close = gtk_button_new_with_label("Close");

//	gtk_widget_set_sensitive(iu->cancel, FALSE);

	/* Add them a h_box */
	h_box = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), save, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(h_box), iu->update, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(h_box), iu->cancel, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(h_box), close, TRUE, TRUE, 10);

	/* Connect the signals of the buttons */
	gtk_signal_connect(GTK_OBJECT(save), "clicked",
			   GTK_SIGNAL_FUNC(user_info_save), iu);
	gtk_signal_connect(GTK_OBJECT(close), "clicked",
			   GTK_SIGNAL_FUNC(user_info_close), iu);
	gtk_signal_connect(GTK_OBJECT(iu->window), "destroy",
			   GTK_SIGNAL_FUNC(user_info_close), iu);
	gtk_signal_connect(GTK_OBJECT(iu->update), "clicked",
			   GTK_SIGNAL_FUNC(update_user_info), iu);
	gtk_signal_connect(GTK_OBJECT(iu->cancel), "clicked",
			   GTK_SIGNAL_FUNC(cancel_user_info), iu);

	gtk_box_pack_start(GTK_BOX(v_box), iu->notebook, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	/* The status bar */
	statusbar = gtk_statusbar_new();
	gtk_signal_connect(GTK_OBJECT(statusbar), "text-pushed",
			   GTK_SIGNAL_FUNC(verify_user_info), iu);
	gtk_box_pack_start(GTK_BOX(v_box), statusbar, FALSE, FALSE, 5);

	/* e_tag_data stuff */
	iu->etag->statusbar = statusbar;
	strcpy(iu->etag->buf, "");

	gtk_container_add(GTK_CONTAINER(iu->window), v_box);

	gtk_widget_show_all(iu->window);
}

void user_info_save(GtkWidget *widget, struct info_user *iu)
{
	ICQUser *user = gUserManager.FetchUser(iu->user->Uin(), LOCK_R);
	user->SetAlias(gtk_editable_get_chars(GTK_EDITABLE(iu->alias), 0, -1));
	gUserManager.DropUser(user);
	contact_list_refresh();
}

gboolean user_info_close(GtkWidget *widget, struct info_user *iu)
{
	/* Remove from the lists */
	iu_list = g_slist_remove(iu_list, iu);
	catcher = g_slist_remove(catcher, iu->etag);

	/* Drop the user (or owner) */
	gUserManager.DropUser(iu->user);

	/* Destroy the window */
	gtk_widget_destroy(iu->window);
	return TRUE;
}

void update_user_info(GtkWidget *widget, struct info_user *iu)
{
	/* Set the buttons accordingly */
//	gtk_widget_set_sensitive(iu->update, FALSE);
//	gtk_widget_set_sensitive(iu->cancel, TRUE);

	guint id =
	    gtk_statusbar_get_context_id(GTK_STATUSBAR(iu->etag->statusbar), "sta");
	gtk_statusbar_pop(GTK_STATUSBAR(iu->etag->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(iu->etag->statusbar), id, "Updating .. ");

	strcpy(iu->etag->buf, "");
	strcpy(iu->etag->buf, "Updating .. ");

	if(iu->user->Uin() == gUserManager.OwnerUin())
	{
		if(gtk_notebook_get_current_page(GTK_NOTEBOOK(iu->notebook))
		   == 0 || gtk_notebook_get_current_page(GTK_NOTEBOOK(iu->notebook))
		   == 1)
		{
			const gchar *_country = gtk_editable_get_chars(
				GTK_EDITABLE(GTK_COMBO(iu->o_country)->entry),
				0, -1);
			unsigned short cc;
			
			if(strcmp("Unspecified", _country) == 0)
				cc = COUNTRY_UNSPECIFIED;
			else
			{
				const SCountry *NewCountry =
					GetCountryByName(_country);
				cc = NewCountry->nCode;
			}
/*
	   		iu->etag->e_tag = icq_daemon->icqSetGeneralInfo(
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->alias), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->fname), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->lname), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->email1), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->email2), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->oldemail), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->city), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->state), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->phone), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->faxnumber), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->address), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->cellphone), 0, -1),
		  	gtk_editable_get_chars(GTK_EDITABLE(iu->zip), 0, -1),
		  	cc, 
	 	  	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(iu->hide_email)));
*/
		}

		else if(gtk_notebook_get_current_page(GTK_NOTEBOOK(iu->notebook))
			== 2)
		{
			const SLanguage *l1 = GetLanguageByName(
				gtk_editable_get_chars(GTK_EDITABLE(GTK_COMBO(
				iu->o_lang[0])->entry), 0, -1));
			unsigned short lang1 = l1->nCode;
			const SLanguage *l2 = GetLanguageByName(
				gtk_editable_get_chars(GTK_EDITABLE(GTK_COMBO(
				iu->o_lang[1])->entry), 0, -1));
			unsigned short lang2 =  l2->nCode;
			const SLanguage *l3 = GetLanguageByName(
				gtk_editable_get_chars(GTK_EDITABLE(GTK_COMBO(
				iu->o_lang[2])->entry), 0, -1));
			unsigned short lang3 =  l3->nCode;
		
			unsigned short sex = 0;
			const char *_gender = gtk_editable_get_chars(
				GTK_EDITABLE(GTK_COMBO(iu->o_gender)->entry), 0,
				-1);
			if(strcmp(_gender, "Female") == 0)
				sex = 1;
			else if (strcmp(_gender, "Male") == 0)
				sex = 2;
				
	   	  iu->etag->e_tag = icq_daemon->icqSetMoreInfo(
	   		atoi(gtk_editable_get_chars(GTK_EDITABLE(iu->age), 0, -1)),
			sex,
			gtk_editable_get_chars(GTK_EDITABLE(iu->homepage), 0, -1),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(
				iu->o_byear)),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(
				iu->o_bmonth)),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(
				iu->o_bday)),
			lang1,
			lang2,
			lang3);

			g_free(const_cast<char *>(_gender));
		}

		else if(gtk_notebook_get_current_page(GTK_NOTEBOOK(iu->notebook))
			== 3)
	   	{
/*
			iu->etag->e_tag = icq_daemon->icqSetWorkInfo(
	   			gtk_editable_get_chars(GTK_EDITABLE(iu->co_city), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->co_state), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->co_phone), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->co_fax), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->co_address), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->company), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->dept), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->pos), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->co_homepage), 0, -1));
*/
		}
		
		else if(gtk_notebook_get_current_page(GTK_NOTEBOOK(iu->notebook))
			== 4)
		{
			iu->etag->e_tag = icq_daemon->icqSetAbout(
				gtk_editable_get_chars(GTK_EDITABLE(
					iu->about), 0, -1));
		}
	}

	else
	{
		iu->etag->e_tag =
			icq_daemon->icqRequestMetaInfo(iu->user->Uin());
	}
	
	catcher = g_slist_append(catcher, iu->etag);
}

void cancel_user_info(GtkWidget *widget, struct info_user *iu)
{
	/* Take care of the buttons */
//	gtk_widget_set_sensitive(iu->update, TRUE);
//	gtk_widget_set_sensitive(iu->cancel, FALSE);

	/* Actually cancel the event */
	icq_daemon->CancelEvent(iu->etag->e_tag);

	/* Remove the etag from the catch singly linked list */
	catcher = g_slist_remove(catcher, iu->etag);
}

void verify_user_info(GtkWidget *widget, guint id, gchar *text,
		      struct info_user *iu)
{
	gchar temp[25];
	strcpy(temp, text);
	g_strreverse(temp);

	if(strncmp(temp, "en", 2) == 0)
		return;

	else
	{
//		gtk_widget_set_sensitive(iu->update, TRUE);
//		gtk_widget_set_sensitive(iu->cancel, FALSE);
	}
}

struct info_user *iu_new(ICQUser *u)
{
	struct info_user *iu;

	iu = g_new0(struct info_user, 1);

	iu->user = u;

	iu_list = g_slist_append(iu_list, iu);

	return iu;
}

struct info_user *iu_find(unsigned long uin)
{
	struct info_user *iu;
	GSList *temp_iu_list = iu_list;

	while(temp_iu_list)
	{
		iu = (struct info_user *)temp_iu_list->data;
		if(iu->user->Uin() == uin)
			return iu;

		temp_iu_list = temp_iu_list->next;
	}

	return 0;
}

void do_entry(GtkWidget *&entry, const gchar *text, gboolean is_owner)
{
	// The entry
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), text);
	gtk_entry_set_editable(GTK_ENTRY(entry), is_owner);
}

void do_label_and_entry(GtkWidget *&entry,
	      const gchar *text,
	      const gchar *lbl,
	      GtkWidget *&table,
	      gint left,
	      gint top,
	      gboolean is_owner)
{
	// The entry
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), text);
	gtk_entry_set_editable(GTK_ENTRY(entry), is_owner);

	// The label
	GtkWidget *label = gtk_label_new(lbl);
	gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);

	// Attach it to the table
	gtk_table_attach(GTK_TABLE(table), label, left, left + 1,
		top, top + 1,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GTK_FILL, 3, 1);
	gtk_table_attach(GTK_TABLE(table), entry, left + 1, left + 2,
		top, top + 1,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		 GTK_FILL, 3, 1);
}
