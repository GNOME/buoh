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
        gint          n_selected;
};

static void buoh_add_comic_dialog_init        (BuohAddComicDialog      *dialog);
static void buoh_add_comic_dialog_class_init  (BuohAddComicDialogClass *klass);

static gint buoh_add_comic_dialog_get_n_selected  (BuohAddComicDialog    *dialog);
static void buoh_comic_add_dialog_update_selected (BuohAddComicDialog    *dialog);
static void buoh_add_comic_toggled_cb             (GtkCellRendererToggle *renderer,
                                                   gchar                 *path,
                                                   gpointer               gdata);

G_DEFINE_TYPE_WITH_PRIVATE (BuohAddComicDialog, buoh_add_comic_dialog, GTK_TYPE_DIALOG)

static void
buoh_add_comic_dialog_init (BuohAddComicDialog *dialog)
{
        dialog->priv = buoh_add_comic_dialog_get_instance_private (dialog);

        gtk_widget_init_template (GTK_WIDGET (dialog));

        dialog->priv->model = buoh_get_comics_model (BUOH);

        gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->tree_view), dialog->priv->model);

        /* Counter */
        dialog->priv->n_selected = buoh_add_comic_dialog_get_n_selected (dialog);
        buoh_comic_add_dialog_update_selected (dialog);
}

static void
buoh_add_comic_dialog_class_init (BuohAddComicDialogClass *klass)
{
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/buoh/ui/add-comic-dialog.ui");

        gtk_widget_class_bind_template_child (widget_class, BuohAddComicDialog, selected_label);
        gtk_widget_class_bind_template_child (widget_class, BuohAddComicDialog, tree_view);

        gtk_widget_class_bind_template_callback (widget_class, buoh_add_comic_toggled_cb);
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
                if (active) {
                        count ++;
                }

                valid = gtk_tree_model_iter_next (dialog->priv->model, &iter);
        }

        return count;
}

static void
buoh_comic_add_dialog_update_selected (BuohAddComicDialog *dialog)
{
        gchar *text;

        text = g_strdup_printf (_("Comics selected: %d"), dialog->priv->n_selected);
        gtk_label_set_text (GTK_LABEL (dialog->selected_label), text);
        g_free (text);
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

        if (active) {
                dialog->priv->n_selected --;
        } else {
                dialog->priv->n_selected ++;
        }

        buoh_comic_add_dialog_update_selected (dialog);
}

GtkWidget *
buoh_add_comic_dialog_new ()
{
        GtkWidget *dialog;

        dialog = GTK_WIDGET (g_object_new (BUOH_TYPE_ADD_COMIC_DIALOG, NULL));

        return dialog;
}
