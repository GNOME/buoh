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
 *  Authors: Pablo Arroyo Loma (zioma) <zioma@linups.org>
 *           Esteban Sanchez Muñoz (steve-o) <esteban@steve-o.org>
 *           Carlos García Campos <carlosgc@gnome.org>
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
        GtkWidget        *title;
        GtkWidget        *author;
        GtkWidget        *uri;
        GtkWidget        *language;
        GtkWidget        *date;
        GtkWidget        *publication_days;
        GtkWidget        *thumbnail;
};

static void buoh_properties_dialog_init       (BuohPropertiesDialog      *dialog);
static void buoh_properties_dialog_class_init (BuohPropertiesDialogClass *klass);

G_DEFINE_TYPE_WITH_PRIVATE (BuohPropertiesDialog, buoh_properties_dialog, GTK_TYPE_DIALOG)

static void
buoh_properties_dialog_init (BuohPropertiesDialog *dialog)
{
        dialog->priv = buoh_properties_dialog_get_instance_private (dialog);

        gtk_widget_init_template (GTK_WIDGET (dialog));
}

static void
buoh_properties_dialog_class_init (BuohPropertiesDialogClass *klass)
{
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/buoh/ui/properties-dialog.ui");

        gtk_widget_class_bind_template_child_private (widget_class, BuohPropertiesDialog, title);
        gtk_widget_class_bind_template_child_private (widget_class, BuohPropertiesDialog, author);
        gtk_widget_class_bind_template_child_private (widget_class, BuohPropertiesDialog, uri);
        gtk_widget_class_bind_template_child_private (widget_class, BuohPropertiesDialog, language);
        gtk_widget_class_bind_template_child_private (widget_class, BuohPropertiesDialog, date);
        gtk_widget_class_bind_template_child_private (widget_class, BuohPropertiesDialog, publication_days);
        gtk_widget_class_bind_template_child_private (widget_class, BuohPropertiesDialog, thumbnail);
}

void
buoh_properties_dialog_set_comic_manager (BuohPropertiesDialog   *dialog,
                                          BuohComicManager       *comic_manager)
{
        GDate     *comic_date;
        gchar      date[DATE_BUFFER];
        gchar     *pub_days;
        BuohComic *comic;
        GdkPixbuf *thumbnail;

        g_return_if_fail (BUOH_IS_PROPERTIES_DIALOG (dialog));
        g_return_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager));

        dialog->priv->comic_manager = comic_manager;

        comic = buoh_comic_manager_get_current (comic_manager);

        thumbnail = buoh_comic_get_thumbnail (comic);
        gtk_image_set_from_pixbuf (GTK_IMAGE (dialog->priv->thumbnail), thumbnail);
        g_object_unref (thumbnail);

        gtk_label_set_text (GTK_LABEL (dialog->priv->title), buoh_comic_manager_get_title (comic_manager));
        gtk_label_set_text (GTK_LABEL (dialog->priv->author), buoh_comic_manager_get_author (comic_manager));
        gtk_label_set_text (GTK_LABEL (dialog->priv->uri), buoh_comic_get_uri (comic));
        gtk_label_set_text (GTK_LABEL (dialog->priv->language), buoh_comic_manager_get_language (comic_manager));

        comic_date = buoh_comic_get_date (comic);
        if (g_date_strftime (date, DATE_BUFFER,
                             "%x", /* Date in locale preferred format */
                             comic_date) == 0) {
                buoh_debug ("Date buffer too short");
        }
        gtk_label_set_text (GTK_LABEL (dialog->priv->date), date);

        if (BUOH_IS_COMIC_MANAGER_DATE (comic_manager)) {
                pub_days = buoh_comic_manager_date_get_publication_days (BUOH_COMIC_MANAGER_DATE (comic_manager));
                gtk_label_set_text (GTK_LABEL (dialog->priv->publication_days), pub_days);
                g_free (pub_days);
        }
}

BuohComicManager *
buoh_properties_dialog_get_comic_manager (BuohPropertiesDialog *dialog)
{
        g_return_val_if_fail (BUOH_IS_PROPERTIES_DIALOG (dialog), NULL);

        return dialog->priv->comic_manager;
}

GtkWidget *
buoh_properties_dialog_new (void)
{
        GtkWidget *dialog;

        dialog = GTK_WIDGET (g_object_new (BUOH_TYPE_PROPERTIES_DIALOG, NULL));

        return dialog;
}
