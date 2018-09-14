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

#include "buoh.h"
#include "buoh-comic-list.h"

struct _BuohComicListPrivate {
        GtkWidget        *swindow;
        GtkWidget        *tree_view;
        GtkTreeModel     *model;
        BuohComicManager *comic_manager;

        BuohView         *view;
};

static void buoh_comic_list_init                   (BuohComicList *buoh_comic_list);
static void buoh_comic_list_class_init             (BuohComicListClass *klass);
static void buoh_comic_list_finalize               (GObject *object);

static void buoh_comic_list_get_preferred_width    (GtkWidget        *widget,
                                                    gint             *minimum,
                                                    gint             *natural);
static void     buoh_comic_list_size_allocate      (GtkWidget        *widget,
                                                    GtkAllocation    *allocation);
static void     buoh_comic_list_selection_changed  (GtkTreeSelection *selection,
                                                    gpointer          gdata);
static gboolean buoh_comic_list_visible            (GtkTreeModel     *model,
                                                    GtkTreeIter      *iter,
                                                    gpointer          gdata);

G_DEFINE_TYPE_WITH_PRIVATE (BuohComicList, buoh_comic_list, GTK_TYPE_BIN)

static void
buoh_comic_list_selection_changed (GtkTreeSelection *selection, gpointer gdata)
{
        BuohComicList *comic_list = BUOH_COMIC_LIST (gdata);
        GtkTreeModel  *model;
        GtkTreeIter    iter;
        GObject       *comic_manager;
        BuohComic     *comic;

        if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
                gtk_tree_model_get (model, &iter,
                                    COMIC_LIST_COMIC_MANAGER, &comic_manager,
                                    -1);

                comic_list->priv->comic_manager = BUOH_COMIC_MANAGER (comic_manager);
                comic = buoh_comic_manager_get_current (comic_list->priv->comic_manager);
                g_object_unref (comic_manager);

                buoh_view_set_comic (comic_list->priv->view, comic);
                buoh_debug ("selection changed: set comic");
        } else {
                buoh_view_clear (comic_list->priv->view);
        }
}

static gboolean
buoh_comic_list_visible (GtkTreeModel *model,
                         GtkTreeIter  *iter,
                         gpointer      gdata)
{
        gboolean visible = FALSE;

        gtk_tree_model_get (model, iter, COMIC_LIST_VISIBLE, &visible, -1);

        return visible;
}

static void
buoh_comic_list_init (BuohComicList *buoh_comic_list)
{
        GtkTreeModel      *model;
        GtkCellRenderer   *renderer;
        GtkTreeViewColumn *column;
        GtkTreeSelection  *selection;

        buoh_comic_list->priv = buoh_comic_list_get_instance_private (buoh_comic_list);

        buoh_comic_list->priv->comic_manager = NULL;
        buoh_comic_list->priv->view = NULL;

        model = buoh_get_comics_model (BUOH);
        buoh_comic_list->priv->model = gtk_tree_model_filter_new (model, NULL);
        gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (buoh_comic_list->priv->model),
                                                buoh_comic_list_visible,
                                                NULL, NULL);

        buoh_comic_list->priv->tree_view = gtk_tree_view_new_with_model (buoh_comic_list->priv->model);
        renderer = gtk_cell_renderer_text_new ();
        g_object_set (G_OBJECT (renderer),
                      "ellipsize-set", TRUE,
                      "ellipsize", PANGO_ELLIPSIZE_END,
                      NULL);
        column = gtk_tree_view_column_new_with_attributes (_("Title"), renderer,
                                                           "text", COMIC_LIST_TITLE,
                                                           NULL);
        gtk_tree_view_insert_column (GTK_TREE_VIEW (buoh_comic_list->priv->tree_view),
                                     column, COMIC_LIST_TITLE);

        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (buoh_comic_list->priv->tree_view));
        g_signal_connect (selection, "changed",
                          G_CALLBACK (buoh_comic_list_selection_changed),
                          (gpointer) buoh_comic_list);

        buoh_comic_list->priv->swindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (buoh_comic_list->priv->swindow),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (buoh_comic_list->priv->swindow),
                                             GTK_SHADOW_IN);
        gtk_container_add (GTK_CONTAINER (buoh_comic_list->priv->swindow),
                           buoh_comic_list->priv->tree_view);
        gtk_widget_show (buoh_comic_list->priv->tree_view);

        gtk_container_add (GTK_CONTAINER (buoh_comic_list), buoh_comic_list->priv->swindow);
        gtk_widget_show (buoh_comic_list->priv->swindow);

        gtk_widget_show (GTK_WIDGET (buoh_comic_list));
}

static void
buoh_comic_list_class_init (BuohComicListClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        widget_class->get_preferred_width = buoh_comic_list_get_preferred_width;
        widget_class->size_allocate = buoh_comic_list_size_allocate;

        object_class->finalize = buoh_comic_list_finalize;
}

static void
buoh_comic_list_finalize (GObject *object)
{
        BuohComicList *comic_list = BUOH_COMIC_LIST (object);

        buoh_debug ("comic-list finalize");

        if (comic_list->priv->model) {
                g_object_unref (comic_list->priv->model);
                comic_list->priv->model = NULL;
        }

        if (G_OBJECT_CLASS (buoh_comic_list_parent_class)->finalize) {
                (* G_OBJECT_CLASS (buoh_comic_list_parent_class)->finalize) (object);
        }
}

static void
buoh_comic_list_get_preferred_width (GtkWidget *widget,
                                     gint      *minimum,
                                     gint      *natural)
{
        GtkBin    *bin = GTK_BIN (widget);
        GtkWidget *child;
        gint       child_minimum;
        gint       child_natural;

        child = gtk_bin_get_child (bin);

        if (child && gtk_widget_get_visible (child)) {
                gtk_widget_get_preferred_width (child,
                                                &child_minimum,
                                                &child_natural);
                /* we need some extra size */
                *minimum = child_minimum + 100;
                *natural = child_natural + 100;
        } else {
                *minimum = 0;
                *natural = 0;
        }
}

static void
buoh_comic_list_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
        GtkBin    *bin = GTK_BIN (widget);
        GtkWidget *child;

        child = gtk_bin_get_child (bin);

        gtk_widget_set_allocation (widget, allocation);

        if (child && gtk_widget_get_visible (child)) {
                gtk_widget_size_allocate (child, allocation);

                /* we need some extra size */
                allocation->width += 100;
        }
}

GtkWidget *
buoh_comic_list_new (void)
{
        GtkWidget *buoh_comic_list;

        buoh_comic_list = GTK_WIDGET (g_object_new (BUOH_TYPE_COMIC_LIST,
                                                    "border-width", 6,
                                                    NULL));
        return buoh_comic_list;
}

void
buoh_comic_list_set_view (BuohComicList *comic_list, BuohView *view)
{
        g_return_if_fail (BUOH_IS_COMIC_LIST (comic_list));
        g_return_if_fail (BUOH_IS_VIEW (view));

        if (comic_list->priv->view) {
                return;
        }

        comic_list->priv->view = view;
}

GtkWidget *
buoh_comic_list_get_list (BuohComicList *comic_list)
{
        g_return_val_if_fail (BUOH_IS_COMIC_LIST (comic_list), NULL);

        return comic_list->priv->tree_view;
}

void
buoh_comic_list_clear_selection (BuohComicList *comic_list)
{
        g_return_if_fail (BUOH_IS_COMIC_LIST (comic_list));

        gtk_tree_selection_unselect_all (
                gtk_tree_view_get_selection (GTK_TREE_VIEW (comic_list->priv->tree_view)));
}

BuohComicManager *
buoh_comic_list_get_selected (BuohComicList *comic_list)
{
        g_return_val_if_fail (BUOH_IS_COMIC_LIST (comic_list), NULL);

        return comic_list->priv->comic_manager;
}
