#include "contact_list.h"

#include "icqd.h"
#include "user.h"

#include <string.h>
#include <gtk/gtk.h>

GtkWidget *contact_list_new(gint height, gint width)
{
	/* Create the contact list using a 2 column clist */
	contact_list = gtk_clist_new(2);

	/* Set the selection mode */
	gtk_clist_set_selection_mode(GTK_CLIST(contact_list),
				     GTK_SELECTION_BROWSE);

	/* Set the column widths */
	gtk_clist_set_column_width(GTK_CLIST(contact_list), 0, 10);
	gtk_clist_set_column_width(GTK_CLIST(contact_list), 1, width - 10);

	/* Size the contact list */
	gtk_widget_set_usize(contact_list, width, height);

	return contact_list;
}

GtkWidget *contact_list_refresh()
{
	gchar *status_nick[2];
	gint num_users = 0;
	gint counter = 0; /* For placing users in order in the list by status */

	/* Don't update the clist window, so we can update all the users */
	gtk_clist_freeze(GTK_CLIST(contact_list));

	/* Clean out the list */
	gtk_clist_clear(GTK_CLIST(contact_list));

	/* Now go through all the users */
	FOR_EACH_USER_START(LOCK_R)
	{
		/* Get the status of the user */
		gushort user_status = pUser->Status();

		switch(user_status)
		{
		  case ICQ_STATUS_FREEFORCHAT:
		  {
			status_nick[0] = "F";
			break;
		  }
		  case ICQ_STATUS_ONLINE:
		  {
			status_nick[0] = "+";
			break;
		  }
 		  case ICQ_STATUS_AWAY:
		  {
			status_nick[0] = "A";
			break;
		  }
		  case ICQ_STATUS_NA:
		  {
			status_nick[0] = "N";
			break;
		  }
		  case ICQ_STATUS_DND:
		  {
			status_nick[0] = "D";
			break;
		  }
		  case ICQ_STATUS_OCCUPIED:
		  {
			status_nick[0] = "O";
			break;
		  }
		  case ICQ_STATUS_OFFLINE:
		  {
			status_nick[0] = "-";
			break;
		  }
		  default:
		  {
			status_nick[0] = "?";
		  }
		}

		status_nick[1] = pUser->GetAlias();

		if(strcmp(status_nick[0],"+") == 0)
		{	
  		  gtk_clist_prepend(GTK_CLIST(contact_list), status_nick);
		  counter++;
		}

		else if(strcmp(status_nick[0],"F") == 0)
		{
		  gtk_clist_insert(GTK_CLIST(contact_list),counter,status_nick);
		  counter++;
		}

		else if(strcmp(status_nick[0],"A") == 0)
		{
		  gtk_clist_insert(GTK_CLIST(contact_list),counter,status_nick);
		  counter++;
		}

		else if(strcmp(status_nick[0],"N") == 0)
		{
		  gtk_clist_insert(GTK_CLIST(contact_list),counter,status_nick);
		  counter++;
		}

		else if(strcmp(status_nick[0],"D") == 0)
		{
		  gtk_clist_insert(GTK_CLIST(contact_list),counter,status_nick);
		  counter++;
		}

		else if(strcmp(status_nick[0],"O") == 0)
		{
		  gtk_clist_insert(GTK_CLIST(contact_list),counter,status_nick);
		  counter++;
		}
		
		else
		  gtk_clist_append(GTK_CLIST(contact_list), status_nick);

		gtk_clist_set_row_data(GTK_CLIST(contact_list), num_users, (gpointer)pUser);

		num_users++;
		FOR_EACH_USER_CONTINUE
	}
	FOR_EACH_USER_END

	/* Now show the updated list */
	gtk_clist_thaw(GTK_CLIST(contact_list));
}
