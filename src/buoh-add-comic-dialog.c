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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

#include "buoh-add-comic-dialog.h"
#include "buoh.h"

struct _BuohAddComicDialogPrivate {
	GladeXML     *gui;
	GtkTreeModel *model;
};

#define BUOH_ADD_COMIC_DIALOG_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_ADD_COMIC_DIALOG, BuohAddComicDialogPrivate))

static GtkDialogClass *parent_class = NULL;

static void buoh_add_comic_dialog_init        (BuohAddComicDialog      *dialog);
static void buoh_add_comic_dialog_class_init  (BuohAddComicDialogClass *klass);
static void buoh_add_comic_dialog_finalize    (GObject                 *object);

static void buoh_add_comic_toggled_cb         (GtkCellRendererToggle *renderer,
					       gchar                 *path,
					       gpointer               gdata);

GType
buoh_add_comic_dialog_get_type ()
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (BuohAddComicDialogClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) buoh_add_comic_dialog_class_init,
			NULL,
			NULL,
			sizeof (BuohAddComicDialog),
			0,
			(GInstanceInitFunc) buoh_add_comic_dialog_init
		};

		type = g_type_register_static (GTK_TYPE_DIALOG, "BuohAddComicDialog",
					       &info, 0);
	}

	return type;
}

static void
buoh_add_comic_dialog_init (BuohAddComicDialog *dialog)
{
	GtkWidget         *frame, *parent;
	GtkWidget         *tree_view; 
	GtkCellRenderer   *renderer;
	GtkTreeViewColumn *column;
	gchar             *glade_path;
	
	dialog->priv = BUOH_ADD_COMIC_DIALOG_GET_PRIVATE (dialog);

	dialog->priv->model = buoh_get_comics_model (BUOH);

	glade_path = g_build_filename (INTERFACES_DIR, "buoh.glade", NULL);
	dialog->priv->gui = glade_xml_new (glade_path, NULL, NULL);
	g_free (glade_path);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Add Comic"));
	gtk_window_set_default_size (GTK_WINDOW (dialog), 400, 300);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (dialog)->vbox), 12);

	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CLOSE,
			       GTK_RESPONSE_ACCEPT);

	/* Reparent frame */
	frame = glade_xml_get_widget (dialog->priv->gui, "add_comic_frame");
	parent = glade_xml_get_widget (dialog->priv->gui, "add_comic_box");
	gtk_widget_ref (frame);
	gtk_container_remove (GTK_CONTAINER (parent), frame);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
			    frame, TRUE, TRUE, 0);
	gtk_widget_unref (frame);
	gtk_widget_show_all (frame);

	tree_view = glade_xml_get_widget (dialog->priv->gui, "supported_comic_list");
	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), dialog->priv->model);

	renderer = gtk_cell_renderer_toggle_new ();
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer,
							   "active", COMIC_LIST_VISIBLE,
							   NULL);
	g_signal_connect (G_OBJECT (renderer), "toggled",
			  G_CALLBACK (buoh_add_comic_toggled_cb),
			  (gpointer) dialog);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (tree_view),
				     column, COMIC_LIST_VISIBLE);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Title"), renderer,
							   "text", COMIC_LIST_TITLE,
							   NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (tree_view),
				     column, COMIC_LIST_TITLE);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Author"), renderer,
							   "text", COMIC_LIST_AUTHOR,
							   NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (tree_view),
				     column, COMIC_LIST_AUTHOR);

	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (gtk_widget_destroy),
			  NULL);
}

static void
buoh_add_comic_dialog_class_init (BuohAddComicDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (BuohAddComicDialogPrivate));

	object_class->finalize = buoh_add_comic_dialog_finalize;
}

static void
buoh_add_comic_dialog_finalize (GObject *object)
{
	BuohAddComicDialog *dialog = BUOH_ADD_COMIC_DIALOG (object);

	g_return_if_fail (BUOH_IS_ADD_COMIC_DIALOG (object));

	g_debug ("buoh-add-comic-dialog finalize\n");

	if (dialog->priv->gui) {
		g_object_unref (dialog->priv->gui);
		dialog->priv->gui = NULL;
	}

	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
buoh_add_comic_toggled_cb (GtkCellRendererToggle *renderer,
			   gchar *path, gpointer gdata)
{
	BuohAddComicDialog *dialog = BUOH_ADD_COMIC_DIALOG (gdata);
	GtkTreeIter         iter;
	gboolean            active;

	active = gtk_cell_renderer_toggle_get_active (renderer);
	
	gtk_tree_model_get_iter_from_string (dialog->priv->model, &iter, path);
	gtk_list_store_set (GTK_LIST_STORE (dialog->priv->model),
			    &iter,
			    COMIC_LIST_VISIBLE, !active,
			    -1);
}

GtkWidget *
buoh_add_comic_dialog_new ()
{
	GtkWidget *dialog;

	dialog = GTK_WIDGET (g_object_new (BUOH_TYPE_ADD_COMIC_DIALOG, NULL));

	return dialog;
}
