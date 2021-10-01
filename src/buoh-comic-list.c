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

#include "buoh-application.h"
#include "buoh-comic-list.h"

/**
 * BuohComicList:
 * @tree_view: The #GtkTreeView displaying the list subscribed comic series.
 * @model: The model of @tree_view.
 * @comic_manager: Manager representing the currently selected series in the @tree_view.
 *
 * A widget displaying the list of subscribed comic series for switching between them.
 */

struct _BuohComicList {
        GtkBin            parent;

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

G_DEFINE_TYPE (BuohComicList, buoh_comic_list, GTK_TYPE_BIN)

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

                comic_list->comic_manager = BUOH_COMIC_MANAGER (comic_manager);
                comic = buoh_comic_manager_get_current (comic_list->comic_manager);
                g_object_unref (comic_manager);

                buoh_view_set_comic (comic_list->view, comic);
                buoh_debug ("selection changed: set comic");
        } else {
                if (comic_list->view != NULL) {
                        buoh_view_clear (comic_list->view);
                }
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
        gtk_widget_init_template (GTK_WIDGET (buoh_comic_list));
}

static void
buoh_comic_list_class_init (BuohComicListClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        widget_class->get_preferred_width = buoh_comic_list_get_preferred_width;
        widget_class->size_allocate = buoh_comic_list_size_allocate;

        object_class->finalize = buoh_comic_list_finalize;

        gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/buoh/ui/comic-list.ui");

        gtk_widget_class_bind_template_child (widget_class, BuohComicList, tree_view);

        gtk_widget_class_bind_template_callback (widget_class, buoh_comic_list_selection_changed);
}

static void
buoh_comic_list_finalize (GObject *object)
{
        BuohComicList *comic_list = BUOH_COMIC_LIST (object);

        buoh_debug ("comic-list finalize");

        if (comic_list->model) {
                g_object_unref (comic_list->model);
                comic_list->model = NULL;
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
        return GTK_WIDGET (g_object_new (BUOH_TYPE_COMIC_LIST, NULL));
}

void
buoh_comic_list_set_view (BuohComicList *comic_list, BuohView *view)
{
        g_return_if_fail (BUOH_IS_COMIC_LIST (comic_list));
        g_return_if_fail (BUOH_IS_VIEW (view));

        if (comic_list->view) {
                return;
        }

        comic_list->view = view;
}

void
buoh_comic_list_set_model (BuohComicList *comic_list, GtkTreeModel *model)
{
        g_return_if_fail (BUOH_IS_COMIC_LIST (comic_list));
        g_return_if_fail (GTK_IS_TREE_MODEL (model));

        comic_list->model = gtk_tree_model_filter_new (model, NULL);
        gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (comic_list->model),
                                                buoh_comic_list_visible,
                                                NULL, NULL);

        gtk_tree_view_set_model (GTK_TREE_VIEW (comic_list->tree_view),
                                 comic_list->model);
}

GtkWidget *
buoh_comic_list_get_list (BuohComicList *comic_list)
{
        g_return_val_if_fail (BUOH_IS_COMIC_LIST (comic_list), NULL);

        return comic_list->tree_view;
}

void
buoh_comic_list_clear_selection (BuohComicList *comic_list)
{
        g_return_if_fail (BUOH_IS_COMIC_LIST (comic_list));

        gtk_tree_selection_unselect_all (
                gtk_tree_view_get_selection (GTK_TREE_VIEW (comic_list->tree_view)));
}

BuohComicManager *
buoh_comic_list_get_selected (BuohComicList *comic_list)
{
        g_return_val_if_fail (BUOH_IS_COMIC_LIST (comic_list), NULL);

        return comic_list->comic_manager;
}
