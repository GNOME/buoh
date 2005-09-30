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

#include "buoh-add-comic-dialog.h"
#include "buoh.h"

struct _BuohAddComicDialogPrivate {
	GtkTreeModel *model;
	GtkWidget    *selected_label;
	gint          n_selected;
};

#define BUOH_ADD_COMIC_DIALOG_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_ADD_COMIC_DIALOG, BuohAddComicDialogPrivate))

static GtkDialogClass *parent_class = NULL;

static void buoh_add_comic_dialog_init        (BuohAddComicDialog      *dialog);
static void buoh_add_comic_dialog_class_init  (BuohAddComicDialogClass *klass);

static gint buoh_add_comic_dialog_get_n_selected  (BuohAddComicDialog    *dialog);
static void buoh_comic_add_dialog_update_selected (BuohAddComicDialog    *dialog);
static void buoh_add_comic_toggled_cb             (GtkCellRendererToggle *renderer,
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
	GtkWidget         *frame, *label;
	GtkWidget         *vbox;
	GtkWidget         *swindow;
	GtkWidget         *tree_view;
	GtkCellRenderer   *renderer;
	GtkTreeViewColumn *column;
	gchar             *markup;
	
	dialog->priv = BUOH_ADD_COMIC_DIALOG_GET_PRIVATE (dialog);

	dialog->priv->model = buoh_get_comics_model (BUOH);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Add Comic"));
	gtk_window_set_default_size (GTK_WINDOW (dialog), 400, 300);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (dialog)->vbox), 12);

	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CLOSE,
			       GTK_RESPONSE_ACCEPT);

	label = gtk_label_new (NULL);

	markup = g_strdup_printf ("<b>%s</b>", _("Select Comics"));
	gtk_label_set_markup (GTK_LABEL (label), markup);
	g_free (markup);
	
	frame = gtk_frame_new (NULL);
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_widget_show (label);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	vbox = gtk_vbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);

	/* List */
	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swindow),
					     GTK_SHADOW_IN);
	
	tree_view = gtk_tree_view_new_with_model (dialog->priv->model);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), TRUE);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (tree_view), TRUE);

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
	g_object_set (G_OBJECT (renderer),
		      "width-chars", 20, 
		      "ellipsize-set", TRUE,
		      "ellipsize", PANGO_ELLIPSIZE_END,
		      NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Title"), renderer,
							   "text", COMIC_LIST_TITLE,
							   NULL);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (tree_view),
				     column, COMIC_LIST_TITLE);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),
		      "width-chars", 20,
		      "ellipsize-set", TRUE,
		      "ellipsize", PANGO_ELLIPSIZE_END,
		      NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Author"), renderer,
							   "text", COMIC_LIST_AUTHOR,
							   NULL);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (tree_view),
				     column, COMIC_LIST_AUTHOR);

	gtk_container_add (GTK_CONTAINER (swindow), tree_view);
	gtk_widget_show (tree_view);

	gtk_box_pack_start (GTK_BOX (vbox), swindow, TRUE, TRUE, 0);
	gtk_widget_show (swindow);

	/* Counter */
	dialog->priv->selected_label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (dialog->priv->selected_label),
				0.0, 0.5);
	dialog->priv->n_selected = buoh_add_comic_dialog_get_n_selected (dialog);
	buoh_comic_add_dialog_update_selected (dialog);

	gtk_box_pack_start (GTK_BOX (vbox), dialog->priv->selected_label,
			    FALSE, FALSE, 0);
	gtk_widget_show (dialog->priv->selected_label);
	
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	gtk_widget_show (vbox);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
			    frame, TRUE, TRUE, 0);
	gtk_widget_show (frame);

	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (gtk_widget_destroy),
			  NULL);
}

static void
buoh_add_comic_dialog_class_init (BuohAddComicDialogClass *klass)
{
	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (BuohAddComicDialogPrivate));
}

static gint
buoh_add_comic_dialog_get_n_selected (BuohAddComicDialog *dialog)
{
	GtkTreeIter iter;
	gboolean    valid;
	gboolean    active;
	gint        count = 0;

	valid = gtk_tree_model_get_iter_first (dialog->priv->model, &iter);
	while (valid) {
		gtk_tree_model_get (dialog->priv->model, &iter,
				    COMIC_LIST_VISIBLE, &active,
				    -1);
		if (active)
			count ++;
		
		valid = gtk_tree_model_iter_next (dialog->priv->model, &iter);
	}

	return count;
}

static void
buoh_comic_add_dialog_update_selected (BuohAddComicDialog *dialog)
{
	gchar *text;
	
	text = g_strdup_printf (_("Comics selected: %d"), dialog->priv->n_selected);
	gtk_label_set_text (GTK_LABEL (dialog->priv->selected_label), text);
	g_free (text);
	
	gtk_widget_show (dialog->priv->selected_label);
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

	if (active) 
		dialog->priv->n_selected --;
	else
		dialog->priv->n_selected ++;
	
	buoh_comic_add_dialog_update_selected (dialog);
}

GtkWidget *
buoh_add_comic_dialog_new ()
{
	GtkWidget *dialog;

	dialog = GTK_WIDGET (g_object_new (BUOH_TYPE_ADD_COMIC_DIALOG, NULL));

	return dialog;
}
