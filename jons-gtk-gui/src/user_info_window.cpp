#include "licq_gtk.h"

#include "licq_user.h"
#include "licq_icqd.h"
#include "licq_countrycodes.h"
#include "licq_languagecodes.h"

#include <gtk/gtk.h>

GSList *iu_list;

void list_info_user(GtkWidget *window, ICQUser *user)
{
	struct info_user *iu = iu_find(user->Uin());

	if(iu != NULL)
		return;

	iu = iu_new(user);

	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *h_box;
	GtkWidget *v_box;
	GtkWidget *v_scroll;
	GtkWidget *general_box;
	GtkWidget *address_box;
	GtkWidget *more_box;
	GtkWidget *work_box;
	GtkWidget *about_box;
	GtkWidget *update;
	GtkWidget *close;
	GtkWidget *notebook;
	GtkWidget *statusbar;
	const gchar *title = g_strdup_printf("Info for %s", user->GetAlias());
	const gchar *name = g_strdup_printf("%s %s", user->GetFirstName(),
					    user->GetLastName());
	const gchar *uin = g_strdup_printf("%ld", user->Uin());
	gchar buf[32];
	
	/* Take care of the e_tag_data stuff */
	iu->etag = (struct e_tag_data *)g_new0(struct e_tag_data, 1);

	iu->user = user;

	/* Make the window */
	iu->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(iu->window), title);
	gtk_window_set_position(GTK_WINDOW(iu->window), GTK_WIN_POS_CENTER);

	/* Make the scroll window for the about box */
	v_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_usize(v_scroll, 250, 135);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(v_scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);

	/* Create new boxes */
	h_box = gtk_hbox_new(FALSE, 5);
	v_box = gtk_vbox_new(FALSE, 5);
	general_box = gtk_vbox_new(FALSE, 5);
	address_box = gtk_vbox_new(FALSE, 5);
	more_box = gtk_vbox_new(FALSE, 5);
	work_box = gtk_vbox_new(FALSE, 5);
	about_box = gtk_vbox_new(FALSE, 5);

	/* The notebook */
	notebook = gtk_notebook_new();

	/* START THE GENERAL TAB */

	/* The Alias entry and label */
	do_entry(iu->alias, label, "Nick:      ", user->GetAlias());
	gtk_entry_set_editable(GTK_ENTRY(iu->alias), TRUE);

	/* Pack the alias */
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), iu->alias, FALSE, FALSE, 0);

	/* The Name box and label */
	do_entry(iu->name, label, "Name:     ", name);

	/* Pack the name */
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), iu->name, FALSE, FALSE, 0);

	/* Pack the Alias and Name E&L's in a horizontal line */
	gtk_box_pack_start(GTK_BOX(general_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* The UIN box and label */
	do_entry(entry, label, "UIN:       ", uin);

	/* Pack the UIN */
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), entry , FALSE, FALSE, 0); 

	/* The IP box and label */
	do_entry(entry, label, "IP:          ", user->IpPortStr(buf));

	/* Pack the IP */
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), entry, FALSE, FALSE, 0);

	/* Pack the UIN and IP in a horizontal line */
	gtk_box_pack_start(GTK_BOX(general_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* The primary e-mails */
	do_entry(iu->email1, label, "E-mail 1:", user->GetEmail1());

	/* Pack the email1 */
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), iu->email1, FALSE, FALSE, 0);

	/* The secondary e-mail */
	do_entry(iu->email2, label, "E-mail 2:", user->GetEmail2());

	/* Pack the email2 */
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), iu->email2, FALSE, FALSE, 0);

	/* Pack e-mail1 and e-mail2 in a horizontal line */
	gtk_box_pack_start(GTK_BOX(general_box), h_box, FALSE, FALSE, 5); 

	/* END GENERAL TAB */

	h_box = gtk_hbox_new(FALSE, 5);

	/* START ADDRESS TAB */

	/* The address and pack it */
	do_entry(iu->address, label, "Address:", user->GetAddress());
	pack_hbox(h_box, label, iu->address);

	/* The city and pack it */
	do_entry(iu->city, label, "City:   ", user->GetCity());
	pack_hbox(h_box, label, iu->city);

	/* Finally pack the address and city */
	gtk_box_pack_start(GTK_BOX(address_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* The state and pack it */
	do_entry(iu->state, label, "State:    ", user->GetState());
	pack_hbox(h_box, label, iu->state);

	/* The zip and pack it */
	const gchar *zip = g_strdup_printf("%ld", user->GetZipCode());
	do_entry(iu->zip, label, "Zip:    ", zip);
	pack_hbox(h_box, label, iu->zip);

	/* Finally pack the state and zip */
	gtk_box_pack_start(GTK_BOX(address_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/*  The country and pack it*/
	SCountry *sc = GetCountryByCode(user->GetCountryCode());
	gchar *country;

	if(sc == NULL)
		country = "Unknown";

	else
		country = g_strdup_printf("%s", sc->szName);
	
	do_entry(iu->country, label, "Country:", country);
	pack_hbox(h_box, label, iu->country);
	
	/* The Phone Number and pack it */
	do_entry(iu->phone, label, "Phone:", user->GetPhoneNumber());
	pack_hbox(h_box, label, iu->phone);

	/* Finally pack the country and phone number */
	gtk_box_pack_start(GTK_BOX(address_box), h_box, FALSE, FALSE, 5);

	/* END ADDRESS TAB */

	h_box = gtk_hbox_new(FALSE, 5);

	/* START MORE TAB */

	/* The Age box and pack it */
	gchar *age;
	if(user->GetAge() != 65535)
		age = g_strdup_printf("%hd", user->GetAge());
	else
		age = "N/A";

	do_entry(iu->age, label, "Age:          ", age);
	pack_hbox(h_box, label, iu->age);

	/* The gender box */
	gchar *gender;
	if(user->GetGender() == 1)
		gender = "Female";
	else if(user->GetGender() == 2)
		gender = "Male";
	else
		gender = "Unspecified";

	do_entry(iu->gender, label, "Gender:  ", gender);
	pack_hbox(h_box, label, iu->gender);

	/* Put age and gender on a horizontal line */
	gtk_box_pack_start(GTK_BOX(more_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* The Homepage and pack it */
	do_entry(iu->homepage, label, "Homepage: ", user->GetHomepage());
	pack_hbox(h_box, label, iu->homepage);


	/* The birthday and pack it */
	gchar *bday;

	if(user->GetBirthMonth() == 0 || user->GetBirthDay() == 0)
		bday = "N/A";

	else
		bday = g_strdup_printf("%d/%d/%d", user->GetBirthMonth(),
				user->GetBirthDay(), user->GetBirthYear());

	do_entry(iu->bday, label, "Birthday: ", bday);
	pack_hbox(h_box, label, iu->bday);

	/* Put the homepate and birthday on a horz line */
	gtk_box_pack_start(GTK_BOX(more_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* The languages and pack it */
	const SLanguage *lang1 = GetLanguageByCode(user->GetLanguage(0));
	do_entry(iu->lang1, label, "Language 1:", lang1->szName);
	gtk_widget_set_usize(iu->lang1, 75, 20);
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), iu->lang1, FALSE, FALSE, 2);

	/* Second language and pack it*/
	const SLanguage *lang2 = GetLanguageByCode(user->GetLanguage(1));
	do_entry(iu->lang2, label, "Language 2:", lang2->szName);
	gtk_widget_set_usize(iu->lang2, 75, 20);
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(h_box), iu->lang2, FALSE, FALSE, 2);

	/* Third language and pack it */
	const SLanguage *lang3 = GetLanguageByCode(user->GetLanguage(2));
	do_entry(iu->lang3, label, "Language 3:", lang3->szName);
	gtk_widget_set_usize(iu->lang3, 75, 20);
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(h_box), iu->lang3, FALSE, FALSE, 2);

	/* Pack the the languages */
	gtk_box_pack_start(GTK_BOX(more_box), h_box, FALSE, FALSE, 5);

	/* END MORE TAB */

	h_box = gtk_hbox_new(FALSE, 5);

	/* START WORK TAB */

	/* Company Name and pack it */
	do_entry(iu->company, label, "Name:    ", user->GetCompanyName());
	pack_hbox(h_box, label, iu->company);

	/* Department and pack it */
	do_entry(iu->dept, label, "Department:", user->GetCompanyDepartment());
	pack_hbox(h_box, label, iu->dept);

	/* Company Name and Department in a horz line */
	gtk_box_pack_start(GTK_BOX(work_box), h_box, FALSE, FALSE, 5);	

	h_box = gtk_hbox_new(FALSE, 5);

	/* Position and pack it */
	do_entry(iu->pos, label, "Position: ", user->GetCompanyPosition());
	pack_hbox(h_box, label, iu->pos);

	/* Company Homepage and pack it */
	do_entry(iu->co_homepage, label, "Homepage: ",
		 user->GetCompanyHomepage());
	pack_hbox(h_box, label, iu->co_homepage);

	/* Pack Position and Homepage in a horz line */
	gtk_box_pack_start(GTK_BOX(work_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* Company Address */
	do_entry(iu->co_address, label, "Address:", user->GetCompanyAddress());
	pack_hbox(h_box, label, iu->co_address);

	/* Company Phone Number */
	do_entry(iu->co_phone, label, "Phone Num:",
		 user->GetCompanyPhoneNumber());
	pack_hbox(h_box, label, iu->co_phone);

	gtk_box_pack_start(GTK_BOX(work_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* Company City and pack it */
	do_entry(iu->co_city, label, "City:      ", user->GetCompanyCity());
	pack_hbox(h_box, label, iu->co_city);

	/* Company State and pack it */
	do_entry(iu->co_state, label, "State:         ",
		 user->GetCompanyState());
	pack_hbox(h_box, label, iu->co_state);

	gtk_box_pack_start(GTK_BOX(work_box), h_box, FALSE, FALSE, 5);

	/* END WORK TAB */

	/* START ABOUT TAB */

	iu->about = gtk_text_new(NULL, NULL);
	gtk_text_set_word_wrap(GTK_TEXT(iu->about), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(iu->about), TRUE);
	gtk_text_freeze(GTK_TEXT(iu->about));
	gtk_text_insert(GTK_TEXT(iu->about), 0, 0, 0, user->GetAbout(), -1);
	gtk_text_thaw(GTK_TEXT(iu->about));
	gtk_container_add(GTK_CONTAINER(v_scroll), iu->about);

	/* Pack the about box */
	gtk_box_pack_start(GTK_BOX(about_box), v_scroll, FALSE, FALSE, 5);

	/* END ABOUT TAB */

	/* Add everything to the notebook */
	label = gtk_label_new("General");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), general_box, label);
	label = gtk_label_new("Address");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), address_box, label);
	label = gtk_label_new("More");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), more_box, label);
	label = gtk_label_new("Work");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), work_box, label);
	label = gtk_label_new("About");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), about_box, label);

	/* The buttons */
	update = gtk_button_new_with_label("Update");
	close = gtk_button_new_with_label("Close");

	/* Add them a h_box */
	h_box = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), update, TRUE, TRUE, 35);
	gtk_box_pack_start(GTK_BOX(h_box), close, TRUE, TRUE, 35);

	/* Connect the signals of the buttons */
	gtk_signal_connect(GTK_OBJECT(close), "clicked",
			   GTK_SIGNAL_FUNC(user_info_close), iu);
	gtk_signal_connect(GTK_OBJECT(iu->window), "destroy",
			   GTK_SIGNAL_FUNC(user_info_close), iu);
	gtk_signal_connect(GTK_OBJECT(update), "clicked",
			   GTK_SIGNAL_FUNC(update_user_info), iu);

	gtk_box_pack_start(GTK_BOX(v_box), notebook, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	/* The status bar */
	statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(v_box), statusbar, FALSE, FALSE, 5);

	/* e_tag_data stuff */
	iu->etag->statusbar = statusbar;
	strcpy(iu->etag->buf, "");

	gtk_container_add(GTK_CONTAINER(iu->window), v_box);

	gtk_widget_show_all(iu->window);
}

gboolean user_info_close(GtkWidget *widget, struct info_user *iu)
{
	gtk_widget_destroy(iu->window);
	iu_list = g_slist_remove(iu_list, iu);
	catcher = g_slist_remove(catcher, iu->etag);
	return TRUE;
}

void update_user_info(GtkWidget *widget, struct info_user *iu)
{
	guint id =
	    gtk_statusbar_get_context_id(GTK_STATUSBAR(iu->etag->statusbar), "sta");
	gtk_statusbar_pop(GTK_STATUSBAR(iu->etag->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(iu->etag->statusbar), id, "Updating .. ");

	strcpy(iu->etag->buf, "");
	strcpy(iu->etag->buf, "Updating .. ");

	iu->etag->e_tag = icq_daemon->icqRequestMetaInfo(iu->user->Uin());
	catcher = g_slist_append(catcher, iu->etag);
}

struct info_user *iu_new(ICQUser *u)
{
	struct info_user *iu;

	iu = (struct info_user *)g_new0(struct info_user, 1);

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

	return NULL;
}

void do_entry(GtkWidget *&entry,
	      GtkWidget *&label,
              const gchar *lbl,
	      const gchar *text)
{
	label = gtk_label_new(lbl);
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), text);
	gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
}

void pack_hbox(GtkWidget *&h_box, GtkWidget *label, GtkWidget *entry)
{
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), entry, FALSE, FALSE, 5);
}
