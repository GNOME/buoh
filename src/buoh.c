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
 *x
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Authors: Pablo Arroyo Loma (zioma) <zioma@linups.org>
 *          Esteban Sanchez Munoz (steve-o) <steve-o@linups.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgnomevfs/gnome-vfs.h>
#include <gnome.h>
#include <glade/glade.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gi18n-lib.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include "buoh.h"
#include "comic-simple.h"

#ifndef BUOH_CALLBACKS
#include "callbacks.h"
#endif

#define ZOOM_IN_FACTOR  1.5
#define ZOOM_OUT_FACTOR (1.0 / ZOOM_IN_FACTOR)

#define MIN_SCALE (ZOOM_OUT_FACTOR * ZOOM_OUT_FACTOR * ZOOM_OUT_FACTOR)
#define MAX_SCALE (ZOOM_IN_FACTOR * ZOOM_IN_FACTOR * ZOOM_IN_FACTOR)

#define BYTES_TO_PROCESS 1024

#define PARENT_TYPE G_TYPE_OBJECT

#define BUOH_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), TYPE_BUOH, BuohPrivate))

static void buoh_init         (Buoh *);
static void buoh_class_init   (BuohClass *);
static void buoh_finalize     (GObject *);

static void buoh_gui_properties_dialog_setup (Buoh *buoh);
static void buoh_gui_new_comic_dialog_setup (Buoh *buoh);

static void buoh_load_user_comic_list (Buoh *buoh);
static void buoh_load_supported_comic_list (Buoh *buoh);

static void buoh_gui_comic_previous_set_sensitive  (Buoh *buoh, gboolean sensitive);
static void buoh_gui_comic_next_set_sensitive      (Buoh *buoh, gboolean sensitive);
static void buoh_gui_toolbar_buttons_set_sensitive (Buoh *buoh, gboolean sensitive);
static void buoh_gui_zoom_in_set_sensitive         (Buoh *buoh, gboolean sensitive);
static void buoh_gui_zoom_out_set_sensitive        (Buoh *buoh, gboolean sensitive);
static void buoh_gui_normal_size_set_sensitive     (Buoh *buoh, gboolean sensitive);

static void buoh_comic_zoom_in_cb     (GtkWidget *widget, gpointer gdata);
static void buoh_comic_zoom_out_cb    (GtkWidget *widget, gpointer gdata);
static void buoh_comic_normal_size_cb (GtkWidget *widget, gpointer gdata);
static void buoh_comic_next_cb        (GtkWidget *widget, gpointer gdata);
static void buoh_comic_previous_cb    (GtkWidget *widget, gpointer gdata);

static void buoh_comic_menu_add_activate_cb (GtkWidget *widget,
					     gpointer gdata);

static void
buoh_gui_supported_comic_toggled_cb (GtkCellRendererToggle *cell_renderer,
				     gchar *path,
				     gpointer gdata);

static gboolean is_comic_of_user (GtkTreeModel *model,
			   GtkTreeIter *iter,
			   gpointer data);

static void loading_comic (GtkWidget *widget, gint x,
			   gint y, gint width,
			   gint height, gpointer *data);

static void buoh_gui_exit (GtkWidget *widget, gpointer *gdata);
static void buoh_gui_hide_window (GtkWidget *widget, gpointer *gdata);

static void buoh_gui_menu_about_activate (GtkWidget *widget, gpointer *gdata);
static void buoh_gui_show_comic_properties (GtkWidget *widget, gpointer *data);
static void buoh_gui_load_comic_from_treeview (GtkTreeSelection *selection,
					       gpointer *data);


/* Buoh Class */
static GObjectClass *parent_class = NULL;

typedef struct _BuohPrivate BuohPrivate;

enum {
	COMIC_USER_COLUMN,
	TITLE_COLUMN,
	AUTHOR_COLUMN,
	COMIC_COLUMN,
	N_COLUMNS
};

struct _BuohPrivate
{
	GladeXML *gui;
	Comic    *current_comic;
	gdouble   scale;
};

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

		type = g_type_register_static (PARENT_TYPE, "Buoh",
					       &info, 0);
	}

	return type;
}

static void
buoh_init (Buoh *buoh)
{
	BuohPrivate *private;
	   
	g_return_if_fail (IS_BUOH (buoh));

	private = BUOH_GET_PRIVATE (buoh);
	   
	private->gui = NULL;
	private->current_comic = NULL;
	private->scale = 1.0;
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
	BuohPrivate      *private;
	
	g_return_if_fail (IS_BUOH (object));

	private = BUOH_GET_PRIVATE (object);
	
	if (private->gui) {
		g_object_unref (private->gui);
		private->gui = NULL;
	}
					   
	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

Buoh *
buoh_new ()
{
	Buoh *buoh;

	buoh = g_object_new (TYPE_BUOH, NULL);

	return BUOH (buoh);
}

/* Connect the signals of properties window*/
static void
buoh_gui_properties_dialog_setup (Buoh *buoh)
{
	static gboolean connected = FALSE;
	GtkWidget       *widget;
	GtkWidget       *window;

	if (connected)
		return;

	window = buoh_get_widget (buoh, "comic_properties");

	gtk_widget_set_size_request (window, 250, 230);

	gtk_widget_hide_on_delete (window);
	   
	widget = buoh_get_widget (buoh, "properties_close");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_gui_hide_window), (gpointer) window);

	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);

	connected = TRUE;

	return;
}

/* Connect the signals of new comic dialog*/
static void
buoh_gui_new_comic_dialog_setup (Buoh *buoh)
{
	static gboolean connected = FALSE;
	GtkWidget    *window;
	BuohPrivate  *private;
	GtkWidget    *widget;

	if (connected)
		return;
	   
	g_return_if_fail (IS_BUOH (buoh));

	private = BUOH_GET_PRIVATE (buoh);
	   
	window = buoh_get_widget (buoh, "new_comic_dialog");
	   
	g_return_if_fail (GTK_IS_WINDOW (window));

	gtk_widget_hide_on_delete (window);
	   
	widget = glade_xml_get_widget (private->gui, "new_comic_okbutton");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_gui_new_dialog_ok_clicked), buoh);
	   
	widget = glade_xml_get_widget (private->gui, "new_comic_cancelbutton");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_gui_hide_window), (gpointer) window);

	connected = TRUE;
}

/* Connect the signals of add comic dialog*/
static void
buoh_gui_add_dialog_setup (Buoh *buoh)
{
	static gboolean connected = FALSE;
	GtkWidget    *window;
	BuohPrivate  *private;
	GtkWidget    *widget;

	if (connected)
		return;
	   
	g_return_if_fail (IS_BUOH (buoh));

	private = BUOH_GET_PRIVATE (buoh);

	window = buoh_get_widget (buoh, "add_comic_dialog");
	g_return_if_fail (GTK_IS_WINDOW (window));

	gtk_widget_set_size_request (window, 300, 300);
	   
	gtk_widget_hide_on_delete (window);
	   
	widget = glade_xml_get_widget (private->gui,
				       "add_comic_close_button");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_gui_hide_window), (gpointer) window);

	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
	
	connected = TRUE;
}

/* Connect the signals of the main window */
void
buoh_gui_setup (Buoh *buoh)
{
	static gboolean    connected = FALSE;
	GtkWidget          *window;
	GtkWidget          *widget;
	GtkWidget          *tree_view;
	gchar              *icon_path;
	BuohPrivate        *private;
	gchar              *glade_path;
	GtkListStore       *comic_list;
	GtkTreeSelection   *selection;

	if (connected)
		return;
	   
	g_return_if_fail (IS_BUOH (buoh));

	private = BUOH_GET_PRIVATE (buoh);

	glade_path = g_build_filename (INTERFACES_DIR, "buoh.glade", NULL);
	private->gui = glade_xml_new (glade_path, NULL, NULL);
	g_free (glade_path);

	g_return_if_fail (private->gui != NULL);

	/* GtkModel (User Comic list) definition */
	comic_list = gtk_list_store_new (N_COLUMNS,
					 G_TYPE_BOOLEAN,
					 G_TYPE_STRING,
					 G_TYPE_STRING,
					 G_TYPE_POINTER,
					 -1);

	/* Comic tree view attached to the Supported Comic list */
	tree_view = glade_xml_get_widget (private->gui, "supported_comic_list");
	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view),
				 GTK_TREE_MODEL (comic_list));
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (comic_list),
					      TITLE_COLUMN, GTK_SORT_ASCENDING);
	tree_view = glade_xml_get_widget (private->gui, "user_comic_list");
	   
	g_object_unref (comic_list);

	/* Hide the tabs on the notebook */
	widget = glade_xml_get_widget (private->gui, "notebook");
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (widget), FALSE);

	/* Signals connection  */
	window = glade_xml_get_widget (private->gui, "main_window");
	gtk_widget_set_size_request (window, 500, 300);

	icon_path = g_build_filename (PIXMAPS_DIR, "buoh16x16.png", NULL);
	gtk_window_set_icon_from_file (GTK_WINDOW (window), icon_path, NULL);
	g_free (icon_path);

	gtk_widget_hide_on_delete (window);
	g_signal_connect (G_OBJECT (window), "hide",
			  G_CALLBACK (buoh_gui_exit), (gpointer) buoh);
	   
	widget = glade_xml_get_widget (private->gui, "menu_quit");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_gui_exit), (gpointer) buoh);
	   
	widget = glade_xml_get_widget (private->gui, "menu_new");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_gui_new_activate), buoh);

	widget = glade_xml_get_widget (private->gui, "menu_add");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_comic_menu_add_activate_cb),
			  (gpointer) buoh);

	widget = glade_xml_get_widget (private->gui, "menu_zoom_in");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_comic_zoom_in_cb),
			  (gpointer) buoh);

	widget = glade_xml_get_widget (private->gui, "menu_zoom_out");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_comic_zoom_out_cb),
			  (gpointer) buoh);
	   
	widget = glade_xml_get_widget (private->gui, "menu_normal_size");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_comic_normal_size_cb),
			  (gpointer) buoh);

	widget = glade_xml_get_widget (private->gui, "menu_about");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_gui_menu_about_activate), NULL);

	/* Toolbar buttons */
	widget = glade_xml_get_widget (private->gui, "zoom_in_tb_button");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_comic_zoom_in_cb),
			  (gpointer) buoh);

	widget = glade_xml_get_widget (private->gui, "zoom_out_tb_button");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_comic_zoom_out_cb),
			  (gpointer) buoh);

	widget = glade_xml_get_widget (private->gui, "normal_size_tb_button");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_comic_normal_size_cb),
			  (gpointer) buoh);

	widget = glade_xml_get_widget (private->gui, "previous_tb_button");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_comic_previous_cb),
			  (gpointer) buoh);
	widget = glade_xml_get_widget (private->gui, "next_tb_button");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_comic_next_cb),
			  (gpointer) buoh);

	/* Selection of the list */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK (buoh_gui_load_comic_from_treeview),
			  (gpointer) buoh);

	/* Properties menu button */
	widget = glade_xml_get_widget (private->gui, "menu_properties");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_gui_show_comic_properties), buoh);

	/* New menu button */
	window = glade_xml_get_widget (private->gui, "new_comic_dialog");
	gtk_widget_hide_on_delete (window);
			 
	connected = TRUE;

	buoh_gui_new_comic_dialog_setup (buoh);
	buoh_gui_properties_dialog_setup (buoh);
	buoh_gui_add_dialog_setup (buoh);

	buoh_load_supported_comic_list (buoh);
	buoh_load_user_comic_list (buoh);

}

/* Read the comic list from user dir and load into the user comic list */
static void
buoh_load_user_comic_list (Buoh *buoh)
{
	BuohPrivate       *private;
	GtkTreeIter       iter;
	GtkWidget         *tree_view_user, *tree_view_supported;
	GtkTreeModel      *user_list, *supported_list;
	GtkCellRenderer   *renderer;
	GtkTreeViewColumn *column;
	xmlDocPtr         doc;
	xmlNodePtr        root;
	xmlNodePtr        node;
	gchar             *id, *user_comic_path, *user_dir_path;
	gchar             *id_supported;
	GnomeVFSHandle    *handle;
	GnomeVFSResult    result;
	gboolean          valid;
	Comic             *comic;
	GnomeVFSDirectoryHandle *dir_handle;
   
	g_return_if_fail (IS_BUOH (buoh));

	private = BUOH_GET_PRIVATE (buoh);
	   
	/* Configure the model (filter of supported list) */
	tree_view_supported = glade_xml_get_widget (private->gui, "supported_comic_list");
	tree_view_user      = glade_xml_get_widget (private->gui, "user_comic_list");
	   
	/* Get the supported list */
	supported_list = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view_supported));

	user_list = gtk_tree_model_filter_new (supported_list,
					       NULL);
	   
	gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (user_list),
						is_comic_of_user,
						NULL, NULL);
			 
	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view_user),
				 GTK_TREE_MODEL (user_list));
	   
	/* Configure the view*/
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Title"), renderer,
							   "text", TITLE_COLUMN,
							   NULL);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (tree_view_user),
				     column, TITLE_COLUMN);
	   
	/* Check the buoh user dir */
	user_dir_path = g_build_filename (g_get_home_dir(), ".buoh", NULL);

	result = gnome_vfs_directory_open (&dir_handle, user_dir_path,
					   GNOME_VFS_FILE_INFO_DEFAULT);

	if (result != GNOME_VFS_OK) {
		result = gnome_vfs_make_directory (user_dir_path, 0755);
		if (result != GNOME_VFS_OK) {
			g_error(_("Cannot create buoh directory %s!\n"), user_dir_path);
			return;
		}
	} else {
		gnome_vfs_directory_close (dir_handle);
	}
	   
	user_comic_path = g_build_filename (user_dir_path, "comics.xml", NULL);

	result = gnome_vfs_open (&handle, user_comic_path, GNOME_VFS_OPEN_READ);

	if (result != GNOME_VFS_OK) {
		gnome_vfs_create (&handle, user_comic_path,
				  GNOME_VFS_OPEN_WRITE | GNOME_VFS_OPEN_TRUNCATE, TRUE, 0644);
		gnome_vfs_write (handle, "<?xml version=\"1.0\"?><comic_list></comic_list>",
				 46, NULL);
	}

	gnome_vfs_close (handle);
	   
	doc = xmlParseFile (user_comic_path);

	g_free (user_dir_path);
	g_free (user_comic_path);
	   
	if (!doc)
		return;
	   
	root = xmlDocGetRootElement (doc);

	if (!root)
		return;

	node = root->xmlChildrenNode;

	while (node != NULL) {
		if (g_str_equal (node->name, "comic")) {
			/* New comic */
			id    = xmlGetProp (node, "id");
			valid = gtk_tree_model_get_iter_first (supported_list,
							       &iter);
			
			/* Search the comic in the supported list */
			while (valid) {
				gtk_tree_model_get (supported_list, &iter,
						    COMIC_COLUMN,
						    &comic,
						    -1);

				id_supported = comic_get_id (comic);

				if (g_str_equal (id_supported, id)) {
					gtk_list_store_set (GTK_LIST_STORE (supported_list),
							    &iter,
							    COMIC_USER_COLUMN,
							    TRUE,
							    -1);
					valid = FALSE;
				} else {
					valid = gtk_tree_model_iter_next (supported_list,
									  &iter);
				}
				g_free (id_supported);
			}

			g_free (id);
		}
		node = node->next;
	}

	xmlFreeDoc (doc);
				
	return;
}

/* Read the supported comic list from disk and load into the supported
   comic list */
static void
buoh_load_supported_comic_list (Buoh *buoh)
{
	ComicSimple       *comic = NULL;
	BuohPrivate       *private;
	GtkTreeIter       iter;
	GtkWidget         *tree_view;
	GtkListStore      *comic_list;
	GtkCellRenderer   *renderer;
	GtkTreeViewColumn *column;
	xmlDocPtr         doc;
	xmlNodePtr        root, node, child;
	gchar             *id, *class, *title, *author, *uri, *supported_comic_path;
	gchar             *restriction;
	GDateWeekday      restriction_date;
	
	g_return_if_fail (IS_BUOH (buoh));

	private = BUOH_GET_PRIVATE (buoh);

	/* Configure the tree view */
	tree_view = glade_xml_get_widget (private->gui, "supported_comic_list");


	renderer = gtk_cell_renderer_toggle_new ();
	column = gtk_tree_view_column_new_with_attributes ("", renderer,
							   "active", COMIC_USER_COLUMN,
							   NULL);
	g_object_set ((gpointer) renderer, "activatable", TRUE);
	   
	g_signal_connect (G_OBJECT (renderer), "toggled",
			  G_CALLBACK (buoh_gui_supported_comic_toggled_cb),
			  (gpointer) buoh);
	   
	gtk_tree_view_insert_column (GTK_TREE_VIEW (tree_view),
				     column, COMIC_USER_COLUMN);
	   
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Title"), renderer,
							   "text", TITLE_COLUMN,
							   NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (tree_view),
				     column, TITLE_COLUMN);
	   
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Author"), renderer,
							   "text", AUTHOR_COLUMN,
							   NULL);

	gtk_tree_view_insert_column (GTK_TREE_VIEW (tree_view),
				     column, AUTHOR_COLUMN);

	/* Get the model */
	comic_list = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view)));
	   
	supported_comic_path = g_build_filename (COMICS_DIR, "comics.xml", NULL);
	doc = xmlParseFile (supported_comic_path);
	g_free (supported_comic_path);
	   
	if (!doc) {
		g_print ("Error when loading supported comic list.\n");
		return;
	}
	   
	root = xmlDocGetRootElement (doc);

	if (!root)
		return;

	node = root->xmlChildrenNode;

	while (node != NULL) {
		if (g_str_equal (node->name, "comic")) {
			/* New comic */
			class  = xmlGetProp (node, "class");

			/* Comic simple */
			if (g_str_equal (class, "simple")) {
				id     = xmlGetProp (node, "id");
				title  = xmlGetProp (node, "title");
				author = xmlGetProp (node, "author");
				uri    = xmlGetProp (node, "generic_uri");

//				g_print ("%s - %s\n", title, author);

				comic = comic_simple_new_with_info (id, title, author, uri);

				/* Read the restrictions */
				child = node->children->next;
				while (child != NULL) {
					if (g_str_equal (child->name, "restrict")) {
						restriction      = xmlNodeGetContent (child);
						restriction_date = atoi (restriction);
						
						comic_simple_set_restriction (comic,
									      restriction_date);
					}
					child = child->next;
				}

				gtk_list_store_append (comic_list, &iter);
				gtk_list_store_set (comic_list, &iter,
						    COMIC_USER_COLUMN, FALSE,
						    TITLE_COLUMN, title,
						    AUTHOR_COLUMN, author,
						    COMIC_COLUMN, (gpointer) comic,
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

	return;
}


void
buoh_load_comic_simple (Buoh *buoh, gpointer *comic)
{
	BuohPrivate  *private;
	GtkTreeIter  iter;
	GtkWidget    *tree_view;
	GtkListStore *comic_list;

	g_return_if_fail (IS_COMIC_SIMPLE (comic));

	g_return_if_fail (IS_BUOH (buoh));
	private = BUOH_GET_PRIVATE (buoh);
	
	/* Get the tree view and the model */
	tree_view = glade_xml_get_widget (private->gui, "user_comic_list");
	comic_list = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view)));

	/* Add the comic */
	gtk_list_store_append (comic_list, &iter);

	gtk_list_store_set (comic_list, &iter,
			    TITLE_COLUMN, "prueba",
			    AUTHOR_COLUMN, "pruebaaa",
			    COMIC_COLUMN, comic,
			    -1);
	   
	return;
}

/* Return a widget of the glade file */
GtkWidget *
buoh_get_widget (Buoh *buoh, const gchar *widget)
{
	BuohPrivate *private;
	   	   
	g_return_val_if_fail (IS_BUOH (buoh), NULL);

	private = BUOH_GET_PRIVATE (buoh);

	return glade_xml_get_widget (private->gui, widget);
}

/* Show the main window */
void
buoh_gui_show (Buoh *buoh)
{
	GtkWidget   *window;
	GtkNotebook *notebook;
			 
	BuohPrivate *private;
	   
	g_return_if_fail (IS_BUOH (buoh));

	private = BUOH_GET_PRIVATE (buoh);

	window = glade_xml_get_widget (private->gui, "main_window");

	/* Show the tab of the wellcome message */
	notebook = GTK_NOTEBOOK (glade_xml_get_widget (private->gui, "notebook"));
	gtk_notebook_set_current_page (notebook, 1);

	buoh_gui_toolbar_buttons_set_sensitive (buoh, FALSE);

	gtk_widget_show (window);

	return;
}

/* Show the properties window */
static void 
buoh_gui_show_comic_properties (GtkWidget *widget, gpointer *data)
{
	GtkWidget        *properties_dialog, *widget1;
	Buoh             *buoh;
	Comic            *comic;
	gchar            *comic_title, *comic_author;
	   
	buoh = BUOH (data);

	comic = buoh_get_current_comic (buoh);
	
	/* If there is a selected comic from the lis	t */
	if (comic != NULL) {
		/* Get data of the selected comic */
		properties_dialog = buoh_get_widget (buoh, "comic_properties");

		widget1 = buoh_get_widget (buoh,
					   "properties_label_comic_author");
		comic_author = comic_get_author (comic);
		gtk_label_set_text (GTK_LABEL (widget1), comic_author);
		g_free (comic_author);
		
		widget1 = buoh_get_widget (buoh,
					   "properties_label_comic_title");
		comic_title = comic_get_title (comic);
		gtk_label_set_text (GTK_LABEL (widget1), comic_title);
		g_free (comic_title);
			 
		gtk_widget_show (properties_dialog);
	}
}

/* Set the comic that is displayed */
void
buoh_set_current_comic (Buoh *buoh, Comic *comic)
{
	BuohPrivate *private;
	GtkWidget   *image;
	GdkPixbuf   *pixbuf;

	g_return_if_fail (IS_BUOH (buoh));

	private = BUOH_GET_PRIVATE (buoh);

	image = glade_xml_get_widget (private->gui, "comic_image");

	private->current_comic = comic;
	
}

/* Get the comic that is displayed */
Comic *
buoh_get_current_comic (Buoh *buoh)
{
	BuohPrivate *private;

	g_return_val_if_fail (IS_BUOH (buoh), NULL);

	private = BUOH_GET_PRIVATE (buoh);

	return private->current_comic;

}

/* Show the current comic */
static void
buoh_comic_show (Buoh *buoh)
{
	BuohPrivate *private;
	GdkPixbuf   *pixbuf, *new_pixbuf;
	GtkWidget   *comic_image;

	g_return_if_fail (IS_BUOH (buoh));

	private = BUOH_GET_PRIVATE (buoh);

	comic_image = buoh_get_widget (buoh, "comic_image");
	
	pixbuf = comic_get_pixbuf (private->current_comic);
	
	new_pixbuf = gdk_pixbuf_scale_simple (pixbuf,
					      gdk_pixbuf_get_width (pixbuf) * private->scale,
					      gdk_pixbuf_get_height (pixbuf) * private->scale,
					      GDK_INTERP_BILINEAR);
	   
	gtk_image_set_from_pixbuf (GTK_IMAGE (comic_image), new_pixbuf);

	g_object_unref (new_pixbuf);

	gtk_widget_show (comic_image);
}

/* Callback attached to the pixbuf loader */
static void
loading_comic (GtkWidget *widget, gint x,
	       gint y, gint width,
	       gint height, gpointer *gdata)
{
	GtkImage         *comic_image;
	GdkPixbufLoader  *pixbuf_loader;
	   
	pixbuf_loader = GDK_PIXBUF_LOADER (widget);

	g_return_if_fail (GDK_IS_PIXBUF_LOADER (pixbuf_loader));
	   
	comic_image = GTK_IMAGE (gdata);

	g_return_if_fail (GTK_IS_IMAGE (comic_image));

	gtk_image_set_from_pixbuf (comic_image, gdk_pixbuf_loader_get_pixbuf (pixbuf_loader));
}

/* Set the sensitiveness of the zoom in button */
static void
buoh_gui_zoom_in_set_sensitive (Buoh *buoh, gboolean sensitive)
{
	GtkWidget *widget;
	   
	widget = buoh_get_widget (buoh, "zoom_in_tb_button");	   
	gtk_widget_set_sensitive (widget, sensitive);

	widget = buoh_get_widget (buoh, "menu_zoom_in");	   
	gtk_widget_set_sensitive (widget, sensitive);
}

/* Set the sensitiveness of the zoom out button */
static void
buoh_gui_zoom_out_set_sensitive (Buoh *buoh, gboolean sensitive)
{
	GtkWidget *widget;
	   
	widget = buoh_get_widget (buoh, "zoom_out_tb_button");	   
	gtk_widget_set_sensitive (widget, sensitive);

	widget = buoh_get_widget (buoh, "menu_zoom_out");	   
	gtk_widget_set_sensitive (widget, sensitive);

}

/* Set the sensitiveness of the zoom 1:1 button */
static void
buoh_gui_normal_size_set_sensitive (Buoh *buoh, gboolean sensitive)
{
	GtkWidget *widget;
	   
	widget = buoh_get_widget (buoh, "normal_size_tb_button");	   
	gtk_widget_set_sensitive (widget, sensitive);

	widget = buoh_get_widget (buoh, "menu_normal_size");	   
	gtk_widget_set_sensitive (widget, sensitive);
	   
}

/* Set the sensitiveness of the previous button */
static void
buoh_gui_comic_previous_set_sensitive (Buoh *buoh, gboolean sensitive)
{
	GtkWidget *widget;
	   
	widget = buoh_get_widget (buoh, "previous_tb_button");	   
	gtk_widget_set_sensitive (widget, sensitive);
	   
	widget = buoh_get_widget (buoh, "menu_previous");
	gtk_widget_set_sensitive (GTK_WIDGET (widget), sensitive);

	widget = buoh_get_widget (buoh, "menu_first");
	gtk_widget_set_sensitive (widget, sensitive);
	   
}

/* Set the sensitiveness of the next button */
static void
buoh_gui_comic_next_set_sensitive (Buoh *buoh, gboolean sensitive)
{
	GtkWidget *widget;
	   
	widget = buoh_get_widget (buoh, "next_tb_button");	   
	gtk_widget_set_sensitive (widget, sensitive);

	widget = buoh_get_widget (buoh, "menu_next");	   
	gtk_widget_set_sensitive (widget, sensitive);

	widget = buoh_get_widget (buoh, "menu_last");
	gtk_widget_set_sensitive (widget, sensitive);
}

static void
buoh_gui_toolbar_buttons_set_sensitive (Buoh *buoh, gboolean sensitive)
{
	buoh_gui_comic_next_set_sensitive (buoh, sensitive);
	buoh_gui_comic_previous_set_sensitive (buoh, sensitive);
	buoh_gui_normal_size_set_sensitive (buoh, sensitive);
	buoh_gui_zoom_out_set_sensitive (buoh, sensitive);
	buoh_gui_zoom_in_set_sensitive (buoh, sensitive);
}

/* Set the size of the main window */
/* FIXME: it should receive a Buoh object */
static void
buoh_window_resize (GtkWidget *window, GdkPixbuf *pixbuf)
{
	gint pb_width, pb_height;
	gint win_width, win_height;
	gint width, height;
	gint margin = 60; 

	/* FIXME: improve */
	   
	pb_width = gdk_pixbuf_get_width (pixbuf);
	pb_height = gdk_pixbuf_get_height (pixbuf);

	gtk_window_get_size (GTK_WINDOW (window),
			     &win_width, &win_height);

	width = win_width;
	height = win_height;

	if (win_width < pb_width)
		width = pb_width + margin;
	if (win_height < pb_height)
		height = pb_height + margin;
			 
	if ((width != win_width) || (height != win_height))
		gtk_window_resize (GTK_WINDOW (window), width, height);
}

/* Load a comic reading the file of the comic URI */
static gboolean
buoh_gui_load_comic (gpointer gdata)
{
	Buoh             *buoh;
	GnomeVFSHandle   *read_handle;
	GnomeVFSResult   result;
	GnomeVFSFileSize bytes_read;
	guint            buffer[BYTES_TO_PROCESS];
	GdkPixbufLoader  *pixbuf_loader;
	GtkImage         *comic_image;
	GtkWidget        *comic_view, *comic_list, *widget;
	GtkWidget        *window;
	gchar            *uri;
	Comic            *comic;
	GdkCursor        *cursor;
	gchar            *page;

	buoh = BUOH (gdata);

	comic = buoh_get_current_comic (buoh);

	buoh_set_current_comic (buoh, comic);

	uri = comic_get_uri (comic);
	if (!uri)
		return FALSE;

	/* Set the page number */
	page = comic_get_page (comic);

	widget = buoh_get_widget (buoh, "pager_tb_button");
	gtk_button_set_label (GTK_BUTTON (widget), page);
	gtk_widget_show (widget);
	
	/* Open the comic from URI */
	result = gnome_vfs_open (&read_handle, uri, GNOME_VFS_OPEN_READ);

	if (result != GNOME_VFS_OK) {
		g_print ("Error %d - %s\n", result,
			 gnome_vfs_result_to_string (result));
	} else {
		comic_view = buoh_get_widget (buoh, "comic_view");
		comic_list = buoh_get_widget (buoh, "user_comic_list");
		cursor = gdk_cursor_new (GDK_WATCH);

		if (!GTK_WIDGET_REALIZED (comic_view))
			gtk_widget_realize (GTK_WIDGET (comic_view));


		gdk_window_set_cursor (comic_view->window, cursor);
		gtk_widget_set_sensitive (comic_list, FALSE);
			 
		/* New pixbuf loader */
		pixbuf_loader = gdk_pixbuf_loader_new ();
		
		/* Connect callback to signal of pixbuf update */
		comic_image = GTK_IMAGE (buoh_get_widget (buoh, "comic_image"));
		g_signal_connect (G_OBJECT (pixbuf_loader), "area-updated",
				  G_CALLBACK (loading_comic),
				  (gpointer) comic_image);

		/* Read the file */
		result = gnome_vfs_read (read_handle, buffer,
					 BYTES_TO_PROCESS, &bytes_read);
		while ((result != GNOME_VFS_ERROR_EOF) &&
		       (result == GNOME_VFS_OK)) {
				    
			gdk_pixbuf_loader_write (pixbuf_loader,
						 (guchar *) buffer,
						 bytes_read, NULL);
			while (gtk_events_pending ())
				gtk_main_iteration ();

			result = gnome_vfs_read (read_handle, buffer,
						 BYTES_TO_PROCESS, &bytes_read);
		}

		if (result != GNOME_VFS_ERROR_EOF)
			g_print ("Error %d - %s\n", result,
				 gnome_vfs_result_to_string (result));

		window = buoh_get_widget (buoh, "main_window");

		buoh_window_resize (window, gdk_pixbuf_loader_get_pixbuf (pixbuf_loader));
			 
		gdk_pixbuf_loader_close (pixbuf_loader, NULL);
		gnome_vfs_close (read_handle);

		comic_set_pixbuf (comic, gdk_pixbuf_loader_get_pixbuf (pixbuf_loader));

		gtk_widget_set_sensitive (comic_list, TRUE);
		gdk_cursor_unref (cursor);
		gdk_window_set_cursor (comic_view->window, NULL);

		g_object_unref (pixbuf_loader);

		buoh_gui_toolbar_buttons_set_sensitive (buoh, TRUE);
		buoh_gui_normal_size_set_sensitive (buoh, FALSE);
		buoh_gui_comic_next_set_sensitive (buoh, (comic_is_the_last (comic)));

		buoh_comic_show (buoh);
	}

	g_free (page);
	g_free (uri);

	return FALSE;
}


/* Callback attached to user comic list */
static void
buoh_gui_load_comic_from_treeview (GtkTreeSelection *selection, gpointer *data)
{
	GtkTreeModel     *comic_list;
	GtkTreeIter      iter;
	GtkNotebook      *notebook;
	gchar            *selected_title;
	GObject          *pointer;
	Buoh             *buoh;
	   
	/* When a comic is selected from the list */
	if (gtk_tree_selection_get_selected (selection, &comic_list, &iter)) {
		buoh = BUOH (data);

		/* Show the tab of the image */
		notebook = GTK_NOTEBOOK (buoh_get_widget (buoh, "notebook"));
			 
		gtk_notebook_set_current_page (notebook, 0);
			 
		/* Get data of the selected comic */
		gtk_tree_model_get (comic_list, &iter,
				    TITLE_COLUMN, &selected_title,
				    COMIC_COLUMN, &pointer,
				    -1);

		buoh_set_current_comic (buoh, COMIC (pointer));

		g_idle_add (buoh_gui_load_comic, (gpointer) buoh); 
	}
}

/* Function that update the zoom of the GtkImage */
static void
buoh_comic_zoom (Buoh *buoh, double factor, gboolean relative)
{
	BuohPrivate *private;
	double       scale;

	g_return_if_fail (IS_BUOH (buoh));

	private = BUOH_GET_PRIVATE (buoh);
	   
	if (relative)
		scale = private->scale * factor;
	else
		scale = factor;

	private->scale = CLAMP (scale, MIN_SCALE, MAX_SCALE);
	   
	if ((float) private->scale == 1.0)
		buoh_gui_normal_size_set_sensitive (buoh, FALSE);
	else
		buoh_gui_normal_size_set_sensitive (buoh, TRUE);
	   
	if (private->scale == MIN_SCALE)
		buoh_gui_zoom_out_set_sensitive (buoh, FALSE);
	else
		buoh_gui_zoom_out_set_sensitive (buoh, TRUE);

	if (private->scale == MAX_SCALE)
		buoh_gui_zoom_in_set_sensitive (buoh, FALSE);
	else
		buoh_gui_zoom_in_set_sensitive (buoh, TRUE);
	   
	buoh_comic_show (buoh);
}

/* Zoom in */
void 
buoh_comic_zoom_in (Buoh *buoh)
{
	buoh_comic_zoom (buoh, ZOOM_IN_FACTOR, TRUE);
}

/* Zoom out */
void
buoh_comic_zoom_out (Buoh *buoh)
{
	buoh_comic_zoom (buoh, ZOOM_OUT_FACTOR, TRUE);
}

/* Zoom 1:1 */
void
buoh_comic_normal_size (Buoh *buoh)
{
	buoh_comic_zoom (buoh, 1.0, FALSE);
}

/* Callback attached to zoom in button */
static void
buoh_comic_zoom_in_cb  (GtkWidget *widget, gpointer gdata)
{
	buoh_comic_zoom_in (BUOH (gdata));
}

/* Callback attached to zoom out button */
static void
buoh_comic_zoom_out_cb (GtkWidget *widget, gpointer gdata)
{
	buoh_comic_zoom_out (BUOH (gdata));
}

/* Callback attached to zoom 1:1 button */
static void
buoh_comic_normal_size_cb (GtkWidget *widget, gpointer gdata)
{
	buoh_comic_normal_size (BUOH (gdata));
}

/* Callback attached to previous button */
static void
buoh_comic_previous_cb (GtkWidget *widget, gpointer gdata)
{
	Buoh        *buoh;
	BuohPrivate *private;
	Comic       *comic;

	buoh = BUOH (gdata);
	g_return_if_fail (IS_BUOH (buoh));
	private = BUOH_GET_PRIVATE (buoh);

	comic = buoh_get_current_comic (buoh);
	comic_go_previous (comic);

	buoh_gui_load_comic ((gpointer) buoh);
}

/* Callback attached to next button */
static void
buoh_comic_next_cb (GtkWidget *widget, gpointer gdata)
{
	Buoh        *buoh;
	BuohPrivate *private;
	Comic       *comic;

	buoh = BUOH (gdata);
	g_return_if_fail (IS_BUOH (buoh));
	private = BUOH_GET_PRIVATE (buoh);

	comic = buoh_get_current_comic (buoh);
	comic_go_next (comic);

	buoh_gui_load_comic ((gpointer) buoh);
}

/* Callback attached to add menu button */
static void
buoh_comic_menu_add_activate_cb (GtkWidget *widget, gpointer gdata)
{
	Buoh            *buoh;
	GtkWidget       *window;

	buoh = BUOH (gdata);
	g_return_if_fail (IS_BUOH (buoh));

	window = buoh_get_widget (buoh, "add_comic_dialog");
	gtk_widget_show (window);
}

/* Set the visible comics in the comic filter */
static gboolean
is_comic_of_user (GtkTreeModel *model,
		  GtkTreeIter *iter,
		  gpointer data)
{
	gboolean visible;
	   
	gtk_tree_model_get (model, iter, COMIC_USER_COLUMN, &visible, -1);

	return visible;
}

/* Callback attached to the toggle widget of the supported comic list */
static void
buoh_gui_supported_comic_toggled_cb (GtkCellRendererToggle *cell_renderer,
				     gchar *path,
				     gpointer gdata)
{
	Buoh         *buoh;
	GtkTreeModel *comic_list;
	GtkTreeView  *tree_view;
	GtkTreeIter  iter;
	gboolean     new_status;
	   
	buoh = BUOH (gdata);

	tree_view = GTK_TREE_VIEW (buoh_get_widget (buoh, "supported_comic_list"));
	comic_list = gtk_tree_view_get_model (tree_view);
	gtk_tree_model_get_iter_from_string (comic_list, &iter, path);
	
	new_status = ! gtk_cell_renderer_toggle_get_active (cell_renderer);
	   
	gtk_list_store_set (GTK_LIST_STORE (comic_list),
			    &iter,
			    COMIC_USER_COLUMN, new_status,
			    -1);
	return;
}

/* Exit the program */
static void
buoh_gui_exit (GtkWidget *widget, gpointer *gdata)
{
	Buoh             *buoh;
	BuohPrivate      *private;
	gchar            *id_comic, *user_comic_path;
	GtkWidget        *tree_view_user;
	GtkTreeModel     *user_list;
	GtkTreeIter      iter;
	gboolean         valid;
	Comic            *comic;
	xmlTextWriterPtr writer;

	buoh = BUOH (gdata);
	
	g_return_if_fail (IS_BUOH (buoh));

	private = BUOH_GET_PRIVATE (buoh);
	
	tree_view_user = glade_xml_get_widget (private->gui, "user_comic_list");
	user_list = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view_user));

	/* Save the user comic list to a XML file on the user dir */
	user_comic_path = g_build_filename (g_get_home_dir(), ".buoh/comics.xml", NULL);

	writer = xmlNewTextWriterFilename (user_comic_path, 0);

	g_free (user_comic_path);
	
	xmlTextWriterStartDocument (writer, NULL, NULL, NULL);
	xmlTextWriterStartElement (writer, BAD_CAST "comic_list");
	
	valid = gtk_tree_model_get_iter_first (user_list, &iter);
	
	while (valid) {
		gtk_tree_model_get (user_list, &iter,
				    COMIC_COLUMN,
				    &comic,
				    -1);

		id_comic = comic_get_id (comic);
		
		xmlTextWriterStartElement(writer, BAD_CAST "comic");
		xmlTextWriterWriteAttribute(writer, BAD_CAST "id",
					    BAD_CAST id_comic);
		xmlTextWriterEndElement(writer);
		
		valid = gtk_tree_model_iter_next (user_list,
						  &iter);
		
		g_free (id_comic);
	}
	
	xmlTextWriterEndElement (writer);
	xmlTextWriterEndDocument (writer);
	xmlFreeTextWriter (writer);
	
	gtk_main_quit ();
}

/* Hide the window passed in gdata or in widget*/
static void
buoh_gui_hide_window (GtkWidget *widget, gpointer *gdata)
{
	if (GTK_IS_WIDGET (gdata))
		gtk_widget_hide (GTK_WIDGET (gdata));
	else
		gtk_widget_hide (GTK_WIDGET (widget));
}

/* Callback when the about button is activated */
static void
buoh_gui_menu_about_activate (GtkWidget *widget, gpointer *gdata)
{
	GdkPixbuf          *pixbuf;
	gchar              *pixbuf_path;
	static const gchar *authors[] = {
		"Esteban Sánchez Muñoz <steve-o@linups.org>",
		"Pablo Arroyo Loma <zioma@linups.org>",
		NULL
	};
	   
	pixbuf_path = g_build_filename (PIXMAPS_DIR, "buoh64x64.png", NULL);
	pixbuf = gdk_pixbuf_new_from_file_at_size (pixbuf_path, 64, 64, NULL);
	g_free (pixbuf_path);

	gtk_show_about_dialog (NULL,
			       "name", _("Buoh comics reader"),
			       "version", VERSION,
			       "copyright", "Copyright \xC2\xA9 2004 Esteban Sánchez Muñoz - Pablo Arroyo Loma",
			       "authors", authors,
			       "translator-credits", _("translator-credits"),
			       "logo", pixbuf,
			       NULL);

	if (pixbuf)
		g_object_unref (pixbuf);
}
