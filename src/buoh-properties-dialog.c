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
 *
 *  Authors : Pablo Arroyo Loma (zioma) <zioma@linups.org>
 *            Esteban Sanchez Muñoz (steve-o) <esteban@steve-o.org>
 *            Carlos García Campos <carlosgc@gnome.org>
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "buoh-properties-dialog.h"
#include "buoh.h"
#include "buoh-comic-manager-date.h"

#define DATE_BUFFER 256

struct _BuohPropertiesDialogPrivate {
	BuohComicManager *comic_manager;
};

#define BUOH_PROPERTIES_DIALOG_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_PROPERTIES_DIALOG, BuohPropertiesDialogPrivate))

static GtkDialogClass *parent_class = NULL;

static void buoh_properties_dialog_init       (BuohPropertiesDialog      *dialog);
static void buoh_properties_dialog_class_init (BuohPropertiesDialogClass *klass);

GType
buoh_properties_dialog_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (BuohPropertiesDialogClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) buoh_properties_dialog_class_init,
			NULL,
			NULL,
			sizeof (BuohPropertiesDialog),
			0,
			(GInstanceInitFunc) buoh_properties_dialog_init
		};

		type = g_type_register_static (GTK_TYPE_DIALOG, "BuohPropertiesDialog",
					       &info, 0);
	}

	return type;
}

static void
buoh_properties_dialog_init (BuohPropertiesDialog *dialog)
{
	dialog->priv = BUOH_PROPERTIES_DIALOG_GET_PRIVATE (dialog);

	dialog->priv->comic_manager = NULL;

	gtk_window_set_title (GTK_WINDOW (dialog), _("Comic Properties"));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (dialog)->vbox), 12);

	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CLOSE,
			       GTK_RESPONSE_ACCEPT);

	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (gtk_widget_destroy),
			  NULL);
}

static void
buoh_properties_dialog_class_init (BuohPropertiesDialogClass *klass)
{
	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (BuohPropertiesDialogPrivate));
}

void
buoh_properties_dialog_set_comic_manager (BuohPropertiesDialog *dialog,
					  BuohComicManager *comic_manager)
{
	GtkWidget *table;
	GtkWidget *label_title, *label_title_val;
	GtkWidget *label_author, * label_author_val;
	GtkWidget *label_uri, *label_uri_val;
	GtkWidget *label_language, *label_language_val;
	GtkWidget *label_date, *label_date_val;
	GtkWidget *label_pub_days, *label_pub_days_val;
	GtkWidget *image;
	GDate     *comic_date;
	gchar     *title, *author, *uri, *language, date[DATE_BUFFER];
	gchar     *pub_days;
	BuohComic *comic;
	GdkPixbuf *thumbnail;
	gchar     *str;
	
	g_return_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager));

	dialog->priv->comic_manager = comic_manager;

	table = gtk_table_new (5, 3, FALSE);

	comic = buoh_comic_manager_get_current (comic_manager);
	
	thumbnail = buoh_comic_get_thumbnail (comic);
	image = gtk_image_new_from_pixbuf (thumbnail);
	gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0);

	g_object_get (comic_manager,
		      "title", &title,
		      "author", &author,
		      "language", &language, NULL);

	
	str = g_strdup_printf ("<b>%s:</b>", _("Title"));
	label_title = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label_title), str);
	gtk_misc_set_alignment (GTK_MISC (label_title), 0, 0.5);
	g_free (str);

	str = g_strdup_printf ("<b>%s:</b>", _("Author"));
	label_author = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label_author), str);
	gtk_misc_set_alignment (GTK_MISC (label_author), 0, 0.5);
	g_free (str);

	str = g_strdup_printf ("<b>%s:</b>", _("Link"));
	label_uri = gtk_label_new (NULL);	
	gtk_label_set_markup (GTK_LABEL (label_uri), str);
	gtk_misc_set_alignment (GTK_MISC (label_uri), 0, 0.5);
	g_free (str);

	str = g_strdup_printf ("<b>%s:</b>", _("Language"));
	label_language = gtk_label_new (NULL);	
	gtk_label_set_markup (GTK_LABEL (label_language), str);
	gtk_misc_set_alignment (GTK_MISC (label_language), 0, 0.5);
	g_free (str);

	str = g_strdup_printf ("<b>%s:</b>", _("Publication date"));
	label_date = gtk_label_new (NULL);	
	gtk_label_set_markup (GTK_LABEL (label_date), str);
	gtk_misc_set_alignment (GTK_MISC (label_date), 0, 0.5);
	g_free (str);

	g_object_get (comic_manager,
		      "title", &title,
		      "author", &author,
		      "language", &language, NULL);

	label_title_val = gtk_label_new (title);
	gtk_label_set_selectable (GTK_LABEL (label_title_val), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label_title_val), 0, 0.5);
	g_free (title);
	
	label_author_val = gtk_label_new (author);
	gtk_label_set_selectable (GTK_LABEL (label_author_val), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label_author_val), 0, 0.5);
	g_free (author);

	uri = buoh_comic_get_uri (comic);
	label_uri_val = gtk_label_new (uri);
	gtk_label_set_selectable (GTK_LABEL (label_uri_val), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label_uri_val), 0, 0.5);
	gtk_label_set_width_chars (GTK_LABEL (label_uri_val), 35);
	gtk_label_set_ellipsize (GTK_LABEL (label_uri_val), PANGO_ELLIPSIZE_END);	
	g_free (uri);
	
	label_language_val = gtk_label_new (language);
	gtk_label_set_selectable (GTK_LABEL (label_language_val), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label_language_val), 0, 0.5);	
	g_free (language);

	comic_date = buoh_comic_get_date (comic);
	if (g_date_strftime (date, DATE_BUFFER,
			     "%x", /* Date in locale preferred format */
			     comic_date) == 0) {
		buoh_debug ("Date buffer too short");
	}
	label_date_val = gtk_label_new (date);
	gtk_label_set_selectable (GTK_LABEL (label_date_val), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label_date_val), 0, 0.5);	

	gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (image),
			  0, 1, 0, 5, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_title),
			  1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_title_val),
			  2, 3, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_author),
			  1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0 );
	gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_author_val),
			  2, 3, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_uri),
			  1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_uri_val),
			  2, 3, 2, 3, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_date),
			  1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_date_val),
			  2, 3, 3, 4, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_language),
			  1, 2, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_language_val),
			  2, 3, 4, 5, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	
	if (BUOH_IS_COMIC_MANAGER_DATE (comic_manager)) {
		str = g_strdup_printf ("<b>%s:</b>", _("Publication days"));
		label_pub_days = gtk_label_new (NULL);	
		gtk_label_set_markup (GTK_LABEL (label_pub_days), str);
		gtk_misc_set_alignment (GTK_MISC (label_pub_days), 0, 0.5);
		g_free (str);
		
		pub_days = buoh_comic_manager_date_get_publication_days (BUOH_COMIC_MANAGER_DATE (comic_manager));
		
		label_pub_days_val = gtk_label_new (pub_days);
		gtk_label_set_selectable (GTK_LABEL (label_pub_days_val), TRUE);
		gtk_misc_set_alignment (GTK_MISC (label_pub_days_val), 0, 0.5);
		g_free (pub_days);
		
		gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_pub_days),
			  1, 2, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
		gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label_pub_days_val),
			  2, 3, 5, 6, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
		
		gtk_widget_show (label_pub_days);
		gtk_widget_show (label_pub_days_val);
	}

	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	gtk_widget_show (image);
	gtk_widget_show (label_title);
	gtk_widget_show (label_title_val);
	gtk_widget_show (label_author);
	gtk_widget_show (label_author_val);
	gtk_widget_show (label_uri);
	gtk_widget_show (label_uri_val);
	gtk_widget_show (label_language);
	gtk_widget_show (label_language_val);
	gtk_widget_show (label_date);
	gtk_widget_show (label_date_val);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
			    table, TRUE, TRUE, 0);

	gtk_widget_show (table);	
}

BuohComicManager *
buoh_properties_dialog_get_comic_manager (BuohPropertiesDialog *dialog)
{
	return dialog->priv->comic_manager;
}

GtkWidget *
buoh_properties_dialog_new (void)
{
	GtkWidget *dialog;

	dialog = GTK_WIDGET (g_object_new (BUOH_TYPE_PROPERTIES_DIALOG, NULL));

	return dialog;
}
	
