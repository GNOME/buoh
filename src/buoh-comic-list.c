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

#include "buoh.h"
#include "buoh-comic-list.h"

struct _BuohComicListPrivate {
	GtkWidget    *swindow;
	GtkWidget    *tree_view;
	GtkTreeModel *model;
	
	BuohView     *view;
};

#define BUOH_COMIC_LIST_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_COMIC_LIST, BuohComicListPrivate))

static GtkFrameClass *parent_class = NULL;

static void buoh_comic_list_init                   (BuohComicList *buoh_comic_list);
static void buoh_comic_list_class_init             (BuohComicListClass *klass);
static void buoh_comic_list_finalize               (GObject *object);

static void buoh_comic_list_size_request           (GtkWidget        *widget,
						    GtkRequisition   *requisition);
static void buoh_comic_list_size_allocate          (GtkWidget        *widget,
						    GtkAllocation    *allocation);

static void     buoh_comic_list_selection_changed  (GtkTreeSelection *selection,
						    gpointer          gdata);
static gboolean buoh_comic_list_visible            (GtkTreeModel     *model,
						    GtkTreeIter      *iter,
						    gpointer          gdata);

GType
buoh_comic_list_get_type ()
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (BuohComicListClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) buoh_comic_list_class_init,
			NULL,
			NULL,
			sizeof (BuohComicList),
			0,
			(GInstanceInitFunc) buoh_comic_list_init
		};

		type = g_type_register_static (GTK_TYPE_FRAME, "BuohComicList",
					       &info, 0);
	}

	return type;
}

static void
buoh_comic_list_selection_changed (GtkTreeSelection *selection, gpointer gdata)
{
	BuohComicList *comic_list = BUOH_COMIC_LIST (gdata);
	GtkTreeModel  *model;
	GtkTreeIter    iter;
	GObject       *comic;

	if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
		gtk_tree_model_get (model, &iter,
				    COMIC_LIST_COMIC, &comic,
				    -1);
		
		buoh_view_set_comic (comic_list->priv->view,
				     BUOH_COMIC (comic));
		g_debug ("selection changed: set comic");
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
	GtkWidget         *align;
	GtkWidget         *label;
	gchar             *text_label;
	
	g_return_if_fail (BUOH_IS_COMIC_LIST (buoh_comic_list));

	buoh_comic_list->priv = BUOH_COMIC_LIST_GET_PRIVATE (buoh_comic_list);

	label = gtk_label_new (NULL);
	
	
	buoh_comic_list->priv->view = NULL;
	
	text_label = g_strdup_printf ("<b>%s</b>", _("Comic List"));
	gtk_label_set_markup (GTK_LABEL (label), text_label);
	g_free (text_label);

	gtk_frame_set_label_widget (GTK_FRAME (buoh_comic_list), label);
	gtk_widget_show (label);

	model = buoh_get_comics_model (BUOH);
	buoh_comic_list->priv->model = gtk_tree_model_filter_new (model, NULL);
	gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (buoh_comic_list->priv->model),
						buoh_comic_list_visible,
						NULL, NULL);
					       
	buoh_comic_list->priv->tree_view = gtk_tree_view_new_with_model (buoh_comic_list->priv->model);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Title"), renderer,
							   "text", COMIC_LIST_TITLE,
							   NULL);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (buoh_comic_list->priv->tree_view),
				     column, COMIC_LIST_TITLE);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (buoh_comic_list->priv->tree_view));
	g_signal_connect_after (selection, "changed",
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

	align = gtk_alignment_new (0.50, 0.50, 1.0, 1.0);
	gtk_container_set_border_width (GTK_CONTAINER (align), 6);
	gtk_container_add (GTK_CONTAINER (align), buoh_comic_list->priv->swindow);
	gtk_widget_show (buoh_comic_list->priv->swindow);
	
	gtk_container_add (GTK_CONTAINER (buoh_comic_list), align);
	gtk_widget_show (align);

	gtk_widget_show (GTK_WIDGET (buoh_comic_list));
}

static void
buoh_comic_list_class_init (BuohComicListClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (BuohComicListPrivate));

	widget_class->size_request = buoh_comic_list_size_request;
	widget_class->size_allocate = buoh_comic_list_size_allocate;

	object_class->finalize = buoh_comic_list_finalize;
}

static void
buoh_comic_list_finalize (GObject *object)
{
	BuohComicList *comic_list = BUOH_COMIC_LIST (object);
	
	g_return_if_fail (BUOH_IS_COMIC_LIST (object));

	g_debug ("comic-list finalize\n");

	if (comic_list->priv->model) {
		g_object_unref (comic_list->priv->model);
		comic_list->priv->model = NULL;
	}

	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
buoh_comic_list_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	(* GTK_WIDGET_CLASS (parent_class)->size_request) (widget, requisition);

	/* we need some extra size */
	requisition->width += 100;
}

static void
buoh_comic_list_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);

	/* we need some extra size */
	allocation->width += 100;
}

GtkWidget *
buoh_comic_list_new (void)
{
	GtkWidget *buoh_comic_list;

	buoh_comic_list = GTK_WIDGET (g_object_new (BUOH_TYPE_COMIC_LIST,
						    "border-width", 6,
						    "shadow-type", GTK_SHADOW_NONE,
						    NULL));
	return buoh_comic_list;
}

void
buoh_comic_list_set_view (BuohComicList *comic_list, BuohView *view)
{
	g_return_if_fail (BUOH_IS_VIEW (view));

	if (comic_list->priv->view) {
		return;
	}

	comic_list->priv->view = view;
}

GtkWidget *
buoh_comic_list_get_list (BuohComicList *comic_list)
{
	return comic_list->priv->tree_view;
}

void
buoh_comic_list_clear_selection (BuohComicList *comic_list)
{
	gtk_tree_selection_unselect_all (
		gtk_tree_view_get_selection (GTK_TREE_VIEW (comic_list->priv->tree_view)));
}

/*Comic *
buoh_comic_list_get_selection (BuohComicList *comic_list)
{
	Comic            *comic;
	GtkTreeModel     *model;
	GtkTreeIter       iter;
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (comic_list->priv->tree_view));
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (comic_list->priv->tree_view));
	if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
		gtk_tree_model_get (user_comic_list, &iter,
				    COMIC_LIST_COMIC, &comic,
				    -1);
		return comic;
	}

	return NULL;
}*/
