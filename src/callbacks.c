/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Authors: Pablo Arroyo Loma (zioma) <zioma@linups.org>
 *          Esteban Sanchez Munoz (steve-o) <steve-o@linups.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgnomevfs/gnome-vfs.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#include "buoh.h"
#include "comic-simple.h"

#include "callbacks.h"

void buoh_gui_new_dialog_reset (Buoh *buoh);

enum {
	TITLE_COLUMN,
	AUTHOR_COLUMN,
	COMIC_COLUMN,
	N_COLUMNS
};

void
buoh_gui_hide_window (GtkWidget *widget, gpointer *gdata)
{
	if (GTK_IS_WIDGET (gdata))
		gtk_widget_hide (GTK_WIDGET (gdata));
	else
		gtk_widget_hide (GTK_WIDGET (widget));
}

static void
buoh_gui_menu_about_activate (GtkWidget *widget, gpointer *gdata)
{
	GdkPixbuf          *pixbuf;
	gchar              *pixbuf_path;
	static const gchar *authors[] = {
		"Esteban Sánchez Muñoz <steve-o@linups.org>",
		"Pablo Arroyo Loma <zioma@linups.org>",
		NULL
	};
	   
	pixbuf_path = g_build_filename (PIXMAPS_DIR, "buoh64x64.png", NULL);
	pixbuf = gdk_pixbuf_new_from_file_at_size (pixbuf_path, 64, 64, NULL);
	g_free (pixbuf_path);

	gtk_show_about_dialog (NULL,
			       "name",        _("Buoh comics reader"),
			       "version",     VERSION,
			       "copyright",   "Copyright \xC2\xA9 2004 Esteban Sánchez Muñoz - Pablo Arroyo Loma",
			       "authors",     authors,
			       "translator-credits",  _("translator-credits"),
			       "logo",        pixbuf,
			       NULL);

	if (pixbuf)
		g_object_unref (pixbuf);
}

void
buoh_gui_new_activate (GtkWidget *widget, gpointer *gdata)
{
	Buoh      *buoh;
	GtkWidget *new_dialog;

	buoh = BUOH (gdata);
	   
	new_dialog = buoh_get_widget (buoh, "new_comic_dialog");

	buoh_gui_new_dialog_reset (buoh);

	gtk_widget_show (GTK_WIDGET (new_dialog));
	   
	return;
}

void
buoh_gui_new_dialog_reset (Buoh *buoh)
{
	GtkWidget *title_entry;
	GtkWidget *author_entry;
	GtkWidget *uri_entry;

	title_entry = buoh_get_widget (buoh, "new_comic_title_entry");
	author_entry = buoh_get_widget (buoh, "new_comic_author_entry");
	uri_entry = buoh_get_widget (buoh, "new_comic_uri_entry");

	gtk_entry_set_text (GTK_ENTRY (title_entry), "");
	gtk_entry_set_text (GTK_ENTRY (author_entry), "");
	gtk_entry_set_text (GTK_ENTRY (uri_entry), "");

	return;
}

void
buoh_gui_new_dialog_ok_clicked (GtkWidget *widget, gpointer *gdata)
{
	Buoh         *buoh;
	ComicSimple  *comic;
	GtkWidget    *new_dialog;
	GtkWidget    *title_entry;
	GtkWidget    *author_entry;
	GtkWidget    *uri_entry;
        const gchar  *title_value, *author_value, *uri_value;
	gchar        *comic_title;

	buoh = BUOH (gdata);

	new_dialog = buoh_get_widget (buoh, "new_comic_dialog");
	title_entry = buoh_get_widget (buoh, "new_comic_title_entry");
	author_entry = buoh_get_widget (buoh, "new_comic_author_entry");
	uri_entry = buoh_get_widget (buoh, "new_comic_uri_entry");

	title_value = gtk_entry_get_text (GTK_ENTRY (title_entry));
	author_value = gtk_entry_get_text (GTK_ENTRY (author_entry));
        uri_value = gtk_entry_get_text (GTK_ENTRY (uri_entry));

	comic = comic_simple_new ();
//	comic = comic_simple_new_with_info (title_value, author_value,
//					    uri_value);
	comic_title = comic_get_title (COMIC (comic));
	g_free (comic_title);

	buoh_load_comic_simple (buoh, (gpointer) comic);

	gtk_widget_hide (new_dialog);

	return;
}




