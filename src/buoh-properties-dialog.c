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

struct _BuohPropertiesDialogPrivate {
	Comic *comic;
};

#define BUOH_PROPERTIES_DIALOG_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_PROPERTIES_DIALOG, BuohPropertiesDialogPrivate))

static GtkDialogClass *parent_class = NULL;

static void buoh_properties_dialog_init                   (BuohPropertiesDialog      *dialog);
static void buoh_properties_dialog_class_init             (BuohPropertiesDialogClass *klass);

GType
buoh_properties_dialog_get_type ()
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

	dialog->priv->comic = NULL;

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
buoh_properties_dialog_set_comic (BuohPropertiesDialog *dialog, Comic *comic)
{
	g_return_if_fail (IS_COMIC (comic));

	dialog->priv->comic = comic;

	/* TODO: fill the dialog */
}

GtkWidget *
buoh_properties_dialog_new ()
{
	GtkWidget *dialog;

	dialog = GTK_WIDGET (g_object_new (BUOH_TYPE_PROPERTIES_DIALOG, NULL));

	return dialog;
}
	
