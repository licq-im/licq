#include "licq_gtk.h"

#include "licq_icqd.h"
#include "licq_user.h"

static GtkWidget *password1;
static GtkWidget *password2;
static GtkWidget *check;
static GtkWidget *uin;
GtkWidget *register_window;

void registration_wizard()
{
	GtkWidget *notebook;
	GtkWidget *ok;
	GtkWidget *cancel;
	GtkWidget *label;
	GtkWidget *h_box;
	GtkWidget *table;

	/* Create the window */
	register_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(register_window),
			     "Licq - Registration Wizard");
	gtk_window_set_position(GTK_WINDOW(register_window), GTK_WIN_POS_CENTER);

	/* Create a table and the button box */
	table = gtk_table_new(5, 3, FALSE);
	h_box = gtk_hbox_new(TRUE, 5);

	/* Create the notebook */
	notebook = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);

	/* The option of signing up a current user */
	check = gtk_check_button_new_with_label("Register existing UIN:");
	uin = gtk_entry_new_with_max_length(MAX_LENGTH_UIN);
	gtk_widget_set_sensitive(uin, FALSE);

	/* Set the uin entry box back to TRUE if check is check or vice versa */
	gtk_signal_connect(GTK_OBJECT(check), "toggled",
			   GTK_SIGNAL_FUNC(current_button_callback), NULL);
			   
	/* Validate numbers only in the uin box */
	gtk_signal_connect(GTK_OBJECT(uin), "insert-text",
			   GTK_SIGNAL_FUNC(verify_numbers), NULL);

	/* Attach them to the table */
	gtk_table_attach(GTK_TABLE(table), check, 0, 1, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 3);
	gtk_table_attach(GTK_TABLE(table), uin, 1, 2, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 3);

	/* The first password entry box */
	password1 = gtk_entry_new_with_max_length(8);
	gtk_entry_set_visibility(GTK_ENTRY(password1), FALSE);
	label = gtk_label_new("Password:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	
	/* Attach them to the table */
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
			 GTK_FILL, GTK_FILL, 3, 3);
	gtk_table_attach(GTK_TABLE(table), password1, 1, 2, 1, 2,
			 GTK_FILL, GTK_FILL, 3, 3);

	/* The second password entry box */
	password2 = gtk_entry_new_with_max_length(8);
	gtk_entry_set_visibility(GTK_ENTRY(password2), FALSE);
	label = gtk_label_new("Verify Password:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);

	/* Attach them to the table */
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
			 GTK_FILL, GTK_FILL, 3, 3);
	gtk_table_attach(GTK_TABLE(table), password2, 1, 2, 2, 3,
			 GTK_FILL, GTK_FILL, 3, 3);

	/* Create the buttons */
	ok = gtk_button_new_with_label("OK");
	cancel = gtk_button_new_with_label("Cancel");

	/* The "clicked" signals for both buttons */
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(wizard_ok), NULL);
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
			   GTK_SIGNAL_FUNC(wizard_cancel), NULL);

	/* Pack them */
	gtk_box_pack_start(GTK_BOX(h_box), ok, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), cancel, TRUE, TRUE, 0);

	/* Attach the h_box to the table */
	gtk_table_attach(GTK_TABLE(table), h_box, 1, 2, 3, 4,
			 GTK_FILL, GTK_FILL, 3, 3);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, NULL);

	gtk_container_add(GTK_CONTAINER(register_window), notebook);
	gtk_widget_show_all(register_window);
}

void wizard_ok(GtkWidget *widget, gpointer data)
{
	const gchar *password_1 =
		gtk_editable_get_chars(GTK_EDITABLE(password1), 0, -1);
		
	const gchar *password_2 = 
		gtk_editable_get_chars(GTK_EDITABLE(password2), 0, -1);

	if(strcmp(password_1, "") == 0 || strlen(password_1) > 8)
	{
		wizard_message(1);
		return;
	}
	else if(strcmp(password_2, "") == 0 || strcmp(password_1, password_2) != 0)
	{
		wizard_message(2);
		return;
	}

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)))
	{
		unsigned long _uin = atol(gtk_entry_get_text(GTK_ENTRY(uin)));

		if(_uin == 0)
		{
			wizard_message(3);
		}

		/* Set the owner if it is a current uin */
		gUserManager.SetOwnerUin(_uin);
		ICQOwner *owner = gUserManager.FetchOwner(LOCK_W);
		owner->SetPassword(password_1);
		gUserManager.DropOwner();

		wizard_message(6);
	}

	/* Registering a new user */
	else
	{
		gtk_window_set_title(GTK_WINDOW(register_window),
				     "User Registration in Progress ... ");
		icq_daemon->icqRegister(password_1);
		gtk_widget_set_sensitive(password1, FALSE);
		gtk_widget_set_sensitive(password2, FALSE);
		gtk_widget_set_sensitive(check, FALSE);
	}

	g_free(password_1);
	g_free(password_2);
}	

void wizard_cancel(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

void current_button_callback(GtkWidget *widget, gpointer data)
{
	gtk_widget_set_sensitive(uin,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)));
	
}

void wizard_message(int mes)
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *ok;
	gchar *message;
	gchar *buf;
	
	switch(mes)
	{
	case 1:
		message="Invalid password, must be 8 characters or less.";
		break;
	case 2:
		message = "Passwords do not match, try again.";
		break;
	case 3:
		message = "Invalid UIN, try again.";
		break;
	case 4:
		message = "Registration failed.\nSee network window for details.";
		break;
	case 5:
		message = "Successfuly registered.";
		break;
	case 6:
		message = "Registered successfully.";
	default:
		break;
	}

	/* Create the dialog */
	dialog = gtk_dialog_new();

	/* The label */
	label = gtk_label_new(message);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);

	/* The ok button */
	ok = gtk_button_new_with_label("OK");
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
			  ok);

	/* Close the dialog window when ok is clicked */
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(dialog_close), dialog);
	
	gtk_widget_show_all(dialog);
}
