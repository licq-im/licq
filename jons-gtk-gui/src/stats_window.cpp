#include "licq_gtk.h"
#include <gtk/gtk.h>

#include "licq_icqd.h"

void menu_daemon_stats()
{
	GtkWidget *dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog),
		"Licq - Statistics");
	gtk_window_set_position(GTK_WINDOW(dialog),
		GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 7);
	gtk_signal_connect(GTK_OBJECT(dialog), "destroy",
		GTK_SIGNAL_FUNC(dialog_close), dialog);

	gchar info[256]; // This should be enough for all the stats

#ifdef SAVE_STATS
	strcpy(info, "Daemon Statistics\n(Today/Total)\n");

	time_t uptime = time(NULL) - icq_daemon->StartTime();
	time_t reset  = icq_daemon->ResetTime();

	// Licq's uptime
	gchar temp[11];
	strcat(info, "Uptime: ");
	
	if(uptime >= (60 * 60 * 24))
	{
		int nDays = uptime / (60 * 60 * 24);
		if(nDays > 1)
			g_snprintf(temp, 9, "%02d Days ", nDays);
		else
			g_snprintf(temp, 8, "%02d Day ", nDays);
		strcat(info, temp);
		uptime -= (60 * 60 * 24) * nDays;
	}

	if(uptime >= (60 * 60))
	{
		int nHours = uptime / (60 * 60);
		if(nHours > 1)
			g_snprintf(temp, 10, "%02d Hours ", nHours);
		else
			g_snprintf(temp, 9, "%02d Hour ", nHours);
		strcat(info, temp);
		uptime -= (60 * 60) * nHours;
	}

	if(uptime >= 60)
	{
		int nMin = uptime / 60;
		if(nMin > 1)
			g_snprintf(temp, 9, "%02d Mins ", nMin);
		else
			g_snprintf(temp, 8, "%02d Min ", nMin);
		strcat(info, temp);
		uptime -= 60 * nMin;
	}

	if(uptime > 0)
	{
		if(uptime > 1)
			g_snprintf(temp, 8, "%02ld Secs", uptime);
		else
			g_snprintf(temp, 7, "%02ld Sec", uptime);
		strcat(info, temp);
	}

	strcat(info, "\n");

	// Last reset
	strcat(info, "Last Reset: ");
	strcat(info, ctime(&reset));

	DaemonStatsList::iterator iter;
	for(iter = icq_daemon->AllStats().begin();
		iter != icq_daemon->AllStats().end(); iter++)
	{
		strcat(info, iter->Name());
		strcat(info, ": ");
		g_snprintf(temp, 10, "%ld", iter->Today());
		strcat(info, temp);
		strcat(info, " / ");
		g_snprintf(temp, 10, "%ld\n", iter->Total());
		strcat(info, temp);
	}
#else

	strcpy(info, "Daemon Statistics\n");

	time_t uptime = time(NULL) - icq_daemon->StartTime();
	time_t reset  = icq_daemon->ResetTime();

	// Licq's uptime
	gchar temp[11];
	strcat(info, "Uptime: ");
	
	if(uptime >= (60 * 60 * 24))
	{
		int nDays = uptime / (60 * 60 * 24);
		if(nDays > 1)
			g_snprintf(temp, 9, "%02d Days ", nDays);
		else
			g_snprintf(temp, 8, "%02d Day ", nDays);
		strcat(info, temp);
		uptime -= (60 * 60 * 24) * nDays;
	}

	if(uptime >= (60 * 60))
	{
		int nHours = uptime / (60 * 60);
		if(nHours > 1)
			g_snprintf(temp, 10, "%02d Hours ", nHours);
		else
			g_snprintf(temp, 9, "%02d Hour ", nHours);
		strcat(info, temp);
		uptime -= (60 * 60) * nHours;
	}

	if(uptime >= 60)
	{
		int nMin = uptime / 60;
		if(nMin > 1)
			g_snprintf(temp, 9, "%02d Mins ", nMin);
		else
			g_snprintf(temp, 8, "%02d Min ", nMin);
		strcat(info, temp);
		uptime -= 60 * nMin;
	}

	if(uptime > 0)
	{
		if(uptime > 1)
			g_snprintf(temp, 8, "%02ld Secs", uptime);
		else
			g_snprintf(temp, 7, "%02ld Sec", uptime);
		strcat(info, temp);
	}

	strcat(info, "\n");

	// Last reset
	strcat(info, "Last Reset: ");
	strcat(info, ctime(&reset));

	DaemonStatsList::iterator iter;
	for(iter = icq_daemon->AllStats().begin();
		iter != icq_daemon->AllStats().end(); iter++)
	{
		strcat(info, iter->Name());
		strcat(info, ": ");
		g_snprintf(temp, 10, "%ld\n", iter->Today());
		strcat(info, temp);
	}
#endif

	GtkWidget *label = gtk_label_new(info);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);

	GtkWidget *ok = gtk_button_new_with_label("OK");
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
		GTK_SIGNAL_FUNC(stats_ok_callback),
		static_cast<gpointer>(dialog));

#ifdef SAVE_STATS
	GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
	gtk_signal_connect(GTK_OBJECT(btn_reset), "clicked",
		GTK_SIGNAL_FUNC(stats_reset_callback),
		static_cast<gpointer>(dialog));
#endif
	GtkWidget *h_box = gtk_hbox_new(true, 0);
	gtk_box_pack_start(GTK_BOX(h_box), ok, true, true, 5);
#ifdef SAVE_STATS
	gtk_box_pack_start(GTK_BOX(h_box), btn_reset, true, true, 5);
#endif
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
		h_box);

	gtk_widget_show_all(dialog);
}

void stats_ok_callback(GtkWidget *widget, gpointer data)
{
	dialog_close(0, static_cast<GtkWidget *>(data));
}

void stats_reset_callback(GtkWidget *widget, gpointer data)
{
	icq_daemon->ResetStats();
	dialog_close(0, static_cast<GtkWidget *>(data));
}
