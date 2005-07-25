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
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include "buoh.h"
#include "buoh-window.h"
#include "buoh-comic.h"
#include "comic-simple.h"

struct _BuohPrivate {
	BuohWindow   *window;
	GtkTreeModel *comic_list;
	gchar        *datadir;
};

#define BUOH_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_BUOH, BuohPrivate))

static GObjectClass *parent_class = NULL;

static void buoh_init                   (Buoh      *buoh);
static void buoh_class_init             (BuohClass *klass);
static void buoh_finalize               (GObject   *object);

static GList        *buoh_parse_selected         (Buoh *buoh);
static GtkTreeModel *buoh_create_model_from_file (Buoh *buoh);

GType
buoh_get_type ()
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (BuohClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) buoh_class_init,
			NULL,
			NULL,
			sizeof (Buoh),
			0,
			(GInstanceInitFunc) buoh_init
		};

		type = g_type_register_static (G_TYPE_OBJECT, "Buoh",
					       &info, 0);
	}

	return type;
}

static GList *
buoh_parse_selected (Buoh *buoh)
{
	GList      *list = NULL;
	xmlDocPtr   doc;
	xmlNodePtr  root;
	xmlNodePtr  node;
	gchar      *selected;
	gchar      *id;
	
	selected = g_build_filename (buoh->priv->datadir, "comics.xml", NULL);

	doc = xmlParseFile (selected);
	g_free (selected);

	if (!doc) {
		return NULL;
	}

	root = xmlDocGetRootElement (doc);

	if (!root) {
		return NULL;
	}

	node = root->xmlChildrenNode;

	while (node) {
		if (g_ascii_strcasecmp (node->name, "comic") == 0) {
			/* New comic */
			id = xmlGetProp (node, "id");
			list = g_list_append (list, g_strdup (id));
			g_free (id);
		}

		node = node->next;
	}

	xmlFreeDoc (doc);

	return list;
}

static GtkTreeModel *
buoh_create_model_from_file (Buoh *buoh)
{
	GtkListStore *model = NULL;
	GtkTreeIter   iter;
	xmlDocPtr     doc;
	xmlNodePtr    root;
	xmlNodePtr    node;
	xmlNodePtr    child;
	BuohComic    *comic;
	gchar        *id, *class, *title, *author, *uri;
	gboolean      visible;
	gchar        *restriction;
	GDateWeekday  restriction_date;
	gchar        *filename;
	GList        *selected = NULL;

	selected = buoh_parse_selected (buoh);

	filename = g_build_filename (COMICS_DIR, "comics.xml", NULL);

	doc = xmlParseFile (filename);
	g_free (filename);

	if (!doc) {
		return NULL;
	}

	root = xmlDocGetRootElement (doc);

	if (!root)
		return NULL;

	model = gtk_list_store_new (N_COLUMNS,
				    G_TYPE_BOOLEAN,
				    G_TYPE_STRING,
				    G_TYPE_STRING,
				    G_TYPE_POINTER,
				    -1);
	
	node = root->xmlChildrenNode;

	while (node) {
		if (g_ascii_strcasecmp (node->name, "comic") == 0) {
			/* New comic */
			class  = xmlGetProp (node, "class");

			/* Comic simple */
			if (g_ascii_strcasecmp (class, "simple") == 0) {
				id     = xmlGetProp (node, "id");
				title  = xmlGetProp (node, "title");
				author = xmlGetProp (node, "author");
				uri    = xmlGetProp (node, "generic_uri");

				comic = BUOH_COMIC (comic_simple_new_with_info (id, title, author, uri));

				/* Read the restrictions */
				child = node->children->next;
				while (child) {
					if (g_ascii_strcasecmp (child->name, "restrict") == 0) {
						restriction      = xmlNodeGetContent (child);
						restriction_date = atoi (restriction);

						comic_simple_set_restriction (COMIC_SIMPLE (comic),
									      restriction_date);
					}
					child = child->next;
				}

				/* Visible */
				if (g_list_find_custom (selected, id, (GCompareFunc)g_ascii_strcasecmp)) {
					visible = TRUE;
				} else {
					visible = FALSE;
				}
				
				gtk_list_store_append (model, &iter);
				gtk_list_store_set (model, &iter,
						    COMIC_LIST_VISIBLE, visible,
						    COMIC_LIST_TITLE, title,
						    COMIC_LIST_AUTHOR, author,
						    COMIC_LIST_COMIC, (gpointer) comic,
						    -1);

				g_free (id);
				g_free (title);
				g_free (author);
				g_free (uri);
			}

			g_free (class);

		}

		node = node->next;
	}

	xmlFreeDoc (doc);

	g_list_foreach (selected, (GFunc)g_free, NULL);
	g_list_free (selected);

	return GTK_TREE_MODEL (model);
}

static void
buoh_init (Buoh *buoh)
{
	buoh->priv = BUOH_GET_PRIVATE (buoh);

	buoh->priv->datadir = g_build_filename (g_get_home_dir (), ".buoh", NULL);
	buoh->priv->comic_list = buoh_create_model_from_file (buoh);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (buoh->priv->comic_list),
					      COMIC_LIST_TITLE, GTK_SORT_ASCENDING);
}

static void
buoh_class_init (BuohClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (BuohPrivate));

	object_class->finalize = buoh_finalize;
}

static void
buoh_finalize (GObject *object)
{

	Buoh *buoh = BUOH_BUOH (object);

	g_return_if_fail (BUOH_IS_BUOH (object));

	g_debug ("buoh finalize\n");

	/* TODO: save to disk selected comics */
	
	if (buoh->priv->datadir) {
		g_free (buoh->priv->datadir);
		buoh->priv->datadir = NULL;
	}

	if (buoh->priv->comic_list) {
		g_object_unref (buoh->priv->comic_list);
		buoh->priv->comic_list = NULL;
	}

	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

Buoh *
buoh_get_instance ()
{
	static Buoh *buoh = NULL;

	if (!buoh) {
		buoh = g_object_new (BUOH_TYPE_BUOH, NULL);
	}

	return buoh;
}

Buoh *
buoh_new ()
{
	return buoh_get_instance ();
}

void
buoh_exit_app (Buoh *buoh)
{
	g_object_unref (buoh);
	
	gtk_main_quit ();

	g_debug ("buoh exit\n");
}

void
buoh_create_main_window (Buoh *buoh)
{
	if (buoh->priv->window) {
		gtk_window_present (GTK_WINDOW (buoh->priv->window));
	} else {
		buoh->priv->window = BUOH_WINDOW (buoh_window_new ());
	}
}

GtkTreeModel *
buoh_get_comics_model (Buoh *buoh)
{
	return buoh->priv->comic_list;
}

