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
#include "utilities.h"

#include "licq_countrycodes.h"
#include "licq_languagecodes.h"

#include <gtk/gtk.h>
#include <stdlib.h>

#include <iostream>
using namespace std;

struct info_user
{
	GtkWidget *window;
	GtkWidget *notebook;
	GtkWidget *alias;
	GtkWidget *fname;
	GtkWidget *lname;
	GtkWidget *email1;
	GtkWidget *email2;
	GtkWidget *oldemail;
	GtkWidget *address;
	GtkWidget *city;
	GtkWidget *state;
	GtkWidget *zip;
	GtkWidget *country;
	GtkWidget *o_country;
	GtkWidget *phone;
	GtkWidget *cellphone;
	GtkWidget *faxnumber;
	GtkWidget *need_auth;
	GtkWidget *hide_email;
	GtkWidget *age;
	GtkWidget *gender;
	GtkWidget *o_gender;
	GtkWidget *homepage;
	GtkWidget *bday;
	GtkWidget *o_bday;
	GtkWidget *o_bmonth;
	GtkWidget *o_byear;
	GtkWidget *lang[3];
	GtkWidget *o_lang[3];
	GtkWidget *company;
	GtkWidget *dept;
	GtkWidget *pos;
	GtkWidget *co_homepage;
	GtkWidget *co_address;
	GtkWidget *co_zip;
	GtkWidget *co_country;
	GtkWidget *co_phone;
	GtkWidget *co_city;
	GtkWidget *co_state;
	GtkWidget *co_fax;
	GtkWidget *about;
	GtkWidget *update;
	GtkWidget *cancel;
  unsigned long uin;
	struct e_tag_data *etag;
};

GSList *iu_list;

void user_info_save(GtkWidget *widget, struct info_user *iu);
gboolean user_info_close(GtkWidget *widget, struct info_user *iu);
void update_user_info(GtkWidget *widget, struct info_user *iu);
void cancel_user_info(GtkWidget *widget, struct info_user *iu);
struct info_user *iu_new(unsigned long uin);
struct info_user *iu_find(unsigned long uin);
void verify_user_info(GtkWidget *widget, guint id, gchar *text,
		      struct info_user *iu);
struct info_user *iu_find(unsigned long);
void do_entry(GtkWidget *&, const gchar *, gboolean);
void do_label_and_entry(GtkWidget *&, const gchar *,
	const gchar *, GtkWidget *&, gint, gint, gboolean);

void
list_info_user(GtkWidget *window, ICQUser *user)
{
	gboolean is_o;
	if (user == 0)	{
		user = gUserManager.FetchOwner(LOCK_R);
		is_o = TRUE;
	}
  else
		is_o = FALSE;

	struct info_user *iu = iu_find(user->Uin());

	if (iu != NULL) {
		gtk_widget_show(iu->window);
		return;
	}

	iu = iu_new(user->Uin());
	/* Take care of the e_tag_data stuff */
	iu->etag = g_new0(struct e_tag_data, 1);

  iu->uin = user->Uin();

	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *v_box;
	GtkWidget *v_scroll;
	GtkWidget *save;
	GtkWidget *close;
	GtkWidget *statusbar;
	
  gchar *title = g_strdup_printf("Info for %s", user->GetAlias());
	gchar *c_uin = g_strdup_printf("%ld", user->Uin());
	gchar real_ip[32];
	const gulong _realip = user->RealIp();
	gchar buf[32];
	strcpy(real_ip, inet_ntoa_r(*(struct in_addr *)&_realip, buf));
	
	/* Make the window */
	iu->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(iu->window), title);
	gtk_window_set_position(GTK_WINDOW(iu->window), GTK_WIN_POS_CENTER);

	/* Make the scroll window for the about box */
	v_scroll = gtk_scrolled_window_new(0, 0);
	gtk_widget_set_size_request(v_scroll, 175, 115);
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
	do_label_and_entry(
      iu->alias, 
      user->GetAlias(), 
      "Alias:",
		  general_table, 
      0, 0, 
      true);

	// UIN (never editable)
	do_label_and_entry(
      entry, 
      c_uin,
      "UIN:",
      general_table, 
		  2, 0, 
      false);

	// Full Name
	do_label_and_entry(
      iu->fname, 
      user->GetFirstName(),
      "First Name:",
		  general_table, 
      0, 1, 
      is_o);
	
	// Last Name
	do_label_and_entry(
      iu->lname, 
      user->GetLastName(),
      "Last Name:",
		  general_table, 
      2, 1, 
      is_o);
	
	// IP (never editable)
  char szIPPort[64];
  char portBuf[32];
	snprintf(szIPPort, 64, "%s:%s", user->IpStr(buf), user->PortStr(portBuf));
	do_label_and_entry(
      entry, 
      szIPPort,
      "IP:", 
		  general_table, 
      0, 2, 
      false);

	// Real IP (never editable)
	do_label_and_entry(
      entry, 
      real_ip, 
      "Real IP:", 
      general_table,
		  2, 2, 
      false);
	
	// Primary e-mail
	do_label_and_entry(
      iu->email1, 
      user->GetEmailPrimary(),
		  "Primary E-Mail:",
      general_table, 
      0, 3, 
      is_o);

	// Secondary e-mail
	do_label_and_entry(
      iu->email2, 
      user->GetEmailSecondary(),
		  "Secondary E-mail:",
      general_table, 
      2, 3, 
      is_o);

	// Old e-mail
	do_label_and_entry(
      iu->oldemail, 
      user->GetEmailOld(),
		  "Old E-mail:",
      general_table, 
      0, 4, 
      is_o);

	// Hide e-mail is needed anyways.. use an if statement to decide
	// where it goes..
	iu->hide_email = gtk_check_button_new_with_label("Hide E-mail Address");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(iu->hide_email),
		  user->GetHideEmail());
		
	// If it's not the owner, show last seen
	if (!is_o)	{
		gchar online[28];
		
		if (!user->StatusOffline())
			strcpy(online, "Now");
		else if (user->LastOnline() == 0)
			strcpy(online, "Unknown");
		else {
			time_t last = user->LastOnline();
			strcpy(online, ctime(&last));
		}
	
		do_label_and_entry(
        entry,
        online,
        "Last Seen:",
        general_table,
			  2, 4, 
        FALSE);

		iu->need_auth = gtk_check_button_new_with_label(
        "Need Authorization To Add");
		gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(iu->need_auth), user->GetAuthorization());
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
	else {
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

	if (sc == 0)
		strcpy(country, "Unspecified");
	else
		strcpy(country, sc->szName);

	if (!is_o)
		do_label_and_entry(iu->country, country, "Country:",
			address_table, 0, 2, FALSE);
	else {
		iu->o_country = gtk_combo_new();
		GList *country_strings = 0;

		country_strings = g_list_append(country_strings,
			const_cast<char *>("Unspecified"));
		for(unsigned short j = 0; j < NUM_COUNTRIES; j++)
			country_strings = g_list_append(country_strings,
				const_cast<char *>(GetCountryByIndex(j)->szName));

		gtk_combo_set_popdown_strings(GTK_COMBO(iu->o_country),
			country_strings);
		//???gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(iu->o_country)->entry),
		//	false);
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
	if (!is_o) {
		const gchar *gender;
		if(user->GetGender() == 1)
			gender = "Female";
		else if(user->GetGender() == 2)
			gender = "Male";
		else
			gender = "Unspecified";

		do_label_and_entry(iu->gender, gender, "Gender:", more_table,
			2, 0, FALSE);
	}
	else {
		iu->o_gender = gtk_combo_new();
		GList *gender_strings = 0;
		gender_strings = g_list_append(gender_strings, const_cast<char *>("(None)"));
		gender_strings = g_list_append(gender_strings, const_cast<char *>("Female"));
		gender_strings = g_list_append(gender_strings, const_cast<char *>("Male"));
		gtk_combo_set_popdown_strings(GTK_COMBO(iu->o_gender),
			gender_strings);
		
		//???gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(iu->o_gender)->entry),
		//	false);
		
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
	if (!is_o) {
		gchar bday[11];

		if(user->GetBirthMonth() == 0 || user->GetBirthDay() == 0)
			strcpy(bday, "N/A");
		else
			sprintf(bday, "%d/%d/%d", user->GetBirthMonth(),
				  user->GetBirthDay(), user->GetBirthYear());

		do_label_and_entry(iu->bday, bday, "Birthday:", more_table,
			2, 1, FALSE);
	}
	else {
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
	
	if (!is_o) {
		for (unsigned short i = 0; i < 3; i++) {
			const SLanguage *lang =	GetLanguageByCode(user->GetLanguage(i));
			char *lng = g_strdup_printf("Language %d:", i + 1);
		  label =	gtk_label_new(lng);
      g_free(lng);
			if (lang)
			  do_entry(iu->lang[i], lang->szName, FALSE);
			else
			  do_entry(iu->lang[i], "", false);

			gtk_widget_set_size_request(iu->lang[i], 75, 20);
			gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
			gtk_box_pack_start(GTK_BOX(h_box), iu->lang[i],	FALSE, FALSE, 2);
		}
	}
	else {
		GList *lang_strings = 0;
		
		for(unsigned short j = 0; j < NUM_LANGUAGES; j++)
			lang_strings = g_list_append(lang_strings,
				const_cast<char *>(GetLanguageByIndex(j)->szName));

		for(unsigned short i = 0; i < 3; i++) {
			label = gtk_label_new(
				g_strdup_printf("Language %d:", i + 1));

			iu->o_lang[i] = gtk_combo_new();
			gtk_widget_set_size_request(iu->o_lang[i], 100, 20);
			gtk_combo_set_popdown_strings(GTK_COMBO(iu->o_lang[i]),
				lang_strings);
			//???gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(
			//	iu->o_lang[i])->entry), false);
			
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
	iu->about = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(iu->about), GTK_WRAP_WORD);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(iu->about), is_o);
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(iu->about));
	gtk_text_buffer_set_text(tb, user->GetAbout(), -1);
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
	save = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	iu->update = gtk_button_new_with_mnemonic("_Update");
	iu->cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	close = gtk_button_new_with_label("Close");

//	gtk_widget_set_sensitive(iu->cancel, FALSE);

	/* Add them a h_box */
	h_box = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), save, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(h_box), iu->update, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(h_box), iu->cancel, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(h_box), close, TRUE, TRUE, 10);

	/* Connect the signals of the buttons */
	g_signal_connect(G_OBJECT(save), "clicked",
			   G_CALLBACK(user_info_save), iu);
	g_signal_connect(G_OBJECT(close), "clicked",
			   G_CALLBACK(user_info_close), iu);
	g_signal_connect(G_OBJECT(iu->window), "destroy",
			   G_CALLBACK(user_info_close), iu);
	g_signal_connect(G_OBJECT(iu->update), "clicked",
			   G_CALLBACK(update_user_info), iu);
	g_signal_connect(G_OBJECT(iu->cancel), "clicked",
			   G_CALLBACK(cancel_user_info), iu);
	
	gtk_box_pack_start(GTK_BOX(v_box), iu->notebook, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	/* The status bar */
	statusbar = gtk_statusbar_new();
	g_signal_connect(G_OBJECT(statusbar), "text-pushed",
			   G_CALLBACK(verify_user_info), iu);
	gtk_box_pack_start(GTK_BOX(v_box), statusbar, FALSE, FALSE, 5);

	/* e_tag_data stuff */
	iu->etag->statusbar = statusbar;
	strcpy(iu->etag->buf, "");

	gtk_container_add(GTK_CONTAINER(iu->window), v_box);

	gtk_widget_show_all(iu->window);
  if (is_o)
    gUserManager.DropUser(user);
}

void user_info_save(GtkWidget *widget, struct info_user *iu)
{
	ICQUser *user = gUserManager.FetchUser(iu->uin, LOCK_R);
	user->SetAlias(gtk_editable_get_chars(GTK_EDITABLE(iu->alias), 0, -1));
	gUserManager.DropUser(user);
	contact_list_refresh();
}

gboolean user_info_close(GtkWidget *widget, struct info_user *iu)
{
	/* Remove from the lists */
	iu_list = g_slist_remove(iu_list, iu);
	catcher = g_slist_remove(catcher, iu->etag);

	/* Destroy the window */
	gtk_widget_destroy(iu->window);
	return TRUE;
}

void update_user_info(GtkWidget *widget, struct info_user *iu)
{
	/* Set the buttons accordingly */
//	gtk_widget_set_sensitive(iu->update, FALSE);
//	gtk_widget_set_sensitive(iu->cancel, TRUE);

	strcpy(iu->etag->buf, "Updating .. ");
	status_change(iu->etag->statusbar, "sta", iu->etag->buf);

	if (iu->uin == gUserManager.OwnerUin()) {
		if (gtk_notebook_get_current_page(GTK_NOTEBOOK(iu->notebook)) == 0 || 
        gtk_notebook_get_current_page(GTK_NOTEBOOK(iu->notebook)) == 1) {
			string country = entry_get_chars(GTK_COMBO(iu->o_country)->entry);
			unsigned short cc;
			
			if (strcmp("Unspecified", country.c_str()) == 0)
				cc = COUNTRY_UNSPECIFIED;
			else {
				const SCountry *NewCountry = GetCountryByName(country.c_str());
				cc = NewCountry->nCode;
			}

   		iu->etag->e_tag = icq_daemon->icqSetGeneralInfo(
		  	  entry_get_chars(iu->alias).c_str(),
		  	  entry_get_chars(iu->fname).c_str(),
		  	  entry_get_chars(iu->lname).c_str(),
		  	  entry_get_chars(iu->email1).c_str(),
		  	  //entry_get_chars(iu->email2).c_str(),
		  	  //entry_get_chars(iu->oldemail).c_str(),
		  	  entry_get_chars(iu->city).c_str(),
		  	  entry_get_chars(iu->state).c_str(),
		  	  entry_get_chars(iu->phone).c_str(),
		  	  entry_get_chars(iu->faxnumber).c_str(),
		  	  entry_get_chars(iu->address).c_str(),
		  	  entry_get_chars(iu->cellphone).c_str(),
		  	  entry_get_chars(iu->zip).c_str(),
		  	  cc, 
	 	  	  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(iu->hide_email)));
		}

		else if(gtk_notebook_get_current_page(GTK_NOTEBOOK(iu->notebook))	== 2)	{
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

		else if(gtk_notebook_get_current_page(GTK_NOTEBOOK(iu->notebook)) == 3)
	   	{
			const gchar *_co_country = gtk_editable_get_chars(
				GTK_EDITABLE(GTK_COMBO(iu->co_country)->entry),
				0, -1);
			unsigned short cc;

			if (strcmp("Unspecified", _co_country) == 0)
				cc = COUNTRY_UNSPECIFIED;
			else
			{
				const SCountry *pNewCountry =
					GetCountryByName(_co_country);
				cc = pNewCountry->nCode;
			}

			iu->etag->e_tag = icq_daemon->icqSetWorkInfo(
	   			gtk_editable_get_chars(GTK_EDITABLE(iu->co_city), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->co_state), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->co_phone), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->co_fax), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->co_address), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->co_zip), 0, -1),
				cc,
				gtk_editable_get_chars(GTK_EDITABLE(iu->company), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->dept), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->pos), 0, -1),
				gtk_editable_get_chars(GTK_EDITABLE(iu->co_homepage), 0, -1));
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
		iu->etag->e_tag =	icq_daemon->icqRequestMetaInfo(iu->uin);
	
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

struct info_user *iu_new(unsigned long uin)
{
	struct info_user *iu = g_new0(struct info_user, 1);

	iu->uin = uin;

	iu_list = g_slist_append(iu_list, iu);

	return iu;
}

struct info_user *iu_find(unsigned long uin)
{
	struct info_user *iu;
	GSList *temp_iu_list = iu_list;

	while (temp_iu_list) {
		iu = (struct info_user *)temp_iu_list->data;
		if (iu->uin == uin)
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

void finish_info(CICQSignal *signal)
{
	cerr << "finish_info\n";
  /* Only do the info.. */
	unsigned long type = signal->SubSignal();
  
	if(!(type == USER_GENERAL || type == USER_BASIC || type == USER_EXT ||
	   type == USER_MORE || type == USER_ABOUT))
		return;

	struct info_user *iu = iu_find(signal->Uin());
	if (iu == NULL)
		return;
    
  ICQUser *user = gUserManager.FetchUser(iu->uin, LOCK_R);

	const SCountry *country = GetCountryByCode(user->GetCountryCode());
//	const SLanguage *l1 = GetLanguageByCode(user->GetLanguage(0));
//	const SLanguage *l2 = GetLanguageByCode(user->GetLanguage(1));
//	const SLanguage *l3 = GetLanguageByCode(user->GetLanguage(2));

	gchar bday[11];
	gchar age[6];

	if(user->GetAge() != 65535)
		sprintf(age, "%hd", user->GetAge());
	else
		strcpy(age, "N/A");

	if(user->GetBirthMonth() == 0 || user->GetBirthDay() == 0)
		strcpy(bday, "N/A");
	else
		sprintf(bday, "%d/%d/%d", user->GetBirthMonth(),
			user->GetBirthDay(), user->GetBirthYear());

	switch(type)
	{
	case USER_GENERAL:
	case USER_BASIC:
	case USER_EXT:
		gtk_entry_set_text(GTK_ENTRY(iu->alias), user->GetAlias());
		gtk_entry_set_text(GTK_ENTRY(iu->fname), user->GetFirstName());
		gtk_entry_set_text(GTK_ENTRY(iu->lname), user->GetLastName());
		gtk_entry_set_text(GTK_ENTRY(iu->email1), user->GetEmailPrimary());
		gtk_entry_set_text(GTK_ENTRY(iu->email2), user->GetEmailSecondary());
		gtk_entry_set_text(GTK_ENTRY(iu->oldemail), user->GetEmailOld());
		gtk_entry_set_text(GTK_ENTRY(iu->address), user->GetAddress());
		gtk_entry_set_text(GTK_ENTRY(iu->city), user->GetCity());
		gtk_entry_set_text(GTK_ENTRY(iu->state), user->GetState());
		gtk_entry_set_text(GTK_ENTRY(iu->zip), user->GetZipCode());
		
		if(country == 0)
			gtk_entry_set_text(GTK_ENTRY(iu->country),
					   "Unspecified");
		else
			gtk_entry_set_text(GTK_ENTRY(iu->country),
					   country->szName);

		gtk_entry_set_text(GTK_ENTRY(iu->phone),
				   user->GetPhoneNumber());
		gtk_entry_set_text(GTK_ENTRY(iu->cellphone),
				   user->GetCellularNumber());
		gtk_entry_set_text(GTK_ENTRY(iu->faxnumber),
				   user->GetFaxNumber());
		break;
	case USER_MORE:
		if(user->GetGender() == 1)
			gtk_entry_set_text(GTK_ENTRY(iu->gender), "Female");
		else if(user->GetGender() == 2)
			gtk_entry_set_text(GTK_ENTRY(iu->gender), "Male");
		else 
			gtk_entry_set_text(GTK_ENTRY(iu->gender), "Unspecified");

		gtk_entry_set_text(GTK_ENTRY(iu->age), age);

		gtk_entry_set_text(GTK_ENTRY(iu->bday), bday);

		gtk_entry_set_text(GTK_ENTRY(iu->homepage),
				   user->GetHomepage());

		for(unsigned short i = 0; i < 3; i++) {
		const SLanguage *l = GetLanguageByCode(user->GetLanguage(i));
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
				   user->GetCompanyName());
		gtk_entry_set_text(GTK_ENTRY(iu->dept),
				   user->GetCompanyDepartment());
		gtk_entry_set_text(GTK_ENTRY(iu->pos),
				   user->GetCompanyPosition());
		gtk_entry_set_text(GTK_ENTRY(iu->co_homepage),
				   user->GetCompanyHomepage());
		gtk_entry_set_text(GTK_ENTRY(iu->co_address),
				   user->GetCompanyAddress());
		gtk_entry_set_text(GTK_ENTRY(iu->co_phone),
				   user->GetCompanyPhoneNumber());
		gtk_entry_set_text(GTK_ENTRY(iu->co_fax),
				   user->GetCompanyFaxNumber());
		gtk_entry_set_text(GTK_ENTRY(iu->co_city),
				   user->GetCompanyCity());
		gtk_entry_set_text(GTK_ENTRY(iu->co_state),
				   user->GetCompanyState());
		break;
	case USER_ABOUT:
		GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(iu->about));
		gtk_text_buffer_set_text(tb, user->GetAbout(), -1);
		break;
	}
  gUserManager.DropUser(user);
}
