#include "licq_gtk.h"

#include "user.h"
#include "icqd.h"
#include "countrycodes.h"
#include "languagecodes.h"

#include <gtk/gtk.h>

void list_info_user(GtkWidget *window, ICQUser *user)
{
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *text;
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
	struct info_user *iu = g_new0(struct info_user, 1);
	const gchar *title = g_strdup_printf("Info for %s", user->GetAlias());
	const gchar *name = g_strdup_printf("%s %s", user->GetFirstName(),
					    user->GetLastName());
	const gchar *uin = g_strdup_printf("%ld", user->Uin());
	gchar buf[32];

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
	do_entry(entry, label, "Nick:      ", user->GetAlias());
	gtk_entry_set_editable(GTK_ENTRY(entry), TRUE);

	/* Pack the alias */
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), entry, FALSE, FALSE, 0);

	/* The Name box and label */
	do_entry(entry, label, "Name:     ", name);

	/* Pack the name */
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), entry, FALSE, FALSE, 0);

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
	do_entry(entry, label, "E-mail 1:", user->GetEmail1());

	/* Pack the email1 */
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), entry, FALSE, FALSE, 0);

	/* The secondary e-mail */
	do_entry(entry, label, "E-mail 2:", user->GetEmail2());

	/* Pack the email2 */
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), entry, FALSE, FALSE, 0);

	/* Pack e-mail1 and e-mail2 in a horizontal line */
	gtk_box_pack_start(GTK_BOX(general_box), h_box, FALSE, FALSE, 5); 

	/* END GENERAL TAB */

	h_box = gtk_hbox_new(FALSE, 5);

	/* START ADDRESS TAB */

	/* The address and pack it */
	do_entry(entry, label, "Address:", user->GetAddress());
	pack_hbox(h_box, label, entry);

	/* The city and pack it */
	do_entry(entry, label, "City:   ", user->GetCity());
	pack_hbox(h_box, label, entry);

	/* Finally pack the address and city */
	gtk_box_pack_start(GTK_BOX(address_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* The state and pack it */
	do_entry(entry, label, "State:    ", user->GetState());
	pack_hbox(h_box, label, entry);

	/* The zip and pack it */
	const gchar *zip = g_strdup_printf("%ld", user->GetZipCode());
	do_entry(entry, label, "Zip:    ", zip);
	pack_hbox(h_box, label, entry);

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
	
	do_entry(entry, label, "Country:", country);
	pack_hbox(h_box, label, entry);
	
	/* The Phone Number and pack it */
	do_entry(entry, label, "Phone:", user->GetPhoneNumber());
	pack_hbox(h_box, label, entry);

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

	do_entry(entry, label, "Age:          ", age);
	pack_hbox(h_box, label, entry);

	/* The gender box */
	gchar *gender;
	if(user->GetGender() == 1)
		gender = "Female";
	else if(user->GetGender() == 2)
		gender = "Male";
	else
		gender = "Unspecified";

	do_entry(entry, label, "Gender:  ", gender);
	pack_hbox(h_box, label, entry);

	/* Put age and gender on a horizontal line */
	gtk_box_pack_start(GTK_BOX(more_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* The Homepage and pack it */
	do_entry(entry, label, "Homepage: ", user->GetHomepage());
	pack_hbox(h_box, label, entry);

	/* The birthday and pack it */
	gchar *bday;

	if(user->GetBirthMonth() == 0 || user->GetBirthDay() == 0)
		bday = "N/A";

	else
		bday = g_strdup_printf("%d/%d/%d", user->GetBirthMonth(),
				user->GetBirthDay(), user->GetBirthYear());

	do_entry(entry, label, "Birthday: ", bday);
	pack_hbox(h_box, label, entry);

	/* Put the homepate and birthday on a horz line */
	gtk_box_pack_start(GTK_BOX(more_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* The languages and pack it */
	SLanguage *lang = GetLanguageByCode(user->GetLanguage(0));
	do_entry(entry, label, "Language 1:", lang->szName);
	gtk_widget_set_usize(entry, 75, 20);
	pack_hbox(h_box, label, entry);

	/* Second language and pack it*/
	lang = GetLanguageByCode(user->GetLanguage(1));
	do_entry(entry, label, "Language 2:", lang->szName);
	gtk_widget_set_usize(entry, 75, 20);
	pack_hbox(h_box, label, entry);

	/* Third language and pack it */
	lang = GetLanguageByCode(user->GetLanguage(2));
	do_entry(entry, label, "Language 3:", lang->szName);
	gtk_widget_set_usize(entry, 75, 20);
	pack_hbox(h_box, label, entry);

	/* Pack the the languages */
	gtk_box_pack_start(GTK_BOX(more_box), h_box, FALSE, FALSE, 5);

	/* END MORE TAB */

	h_box = gtk_hbox_new(FALSE, 5);

	/* START WORK TAB */

	/* Company Name and pack it */
	do_entry(entry, label, "Name:    ", user->GetCompanyName());
	pack_hbox(h_box, label, entry);

	/* Department and pack it */
	do_entry(entry, label, "Department:", user->GetCompanyDepartment());
	pack_hbox(h_box, label, entry);

	/* Company Name and Department in a horz line */
	gtk_box_pack_start(GTK_BOX(work_box), h_box, FALSE, FALSE, 5);	

	h_box = gtk_hbox_new(FALSE, 5);

	/* Position and pack it */
	do_entry(entry, label, "Position: ", user->GetCompanyPosition());
	pack_hbox(h_box, label, entry);

	/* Company Homepage and pack it */
	do_entry(entry, label, "Homepage: ", user->GetCompanyHomepage());
	pack_hbox(h_box, label, entry);

	/* Pack Position and Homepage in a horz line */
	gtk_box_pack_start(GTK_BOX(work_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* Company Address */
	do_entry(entry, label, "Address:", user->GetCompanyAddress());
	pack_hbox(h_box, label, entry);

	/* Company Phone Number */
	do_entry(entry, label, "Phone Num:", user->GetCompanyPhoneNumber());
	pack_hbox(h_box, label, entry);

	gtk_box_pack_start(GTK_BOX(work_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(FALSE, 5);

	/* Company City and pack it */
	do_entry(entry, label, "City:      ", user->GetCompanyCity());
	pack_hbox(h_box, label, entry);

	/* Company State and pack it */
	do_entry(entry, label, "State:         ", user->GetCompanyState());
	pack_hbox(h_box, label, entry);

	gtk_box_pack_start(GTK_BOX(work_box), h_box, FALSE, FALSE, 5);

	/* END WORK TAB */

	/* START ABOUT TAB */

	text = gtk_text_new(NULL, NULL);
	gtk_text_set_word_wrap(GTK_TEXT(text), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(text), TRUE);
	gtk_text_freeze(GTK_TEXT(text));
	gtk_text_insert(GTK_TEXT(text), 0, 0, 0, user->GetAbout(), -1);
	gtk_text_thaw(GTK_TEXT(text));
	gtk_container_add(GTK_CONTAINER(v_scroll), text);

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
			   GTK_SIGNAL_FUNC(dialog_close), iu->window);
	gtk_signal_connect(GTK_OBJECT(iu->window), "destroy",
			   GTK_SIGNAL_FUNC(dialog_close), iu->window);
	gtk_signal_connect(GTK_OBJECT(update), "clicked",
			   GTK_SIGNAL_FUNC(update_user_info), iu);

	gtk_box_pack_start(GTK_BOX(v_box), notebook, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	gtk_container_add(GTK_CONTAINER(iu->window), v_box);

	gtk_widget_show_all(iu->window);
}

void update_user_info(GtkWidget *widget, struct info_user *iu)
{
	icq_daemon->icqRequestMetaInfo(iu->user->Uin());
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
