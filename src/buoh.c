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

#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <gconf/gconf-client.h>

#include "buoh.h"
#include "buoh-window.h"
#include "buoh-comic.h"
#include "buoh-comic-manager.h"
#include "buoh-comic-manager-date.h"

struct _BuohPrivate {
	BuohWindow   *window;
	GtkTreeModel *comic_list;
	gchar        *datadir;
	gchar        *proxy_uri;

	GConfClient  *gconf_client;
};

#define BUOH_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_BUOH, BuohPrivate))

#define GCONF_HTTP_PROXY_DIR "/system/http_proxy"
#define GCONF_USE_HTTP_PROXY "/system/http_proxy/use_http_proxy"
#define GCONF_HTTP_PROXY_HOST "/system/http_proxy/host"
#define GCONF_HTTP_PROXY_PORT "/system/http_proxy/port"
#define GCONF_HTTP_PROXY_USE_AUTHENTICATION "/system/http_proxy/use_authentication"
#define GCONF_HTTP_PROXY_AUTHENTICATION_USER "/system/http_proxy/authentication_user"
#define GCONF_HTTP_PROXY_AUTHENTICATION_PASSWORD "/system/http_proxy/authentication_password"

static void          buoh_init                   (Buoh         *buoh);
static void          buoh_class_init             (BuohClass    *klass);
static void          buoh_finalize               (GObject      *object);

static GList        *buoh_parse_selected         (Buoh         *buoh);
static GtkTreeModel *buoh_create_model_from_file (Buoh         *buoh);
static void          buoh_save_comic_list        (GtkTreeModel *model,
						  GtkTreePath  *arg1,
						  GtkTreeIter  *arg2,
						  gpointer      gdata);
static void          buoh_create_user_dir        (Buoh         *buoh);

G_DEFINE_TYPE (Buoh, buoh, G_TYPE_OBJECT)

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
		xmlFreeDoc (doc);
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
	xmlChar          *offset;
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

	if (!root) {
		xmlFreeDoc (doc);
		return NULL;
	}

	model = gtk_list_store_new (N_COLUMNS,
				    G_TYPE_BOOLEAN,
				    G_TYPE_STRING,
				    G_TYPE_STRING,
				    G_TYPE_STRING,
				    G_TYPE_OBJECT,
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
			
			/* Comic date */
			if (BUOH_IS_COMIC_MANAGER_DATE (comic_manager)) {
				
				first = xmlGetProp (node, (xmlChar *) "first");
				
				buoh_comic_manager_date_set_first (BUOH_COMIC_MANAGER_DATE (comic_manager),
								   (gchar *) first);
				g_free (first);
				
				offset = xmlGetProp (node, (xmlChar *) "offset");
				if (offset != NULL) {
					buoh_comic_manager_date_set_offset (BUOH_COMIC_MANAGER_DATE (comic_manager),
									    atoi ((gchar *) offset));
					g_free (offset);
				}
				
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
			g_object_unref (comic_manager);
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

static gboolean
save_comic_list (gpointer gdata)
{
	Buoh             *buoh = BUOH_BUOH (gdata);
	xmlTextWriterPtr  writer;
	gchar            *filename;
	GtkTreeModel     *filter;
	GtkTreeIter       iter;
	gboolean          valid;
	BuohComicManager *comic_manager;
	
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
		const gchar *id;
		
		gtk_tree_model_get (filter, &iter,
				    COMIC_LIST_COMIC_MANAGER,
				    &comic_manager, -1);
		id = buoh_comic_manager_get_id (comic_manager);
		g_object_unref (comic_manager);
		
		xmlTextWriterStartElement (writer, BAD_CAST "comic");
		xmlTextWriterWriteAttribute (writer,
					     BAD_CAST "id",
					     BAD_CAST id);
		xmlTextWriterEndElement (writer);
		
		valid = gtk_tree_model_iter_next (filter, &iter);
	}

	g_object_unref (filter);
	
	xmlTextWriterEndElement (writer);
	xmlTextWriterEndDocument (writer);
	xmlFreeTextWriter (writer);

	return FALSE;
}

static void
buoh_save_comic_list (GtkTreeModel *model,
		      GtkTreePath  *arg1,
		      GtkTreeIter  *arg2,
		      gpointer      gdata)
{
	g_idle_add ((GSourceFunc) save_comic_list, gdata);
}

static gboolean
buoh_create_comics_file (Buoh *buoh, const gchar *filename, const gchar *contents)
{
#if GTK_CHECK_VERSION(2,8,0)
	return g_file_set_contents (filename, contents, -1, NULL);
#else
	gint fd;

	if ((fd = open (filename, O_CREAT | O_WRONLY, 0644)) < 0) {
		return FALSE;
	}

	if (write (fd, contents, strlen (contents)) < 0) {
		close (fd);
		return FALSE;
	}

	if (close (fd) < 0) {
		return FALSE;
	}

	return TRUE;
#endif
}

static void
buoh_create_user_dir (Buoh *buoh)
{
	gchar       *filename;
	gchar       *cache_dir;
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
		if (!buoh_create_comics_file (buoh, filename, contents)) {
			g_free (filename);
			g_error ("Cannot create user's comics list file");
		}
	}
	
	g_free (filename);

	cache_dir = g_build_filename (buoh->priv->datadir, "cache", NULL);
	
	if (!g_file_test (cache_dir, G_FILE_TEST_IS_DIR)) {
		buoh_debug ("Cache directory doesn't exist, creating it ...");
		if (g_mkdir (cache_dir, 0755) != 0) {
			g_error ("Cannot create cache directory");
		}
	}
	
	g_free (cache_dir);
}

static gchar *
buoh_get_http_proxy_uri_from_gconf (Buoh *buoh)
{
	GConfClient *gconf_client = buoh->priv->gconf_client;
	gchar       *uri = NULL;

	if (gconf_client_get_bool (gconf_client, GCONF_USE_HTTP_PROXY, NULL)) {
		gchar *host = NULL;
		gchar *port = NULL;
		gchar *user = NULL;
		gchar *pass = NULL;
		guint  p = 0;
			
		host = gconf_client_get_string (gconf_client,
						GCONF_HTTP_PROXY_HOST,
						NULL);
		p = gconf_client_get_int (gconf_client,
					  GCONF_HTTP_PROXY_PORT,
					  NULL);
		if (p > 0)
			port = g_strdup_printf ("%d", p);
		
		if (gconf_client_get_bool (gconf_client, GCONF_HTTP_PROXY_USE_AUTHENTICATION, NULL)) {
			user = gconf_client_get_string (gconf_client,
							GCONF_HTTP_PROXY_AUTHENTICATION_USER,
							NULL);
			pass = gconf_client_get_string (gconf_client,
							GCONF_HTTP_PROXY_AUTHENTICATION_PASSWORD,
							NULL);

		}

		/* http://user:pass@host:port */
		uri = g_strdup_printf ("http://%s%s%s%s%s%s%s",
				       (user) ? user : "",
				       (user && pass) ? ":" : "",
				       (user && pass) ? pass : "",
				       (user) ? "@" : "",
				       host,
				       (port) ? ":" :  "",
				       (port) ? port : "");
	}

	return uri;
}

static void
buoh_update_http_proxy (GConfClient *gconf_client,
			guint        cnxn_id,
			GConfEntry  *entry,
			Buoh        *buoh)
{
	buoh_debug ("Proxy configuration changed");

	if (buoh->priv->proxy_uri)
		g_free (buoh->priv->proxy_uri);
	buoh->priv->proxy_uri = buoh_get_http_proxy_uri_from_gconf (buoh);
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

	buoh->priv->gconf_client = gconf_client_get_default ();
	gconf_client_add_dir (buoh->priv->gconf_client,
			      GCONF_HTTP_PROXY_DIR,
			      GCONF_CLIENT_PRELOAD_NONE,
			      NULL);
	gconf_client_notify_add (buoh->priv->gconf_client,
				 GCONF_HTTP_PROXY_DIR,
				 (GConfClientNotifyFunc)buoh_update_http_proxy,
				 (gpointer) buoh,
				 NULL, NULL);
}

static void
buoh_class_init (BuohClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (BuohPrivate));

	object_class->finalize = buoh_finalize;
}

static void
buoh_finalize (GObject *object)
{

	Buoh *buoh = BUOH_BUOH (object);

	buoh_debug ("buoh finalize");

	if (buoh->priv->datadir) {
		g_free (buoh->priv->datadir);
		buoh->priv->datadir = NULL;
	}

	if (buoh->priv->comic_list) {
		g_object_unref (buoh->priv->comic_list);
		buoh->priv->comic_list = NULL;
	}

	if (buoh->priv->proxy_uri) {
		g_free (buoh->priv->proxy_uri);
		buoh->priv->proxy_uri = NULL;
	}

	if (G_OBJECT_CLASS (buoh_parent_class)->finalize)
		(* G_OBJECT_CLASS (buoh_parent_class)->finalize) (object);
}

Buoh *
buoh_get_instance (void)
{
	static Buoh *buoh = NULL;

	if (!buoh) {
		buoh = g_object_new (BUOH_TYPE_BUOH, NULL);
	}

	return buoh;
}

Buoh *
buoh_new (void)
{
	return buoh_get_instance ();
}

void
buoh_exit_app (Buoh *buoh)
{
	g_return_if_fail (BUOH_IS_BUOH (buoh));
			  
	g_object_unref (buoh);
	
	gtk_main_quit ();

	buoh_debug ("buoh exit");
}

void
buoh_create_main_window (Buoh *buoh)
{
	g_return_if_fail (BUOH_IS_BUOH (buoh));
	
	if (buoh->priv->window) {
		gtk_window_present (GTK_WINDOW (buoh->priv->window));
	} else {
		buoh->priv->window = BUOH_WINDOW (buoh_window_new ());
	}
}

GtkTreeModel *
buoh_get_comics_model (Buoh *buoh)
{
	g_return_val_if_fail (BUOH_IS_BUOH (buoh), NULL);
	
	return buoh->priv->comic_list;
}

const gchar *
buoh_get_datadir (Buoh *buoh)
{
	g_return_val_if_fail (BUOH_IS_BUOH (buoh), NULL);
	
	return buoh->priv->datadir;
}

const gchar *
buoh_get_http_proxy_uri (Buoh *buoh)
{
	g_return_val_if_fail (BUOH_IS_BUOH (buoh), NULL);

	return buoh->priv->proxy_uri;
}
