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
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include "buoh.h"
#include "buoh-window.h"
#include "buoh-comic.h"
#include "buoh-comic-manager.h"
#include "buoh-comic-manager-date.h"

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

static GList        *buoh_parse_selected         (Buoh         *buoh);
static GtkTreeModel *buoh_create_model_from_file (Buoh         *buoh);
static void          buoh_save_comic_list        (GtkTreeModel *model,
						  GtkTreePath  *arg1,
						  GtkTreeIter  *arg2,
						  gpointer      gdata);
static void          buoh_create_user_dir        (Buoh         *buoh);

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

void
buoh_debug (const gchar *format, ...)
{
#ifdef GNOME_ENABLE_DEBUG
	va_list  args;
	gchar   *string;

	g_return_if_fail (format != NULL);
	
	va_start (args, format);
	string = g_strdup_vprintf (format, args);
	va_end (args);
	
	g_debug (string);

	g_free (string);
#endif
	return;
}

static GList *
buoh_parse_selected (Buoh *buoh)
{
	GList      *list = NULL;
	xmlDocPtr   doc;
	xmlNodePtr  root;
	xmlNodePtr  node;
	gchar      *selected;
	xmlChar    *id;
	
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
		if (g_ascii_strcasecmp ((const gchar*) node->name, "comic") == 0) {
			/* New comic */
			id = xmlGetProp (node, (xmlChar *) "id");
			list = g_list_append (list, g_strdup ((gchar *)id));
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
	GtkListStore     *model = NULL;
	GtkTreeIter       iter;
	xmlDocPtr         doc;
	xmlNodePtr        root;
	xmlNodePtr        node;
	xmlNodePtr        child;
	BuohComicManager *comic_manager;
	xmlChar          *id, *class, *title, *author, *language, *uri, *first;
	gboolean          visible;
	xmlChar          *restriction;
	GDateWeekday      restriction_date;
	gchar            *filename;
	GList            *selected = NULL;

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
				    G_TYPE_STRING,
				    G_TYPE_POINTER,
				    -1);
	
	node = root->xmlChildrenNode;

	while (node) {
		if (g_ascii_strcasecmp ((const gchar *)node->name, "comic") == 0) {
			/* New comic */
			class    = xmlGetProp (node, (xmlChar *) "class");
			id       = xmlGetProp (node, (xmlChar *) "id");
			title    = xmlGetProp (node, (xmlChar *) "title");
			author   = xmlGetProp (node, (xmlChar *) "author");
			language = xmlGetProp (node, (xmlChar *) "language");
			uri      = xmlGetProp (node, (xmlChar *) "generic_uri");
			
			comic_manager = buoh_comic_manager_new ((gchar *)class,
								(gchar *)id,
								(gchar *)title,
								(gchar *)author,
								(gchar *)language,
								(gchar *)uri);
			
			/* Comic simple */
			if (BUOH_IS_COMIC_MANAGER_DATE (comic_manager)) {
				
				first = xmlGetProp (node, (xmlChar *) "first");
				
				buoh_comic_manager_date_set_first (BUOH_COMIC_MANAGER_DATE (comic_manager),
								   (gchar *) first);
				g_free (first);

				/* Read the restrictions */
				child = node->children->next;
				while (child) {
					if (g_ascii_strcasecmp ((const gchar *) child->name, "restrict") == 0) {
								
						restriction      = xmlNodeGetContent (child);
						restriction_date = atoi ((gchar *)restriction);

						buoh_comic_manager_date_set_restriction (BUOH_COMIC_MANAGER_DATE (comic_manager),
									     		 restriction_date);
						g_free (restriction);
					}
					child = child->next;
				}
				
			}
			
			/* Visible */
			if (selected &&
			    g_list_find_custom (selected, id,
						(GCompareFunc)g_ascii_strcasecmp)) {
				visible = TRUE;
			} else {
				visible = FALSE;
			}
			
			gtk_list_store_append (model, &iter);
			gtk_list_store_set (model, &iter,
					    COMIC_LIST_VISIBLE, visible,
					    COMIC_LIST_TITLE, title,
					    COMIC_LIST_AUTHOR, author,
					    COMIC_LIST_LANGUAGE, language,
					    COMIC_LIST_COMIC_MANAGER, (gpointer) comic_manager,
					    -1);
			
			g_free (id);
			g_free (title);
			g_free (author);
			g_free (language);
			g_free (uri);
			g_free (class);

		}

		node = node->next;
	}

	xmlFreeDoc (doc);

	if (selected) {
		g_list_foreach (selected, (GFunc)g_free, NULL);
		g_list_free (selected);
	}

	return GTK_TREE_MODEL (model);
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
buoh_save_comic_list (GtkTreeModel *model,
		      GtkTreePath  *arg1,
		      GtkTreeIter  *arg2,
		      gpointer      gdata)
{
	Buoh             *buoh = BUOH_BUOH (gdata);
	xmlTextWriterPtr  writer;
	gchar            *filename;
	GtkTreeModel     *filter;
	GtkTreeIter       iter;
	gboolean          valid;
	BuohComicManager *comic_manager;
	gchar            *id;
	
	buoh_debug ("Buoh comic model changed");

	filter = gtk_tree_model_filter_new (buoh->priv->comic_list, NULL);
	gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filter),
						buoh_comic_list_visible,
						NULL, NULL);
	
	filename = g_build_filename (buoh->priv->datadir, "comics.xml", NULL);
	writer = xmlNewTextWriterFilename (filename, 0);
	g_free (filename);

	xmlTextWriterStartDocument (writer, NULL, NULL, NULL);
	xmlTextWriterStartElement (writer, BAD_CAST "comic_list");

	valid = gtk_tree_model_get_iter_first (filter, &iter);
	while (valid) {
		gtk_tree_model_get (filter, &iter,
				    COMIC_LIST_COMIC_MANAGER,
				    &comic_manager, -1);
		id = buoh_comic_manager_get_id (comic_manager);
		
		xmlTextWriterStartElement (writer, BAD_CAST "comic");
		xmlTextWriterWriteAttribute (writer,
					     BAD_CAST "id",
					     BAD_CAST id);
		xmlTextWriterEndElement (writer);
		
		g_free (id);

		valid = gtk_tree_model_iter_next (filter, &iter);
	}

	g_object_unref (filter);
	
	xmlTextWriterEndElement (writer);
	xmlTextWriterEndDocument (writer);
	xmlFreeTextWriter (writer);
}

static void
buoh_create_user_dir (Buoh *buoh)
{
	gchar       *filename;
	const gchar *contents = "<?xml version=\"1.0\"?>\n<comic_list>\n</comic_list>\n";

	if (!g_file_test (buoh->priv->datadir, G_FILE_TEST_IS_DIR)) {
		buoh_debug ("User directory doesn't exist, creating it ...");
		if (g_mkdir (buoh->priv->datadir, 0755) != 0) {
			g_error ("Cannot create user's directory");
		}
	}
	
	filename = g_build_filename (buoh->priv->datadir, "comics.xml", NULL);

	if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {
		buoh_debug ("User comics file doesn't exist, creating it ...");
		if (!g_file_set_contents (filename, contents, -1, NULL)) {
			g_free (filename);
			g_error ("Cannot create user's comics list file");
		}
	}

	g_free (filename);
}

static void
buoh_init (Buoh *buoh)
{
	buoh->priv = BUOH_GET_PRIVATE (buoh);

	buoh->priv->datadir = g_build_filename (g_get_home_dir (), ".buoh", NULL);
	buoh_create_user_dir (buoh);
	
	buoh->priv->comic_list = buoh_create_model_from_file (buoh);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (buoh->priv->comic_list),
					      COMIC_LIST_TITLE, GTK_SORT_ASCENDING);
	g_signal_connect (G_OBJECT (buoh->priv->comic_list), "row-changed",
			  G_CALLBACK (buoh_save_comic_list),
			  (gpointer) buoh);
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

	buoh_debug ("buoh finalize");

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

	buoh_debug ("buoh exit");
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

