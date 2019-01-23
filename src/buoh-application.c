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

#include "buoh-application.h"
#include "buoh-window.h"
#include "buoh-comic.h"
#include "buoh-comic-manager.h"
#include "buoh-comic-manager-date.h"

struct _BuohApplication {
        GtkApplication parent;

        GtkTreeModel  *comic_list;
        gchar         *datadir;
};

static void          buoh_application_init                   (BuohApplication         *buoh);
static void          buoh_application_class_init             (BuohApplicationClass    *klass);
static void          buoh_application_finalize               (GObject                 *object);

static GList        *buoh_application_parse_selected         (BuohApplication         *buoh);
static GtkTreeModel *buoh_application_create_model_from_file (BuohApplication         *buoh);
static void          buoh_application_save_comic_list        (GtkTreeModel            *model,
                                                              GtkTreePath             *arg1,
                                                              GtkTreeIter             *arg2,
                                                              gpointer                 gdata);
static void          buoh_application_create_user_dir        (BuohApplication         *buoh);

G_DEFINE_TYPE (BuohApplication, buoh_application, GTK_TYPE_APPLICATION)

void
buoh_debug (const gchar *format, ...)
{
#if !defined(NDEBUG)
        va_list  args;
        gchar   *string;

        g_return_if_fail (format != NULL);

        va_start (args, format);
        string = g_strdup_vprintf (format, args);
        va_end (args);

        g_debug ("%s", string);

        g_free (string);
#endif
        return;
}

static GList *
buoh_application_parse_selected (BuohApplication *buoh)
{
        GList      *list = NULL;
        xmlDocPtr   doc;
        xmlNodePtr  root;
        xmlNodePtr  node;
        gchar      *selected;
        xmlChar    *id;

        selected = g_build_filename (buoh->datadir, "comics.xml", NULL);

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
buoh_application_create_model_from_file (BuohApplication *buoh)
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

        selected = buoh_application_parse_selected (buoh);

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
                                child = node->children;
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
        BuohApplication  *buoh = BUOH_APPLICATION (gdata);
        xmlTextWriterPtr  writer;
        gchar            *filename;
        GtkTreeModel     *filter;
        GtkTreeIter       iter;
        gboolean          valid;
        BuohComicManager *comic_manager;

        buoh_debug ("Buoh comic model changed");

        filter = gtk_tree_model_filter_new (buoh->comic_list, NULL);
        gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filter),
                                                buoh_comic_list_visible,
                                                NULL, NULL);

        filename = g_build_filename (buoh->datadir, "comics.xml", NULL);
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
buoh_application_save_comic_list (GtkTreeModel *model,
                      GtkTreePath  *arg1,
                      GtkTreeIter  *arg2,
                      gpointer      gdata)
{
        g_idle_add ((GSourceFunc) save_comic_list, gdata);
}

static gboolean
buoh_application_create_comics_file (BuohApplication *buoh, const gchar *filename, const gchar *contents)
{
        return g_file_set_contents (filename, contents, -1, NULL);
}

static void
buoh_application_create_user_dir (BuohApplication *buoh)
{
        gchar       *filename;
        gchar       *cache_dir;
        const gchar *contents = "<?xml version=\"1.0\"?>\n<comic_list>\n</comic_list>\n";

        if (!g_file_test (buoh->datadir, G_FILE_TEST_IS_DIR)) {
                buoh_debug ("User directory doesn't exist, creating it ...");
                if (g_mkdir (buoh->datadir, 0755) != 0) {
                        g_error ("Cannot create user's directory");
                }
        }

        filename = g_build_filename (buoh->datadir, "comics.xml", NULL);

        if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {
                buoh_debug ("User comics file doesn't exist, creating it ...");
                if (!buoh_application_create_comics_file (buoh, filename, contents)) {
                        g_free (filename);
                        g_error ("Cannot create user's comics list file");
                }
        }

        g_free (filename);

        cache_dir = g_build_filename (buoh->datadir, "cache", NULL);

        if (!g_file_test (cache_dir, G_FILE_TEST_IS_DIR)) {
                buoh_debug ("Cache directory doesn't exist, creating it ...");
                if (g_mkdir (cache_dir, 0755) != 0) {
                        g_error ("Cannot create cache directory");
                }
        }

        g_free (cache_dir);
}

static void
buoh_application_init (BuohApplication *buoh)
{
        buoh->datadir = g_build_filename (g_get_home_dir (), ".buoh", NULL);
        buoh_application_create_user_dir (buoh);

        buoh->comic_list = buoh_application_create_model_from_file (buoh);
        gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (buoh->comic_list),
                                              COMIC_LIST_TITLE, GTK_SORT_ASCENDING);
        g_signal_connect (G_OBJECT (buoh->comic_list), "row-changed",
                          G_CALLBACK (buoh_application_save_comic_list),
                          (gpointer) buoh);
}

static void
buoh_application_class_init (BuohApplicationClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

        object_class->finalize = buoh_application_finalize;

        app_class->activate = buoh_application_activate;
        app_class->startup = buoh_application_startup;
}

static void
buoh_application_finalize (GObject *object)
{

        BuohApplication *buoh = BUOH_APPLICATION (object);

        buoh_debug ("buoh finalize");

        if (buoh->datadir) {
                g_free (buoh->datadir);
                buoh->datadir = NULL;
        }

        if (buoh->comic_list) {
                g_object_unref (buoh->comic_list);
                buoh->comic_list = NULL;
        }

        if (G_OBJECT_CLASS (buoh_application_parent_class)->finalize) {
                (* G_OBJECT_CLASS (buoh_application_parent_class)->finalize) (object);
        }
}

BuohApplication *
buoh_application_get_instance (void)
{
        static BuohApplication *buoh = NULL;

        if (!buoh) {
                buoh = g_object_new (BUOH_TYPE_APPLICATION,
                                     "application-id", "org.gnome.buoh",
                                     NULL);
        }

        return buoh;
}

BuohApplication *
buoh_application_new (void)
{
        return buoh_application_get_instance ();
}

void
buoh_application_activate (GApplication *buoh)
{
        g_return_if_fail (BUOH_IS_APPLICATION (buoh));

        GList *list;
        GtkWidget *window;

        list = gtk_application_get_windows (GTK_APPLICATION (buoh));

        if (list) {
                gtk_window_present (GTK_WINDOW (list->data));
        } else {
                window = buoh_window_new ();
                gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (buoh));
                gtk_widget_show (window);
        }
}

void
buoh_application_startup (GApplication *app)
{
        const gchar *comic_properties_accel[2] = { "<alt>Return", NULL };
        const gchar *comic_quit_accel[2] = { "<control>Q", NULL };
        const gchar *view_zoom_in_accel[2] = { "<control>plus", NULL };
        const gchar *view_zoom_out_accel[2] = { "<control>minus", NULL };
        const gchar *view_zoom_normal_accel[2] = { "<control>0", NULL };
        const gchar *go_previous_accel[2] = { "<alt>Left", NULL };
        const gchar *go_next_accel[2] = { "<alt>Right", NULL };
        const gchar *go_first_accel[2] = { "<control>Home", NULL };
        const gchar *go_last_accel[2] = { "<control>End", NULL };

        G_APPLICATION_CLASS (buoh_application_parent_class)->startup (app);

        gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.comic-properties", comic_properties_accel);
        gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.comic-quit", comic_quit_accel);
        gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.view-zoom-in", view_zoom_in_accel);
        gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.view-zoom-out", view_zoom_out_accel);
        gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.view-zoom-normal", view_zoom_normal_accel);
        gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.go-previous", go_previous_accel);
        gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.go-next", go_next_accel);
        gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.go-first", go_first_accel);
        gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.go-last", go_last_accel);
}

GtkTreeModel *
buoh_application_get_comics_model (BuohApplication *buoh)
{
        g_return_val_if_fail (BUOH_IS_APPLICATION (buoh), NULL);

        return buoh->comic_list;
}

const gchar *
buoh_application_get_datadir (BuohApplication *buoh)
{
        g_return_val_if_fail (BUOH_IS_APPLICATION (buoh), NULL);

        return buoh->datadir;
}
