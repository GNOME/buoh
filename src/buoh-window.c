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
 *            Esteban Sanchez Mu�oz (steve-o) <esteban@steve-o.org>
 *            Carlos Garc�a Campos <carlosgc@gnome.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#include "buoh.h"
#include "buoh-window.h"
#include "buoh-properties-dialog.h"
#include "buoh-add-comic-dialog.h"
#include "buoh-view.h"
#include "buoh-comic-list.h"

struct _BuohWindowPrivate {
	GtkActionGroup *action_group;
	GtkUIManager   *ui_manager;

	BuohView       *view;
	BuohComicList  *comic_list;

	GtkWidget      *properties;
	GtkWidget      *add_dialog;
};

#define BUOH_WINDOW_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_WINDOW, BuohWindowPrivate))

#define GCONF_LOCKDOWN_SAVE "/desktop/gnome/lockdown/disable_save_to_disk"

static GtkWindowClass *parent_class = NULL;

static void buoh_window_init                        (BuohWindow *buoh_window);
static void buoh_window_class_init                  (BuohWindowClass *klass);
static void buoh_window_finalize                    (GObject *object);

/* Sensitivity */
static void buoh_window_set_sensitive                    (BuohWindow    *window,
							  const gchar   *name,
							  gboolean       sensitive);
static void buoh_window_comic_actions_set_sensitive      (BuohWindow    *window,
							  gboolean       sensitive);
static void buoh_window_comic_save_to_disk_set_sensitive (BuohWindow    *window,
							  gboolean       sensitive);

/* Callbacks */
static void buoh_window_view_status_change_cb           (GObject        *object,
							 GParamSpec     *arg,
							 gpointer        gdata);
static void buoh_window_view_zoom_change_cb             (BuohView       *view,
							 gpointer        gdata);
static gboolean buoh_window_comic_list_button_press_cb  (GtkWidget      *widget,
							 GdkEventButton *event,
							 gpointer        gdata);

/* Action callbacks */
static void buoh_window_cmd_comic_add                   (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_comic_remove                (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_comic_save_a_copy           (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_comic_copy_location         (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_comic_properties            (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_comic_quit                  (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_view_toolbar                (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_view_zoom_in                (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_view_zoom_out               (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_view_zoom_normal            (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_go_previous                 (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_go_next                     (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_go_first                    (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_go_last                     (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_help_contents               (GtkAction      *action,
							 gpointer        gdata);
static void buoh_window_cmd_help_about                  (GtkAction      *action,
							 gpointer        gdata);

static const GtkActionEntry menu_entries [] = {

	/* Top Level */
	{ "Comic", NULL, N_("_Comic") },
	{ "View", NULL, N_("_View") },
	{ "Go", NULL, N_("_Go") },
	{ "Help", NULL, N_("_Help") },

	/* Comic menu */
	{ "ComicAdd", GTK_STOCK_ADD, N_("_Add..."), "<control>A",
	  N_("Add a comic to the comic list"),
	  G_CALLBACK (buoh_window_cmd_comic_add) },
	{ "ComicRemove", GTK_STOCK_REMOVE, N_("_Remove"), "<control>R",
	  N_("Remove this comic from the comic list"),
	  G_CALLBACK (buoh_window_cmd_comic_remove) },
	{ "ComicSaveACopy", NULL, N_("_Save A Copy..."), NULL,
	  N_("Save the current comic with a new filename"),
	  G_CALLBACK (buoh_window_cmd_comic_save_a_copy) },
	{ "ComicCopyURI", NULL, N_("_Copy Location"), NULL,
	  N_("Copuy the location of this comic"),
	  G_CALLBACK (buoh_window_cmd_comic_copy_location) },
	{ "ComicProperties", GTK_STOCK_PROPERTIES, N_("_Properties..."), "<alt>Return",
	  N_("View the properties of this comic"),
	  G_CALLBACK (buoh_window_cmd_comic_properties) },
	{ "ComicQuit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
	  N_("Quit application"),
	  G_CALLBACK (buoh_window_cmd_comic_quit) },

	/* View menu*/
	{ "ViewZoomIn", GTK_STOCK_ZOOM_IN, N_("Zoom _In"), "<control>plus",
	  N_("Increase the comic size"),
	  G_CALLBACK (buoh_window_cmd_view_zoom_in) },
	{ "ViewZoomOut", GTK_STOCK_ZOOM_OUT, N_("Zoom _Out"), "<control>minus",
	  N_("Decrease the comic size"),
	  G_CALLBACK (buoh_window_cmd_view_zoom_out) },
	{ "ViewZoomNormal", GTK_STOCK_ZOOM_100, N_("_Normal Size"), "<control>0",
	  N_("Use the normal comic size"),
	  G_CALLBACK (buoh_window_cmd_view_zoom_normal) },

	/* Go menu */
	{ "GoPrevious", GTK_STOCK_GO_BACK, N_("_Previous Comic"), "Page_Up",
	  N_("Go to the previous comic"),
	  G_CALLBACK (buoh_window_cmd_go_previous) },
	{ "GoNext", GTK_STOCK_GO_FORWARD, N_("_Next Comic"), "Page_Down",
	  N_("Go to the next comic"),
	  G_CALLBACK (buoh_window_cmd_go_next) },
	{ "GoFirst", GTK_STOCK_GOTO_FIRST, N_("_First Comic"), "<control>Home",
	  N_("Go to the first comic"),
	  G_CALLBACK (buoh_window_cmd_go_first) },
	{ "GoLast", GTK_STOCK_GOTO_LAST, N_("_Last Comic"), "<control>End",
	  N_("Go to the last comic"),
	  G_CALLBACK (buoh_window_cmd_go_last) },

	/* Help menu */
	{ "HelpContents", GTK_STOCK_HELP, N_("_Contents"), "F1",
	  N_("Display help for the Buoh Comic Browser"),
	  G_CALLBACK (buoh_window_cmd_help_contents) },
	{ "HelpAbout", GTK_STOCK_ABOUT, N_("_About"), NULL,
	  N_("Display credits for the Buoh Comic Browser creators"),
	  G_CALLBACK (buoh_window_cmd_help_about) }
};

static const GtkToggleActionEntry menu_toggle_entries[] = {
	
	/* View menu*/
	{ "ViewToolbar", NULL, N_("_Toolbar"), NULL,
	  N_("Changes the visibility of the toolbar"),
	  G_CALLBACK (buoh_window_cmd_view_toolbar), TRUE }
};

GType
buoh_window_get_type ()
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (BuohWindowClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) buoh_window_class_init,
			NULL,
			NULL,
			sizeof (BuohWindow),
			0,
			(GInstanceInitFunc) buoh_window_init
		};

		type = g_type_register_static (GTK_TYPE_WINDOW, "BuohWindow",
					       &info, 0);
	}

	return type;
}

static void
buoh_window_init (BuohWindow *buoh_window)
{
	gchar          *icon_path;
	GtkWidget      *tree_view;
	GtkWidget      *vbox, *paned, *menubar;
	GtkWidget      *toolbar;
	GtkActionGroup *action_group;
	GtkAccelGroup  *accel_group;
	GError         *error = NULL;
      
	g_return_if_fail (BUOH_IS_WINDOW (buoh_window));

	buoh_window->priv = BUOH_WINDOW_GET_PRIVATE (buoh_window);

	gtk_window_set_title (GTK_WINDOW (buoh_window), _("Buoh Comics Reader"));
	icon_path = g_build_filename (PIXMAPS_DIR, "buoh16x16.png", NULL);
	gtk_window_set_icon_from_file (GTK_WINDOW (buoh_window), icon_path, NULL);
	g_free (icon_path);

	buoh_window->priv->properties = NULL;
	buoh_window->priv->add_dialog = NULL;

	vbox = gtk_vbox_new (FALSE, 0);

	/* Menu bar */
	action_group = gtk_action_group_new ("MenuActions");
	buoh_window->priv->action_group = action_group;
	gtk_action_group_set_translation_domain (action_group, NULL);
	gtk_action_group_add_actions (action_group, menu_entries,
				      G_N_ELEMENTS (menu_entries),
				      (gpointer) buoh_window);
	gtk_action_group_add_toggle_actions (action_group, menu_toggle_entries,
					     G_N_ELEMENTS (menu_toggle_entries),
					     (gpointer) buoh_window);
	
	buoh_window->priv->ui_manager = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (buoh_window->priv->ui_manager,
					    action_group, 0);

	accel_group = gtk_ui_manager_get_accel_group (buoh_window->priv->ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (buoh_window), accel_group);

	if (!gtk_ui_manager_add_ui_from_file (buoh_window->priv->ui_manager,
					      UI_DIR"/buoh-ui.xml",
					      &error)) {
		buoh_debug ("Could not merge buoh-ui.xml: %s", error->message);
		g_error_free (error);
	}

	/* Menu */
	menubar = gtk_ui_manager_get_widget (buoh_window->priv->ui_manager,
					     "/MainMenu");
	gtk_box_pack_start (GTK_BOX (vbox), menubar,
			    FALSE, FALSE, 0);
	gtk_widget_show (menubar);

	/* Toolbar */
	toolbar = gtk_ui_manager_get_widget (buoh_window->priv->ui_manager,
					     "/Toolbar");
	gtk_box_pack_start (GTK_BOX (vbox), toolbar,
			    FALSE, FALSE, 0);
	gtk_widget_show (toolbar);

	/* Pane */
	paned = gtk_hpaned_new ();
	gtk_paned_set_position (GTK_PANED (paned), 230);

	/* buoh view */
	buoh_window->priv->view = BUOH_VIEW (buoh_view_new ());
	g_signal_connect (G_OBJECT (buoh_window->priv->view), "notify::status",
			  G_CALLBACK (buoh_window_view_status_change_cb),
			  (gpointer) buoh_window);
	g_signal_connect (G_OBJECT (buoh_window->priv->view), "scale-changed",
			  G_CALLBACK (buoh_window_view_zoom_change_cb),
			  (gpointer) buoh_window);
	gtk_paned_pack2 (GTK_PANED (paned), GTK_WIDGET (buoh_window->priv->view),
			 TRUE, FALSE);
	gtk_widget_show (GTK_WIDGET (buoh_window->priv->view));

	/* buoh comic list */
	buoh_window->priv->comic_list = BUOH_COMIC_LIST (buoh_comic_list_new ());
	buoh_comic_list_set_view (buoh_window->priv->comic_list, buoh_window->priv->view);
	gtk_paned_pack1 (GTK_PANED (paned), GTK_WIDGET (buoh_window->priv->comic_list),
			 FALSE, TRUE);
	gtk_widget_show (GTK_WIDGET (buoh_window->priv->comic_list));
	
	gtk_box_pack_start (GTK_BOX (vbox), paned,
			    TRUE, TRUE, 0);
	gtk_widget_show (paned);

	gtk_container_add (GTK_CONTAINER (buoh_window), vbox);
	gtk_widget_show (vbox);

	tree_view = buoh_comic_list_get_list (buoh_window->priv->comic_list);
	g_signal_connect (G_OBJECT (tree_view), "button-press-event",
			  G_CALLBACK (buoh_window_comic_list_button_press_cb),
			  (gpointer) buoh_window);

	buoh_window_comic_actions_set_sensitive (buoh_window, FALSE);
	buoh_window_comic_save_to_disk_set_sensitive (buoh_window, FALSE);

	gtk_widget_grab_focus (GTK_WIDGET (buoh_window->priv->view));

	gtk_widget_show (GTK_WIDGET (buoh_window));
}

static void
buoh_window_class_init (BuohWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (BuohWindowPrivate));

	object_class->finalize = buoh_window_finalize;
}

static void
buoh_window_finalize (GObject *object)
{
	BuohWindow *buoh_window = BUOH_WINDOW (object);
	
	g_return_if_fail (BUOH_IS_WINDOW (object));

	buoh_debug ("buoh-window finalize");

	if (buoh_window->priv->ui_manager) {
		g_object_unref (buoh_window->priv->ui_manager);
		buoh_window->priv->ui_manager = NULL;
	}
	
	if (buoh_window->priv->action_group) {
		g_object_unref (buoh_window->priv->action_group);
		buoh_window->priv->action_group = NULL;
	}

	if (buoh_window->priv->properties) {
		g_object_unref (buoh_window->priv->properties);
		buoh_window->priv->properties = NULL;
	}

	if (buoh_window->priv->add_dialog) {
		g_object_unref (buoh_window->priv->add_dialog);
		buoh_window->priv->add_dialog = NULL;
	}

	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);

	buoh_exit_app (BUOH);
}

GtkWidget *
buoh_window_new (void)
{
	GtkWidget *buoh_window;

	buoh_window = GTK_WIDGET (g_object_new (BUOH_TYPE_WINDOW,
						"type", GTK_WINDOW_TOPLEVEL,
						"default-width", 600,
						"default-height", 300,
						NULL));
	return buoh_window;
}

static void
buoh_window_cmd_comic_add (GtkAction *action, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	if (!window->priv->add_dialog) {
		window->priv->add_dialog = buoh_add_comic_dialog_new ();
		g_object_add_weak_pointer (G_OBJECT (window->priv->add_dialog),
					   (gpointer *) &(window->priv->add_dialog));
		gtk_window_set_transient_for (GTK_WINDOW (window->priv->add_dialog),
					      GTK_WINDOW (window));
	}

	gtk_widget_show (window->priv->add_dialog);
}

static void
buoh_window_cmd_comic_remove (GtkAction *action, gpointer gdata)
{
	BuohWindow       *window = BUOH_WINDOW (gdata);
	GtkTreeModel     *model = buoh_get_comics_model (BUOH);
	GtkTreeIter       iter;
	BuohComicManager *cm, *current_cm;
	gchar            *current_cm_id, *cm_id;
	gboolean          valid;

	current_cm = buoh_comic_list_get_comic_manager (window->priv->comic_list);

	if (current_cm) {
		current_cm_id = buoh_comic_manager_get_id (current_cm);

		valid = gtk_tree_model_get_iter_first (model, &iter);

		while (valid) {
			gtk_tree_model_get (model, &iter,
					    COMIC_LIST_COMIC_MANAGER, &cm,
					    -1);
			cm_id = buoh_comic_manager_get_id (cm);

			if (g_ascii_strcasecmp (current_cm_id, cm_id) == 0) {
				buoh_comic_list_clear_selection (window->priv->comic_list);
				gtk_list_store_set (GTK_LIST_STORE (model), &iter,
						    COMIC_LIST_VISIBLE, FALSE,
						    -1);
				valid = FALSE;
			} else {
				valid = gtk_tree_model_iter_next (model, &iter);
			}

			g_free (cm_id);
		}

		g_free (current_cm_id);
	}
}

static void
buoh_window_cmd_comic_save_a_copy (GtkAction *action, gpointer gdata)
{
	GtkWidget        *chooser;
	GtkFileFilter    *filter;
	gchar            *suggested;
	gchar            *name;
	gchar            *page;
	gchar            *filename = NULL;
	static gchar     *folder = NULL;
	BuohWindow       *window = BUOH_WINDOW (gdata);
	BuohComic        *comic;
	BuohComicManager *cm;
	GdkPixbuf        *pixbuf;
	GtkWidget        *dialog;
	gboolean          successful;
	GError           *error;

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*.png");
	gtk_file_filter_set_name (filter, _("PNG Images"));

	chooser = gtk_file_chooser_dialog_new (_("Save comic"),
					       GTK_WINDOW (window),
					       GTK_FILE_CHOOSER_ACTION_SAVE,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					       NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (chooser), TRUE);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	if (folder) {
		gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (chooser),
							 folder);
	}

	comic     = buoh_view_get_comic (window->priv->view);
	cm        = buoh_comic_list_get_comic_manager (window->priv->comic_list);
	name      = buoh_comic_manager_get_title (cm);
	page      = buoh_comic_get_id (comic);
	suggested = g_strconcat (name, " (", page, ").png", NULL);
	pixbuf   = buoh_comic_get_pixbuf (comic);

	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (chooser),
					   suggested);

	do {
		if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT) {
			filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

			if (folder != NULL)
				g_free (folder);

			folder = gtk_file_chooser_get_current_folder_uri (GTK_FILE_CHOOSER (chooser));

			error = NULL;

			if (!gdk_pixbuf_save (pixbuf, filename, "png", &error, NULL)) {
				successful = FALSE;

				dialog = gtk_message_dialog_new (GTK_WINDOW (chooser),
								 GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
								 GTK_MESSAGE_ERROR,
								 GTK_BUTTONS_CLOSE,
								 _("Unable to save comic"));

				gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
									  error->message);
				gtk_dialog_run (GTK_DIALOG (dialog));

				gtk_widget_destroy (dialog);
				g_error_free (error);
			} else {
				successful = TRUE;
			}

			g_free (filename);
		} else {
			successful = TRUE;
		}
	} while (!successful);

	g_object_unref (pixbuf);
	g_free (name);
	g_free (page);
	g_free (suggested);
	gtk_widget_destroy (chooser);
}

static void
buoh_window_cmd_comic_copy_location (GtkAction *action, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);
	BuohComic  *comic = buoh_view_get_comic (window->priv->view);
	gchar      *uri;

	if (comic) {
		uri = buoh_comic_get_uri (comic);
		buoh_debug ("Copy %s to clipboard", uri);

		gtk_clipboard_set_text (gtk_clipboard_get (GDK_NONE), uri,
					g_utf8_strlen (uri, -1));

		gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_PRIMARY), uri,
					g_utf8_strlen (uri, -1));

		g_free (uri);
	}
}

static void
buoh_window_cmd_comic_properties (GtkAction *action, gpointer gdata)
{
	BuohWindow       *window = BUOH_WINDOW (gdata);
	BuohComicManager *cm = buoh_comic_list_get_comic_manager (window->priv->comic_list);

	if (cm) {
		if (!window->priv->properties) {
			window->priv->properties = buoh_properties_dialog_new ();
			buoh_properties_dialog_set_comic_manager (BUOH_PROPERTIES_DIALOG (window->priv->properties),
								  cm);
			g_object_add_weak_pointer (G_OBJECT (window->priv->properties),
						   (gpointer *) &(window->priv->properties));
			gtk_window_set_transient_for (GTK_WINDOW (window->priv->properties),
						      GTK_WINDOW (window));
		}

		gtk_widget_show (window->priv->properties);
	}
}

static void
buoh_window_cmd_comic_quit (GtkAction *action, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	gtk_widget_destroy (GTK_WIDGET (window));
}

static void
buoh_window_cmd_view_toolbar (GtkAction *action, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);
	GtkWidget  *toolbar;

	toolbar = gtk_ui_manager_get_widget (window->priv->ui_manager, "/Toolbar");
	g_object_set (G_OBJECT (toolbar), "visible",
		      gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)),
		      NULL);
	/* TODO: GConf stuff */
}

static void
buoh_window_cmd_view_zoom_in (GtkAction *action, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	buoh_view_zoom_in (window->priv->view);
}

static void
buoh_window_cmd_view_zoom_out (GtkAction *action, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	buoh_view_zoom_out (window->priv->view);
}

static void
buoh_window_cmd_view_zoom_normal (GtkAction *action, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	buoh_view_zoom_normal_size (window->priv->view);
}

static void
buoh_window_cmd_go_previous (GtkAction *action, gpointer gdata)
{
	BuohWindow       *window = BUOH_WINDOW (gdata);
	BuohComicManager *comic_manager;
	BuohComic        *comic;

	comic_manager = buoh_comic_list_get_comic_manager (window->priv->comic_list);

	comic = buoh_comic_manager_get_previous (comic_manager);

	buoh_view_set_comic (window->priv->view, comic);
}

static void
buoh_window_cmd_go_next (GtkAction *action, gpointer gdata)
{
	BuohWindow       *window = BUOH_WINDOW (gdata);
	BuohComicManager *comic_manager;
	BuohComic        *comic;

	comic_manager = buoh_comic_list_get_comic_manager (window->priv->comic_list);

	comic = buoh_comic_manager_get_next (comic_manager);

	buoh_view_set_comic (window->priv->view, comic);
}

static void
buoh_window_cmd_go_first (GtkAction *action, gpointer gdata)
{
	/* TODO */
}
static void
buoh_window_cmd_go_last (GtkAction *action, gpointer gdata)
{
	/* TODO */
}

static void
buoh_window_cmd_help_contents (GtkAction *action, gpointer gdata)
{
	/* TODO */
}

static void
buoh_window_cmd_help_about (GtkAction *action, gpointer gdata)
{
	BuohWindow         *window = BUOH_WINDOW (gdata);
	GdkPixbuf          *pixbuf;
	gchar              *pixbuf_path;
	static const gchar *authors[] = {
		"Esteban Sánchez Muñoz <esteban@steve-o.org>",
		"Pablo Arroyo Loma <zioma@linups.org>",
		"Carlos Garcia Campos <carlosgc@gnome.org>",
		                NULL
	};

	pixbuf_path = g_build_filename (PIXMAPS_DIR, "buoh64x64.png", NULL);
	pixbuf = gdk_pixbuf_new_from_file_at_size (pixbuf_path, 48, 48, NULL);
	g_free (pixbuf_path);

	gtk_show_about_dialog (GTK_WINDOW (window),
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

static void
buoh_window_set_sensitive (BuohWindow *window, const gchar *name, gboolean sensitive)
{
	GtkAction *action = gtk_action_group_get_action (window->priv->action_group,
							 name);
	gtk_action_set_sensitive (action, sensitive);
}

static void
buoh_window_comic_actions_set_sensitive (BuohWindow *window, gboolean sensitive)
{
	buoh_window_set_sensitive (window, "GoPrevious", sensitive);
	buoh_window_set_sensitive (window, "GoNext", sensitive);
	buoh_window_set_sensitive (window, "GoFirst", sensitive);
	buoh_window_set_sensitive (window, "GoLast", sensitive);
	buoh_window_set_sensitive (window, "ViewZoomIn",
				   buoh_view_is_max_zoom (window->priv->view) ?
				   FALSE : sensitive);
	buoh_window_set_sensitive (window, "ViewZoomOut",
				   buoh_view_is_min_zoom (window->priv->view) ?
				   FALSE : sensitive);
	buoh_window_set_sensitive (window, "ViewZoomNormal",
				   buoh_view_is_normal_size (window->priv->view) ?
				   FALSE : sensitive);
	buoh_window_set_sensitive (window, "ComicProperties", sensitive);
}

static void
buoh_window_comic_save_to_disk_set_sensitive (BuohWindow *window, gboolean sensitive)
{
	static GConfClient *client = NULL;
	gboolean            save_disabled = FALSE;

	if (!client) {
		client = gconf_client_get_default ();
	}

	if (gconf_client_get_bool (client, GCONF_LOCKDOWN_SAVE, NULL)) {
		save_disabled = TRUE;
	}

	buoh_window_set_sensitive (window, "ComicSaveACopy",
				   (save_disabled) ?  FALSE : sensitive);
}

static void
buoh_window_view_status_change_cb (GObject *object, GParamSpec *arg, gpointer gdata)
{
	BuohView   *view = BUOH_VIEW (object);
	BuohWindow *window = BUOH_WINDOW (gdata);
	BuohComic  *comic = NULL;

	switch (buoh_view_get_status (view)) {
	case STATE_MESSAGE_WELCOME:
	case STATE_MESSAGE_ERROR:
	case STATE_EMPTY:
		buoh_window_comic_actions_set_sensitive (window, FALSE);
		buoh_window_comic_save_to_disk_set_sensitive (window, FALSE);
		break;
	case STATE_COMIC_LOADING:
		comic = buoh_view_get_comic (view);
		buoh_window_comic_actions_set_sensitive (window,
							 (comic) ? TRUE : FALSE);
		buoh_window_comic_save_to_disk_set_sensitive (window, FALSE);
		break;
	case STATE_COMIC_LOADED:
		comic = buoh_view_get_comic (view);
		buoh_window_comic_actions_set_sensitive (window,
							 (comic) ? TRUE : FALSE);
		buoh_window_comic_save_to_disk_set_sensitive (window,
							      (comic) ? TRUE : FALSE);
		break;
	default:
		break;
	}

	gtk_widget_grab_focus (GTK_WIDGET (window->priv->view));
}

static void
buoh_window_view_zoom_change_cb (BuohView *view, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	buoh_window_set_sensitive (window, "ViewZoomIn",
				   !buoh_view_is_max_zoom (window->priv->view));
	buoh_window_set_sensitive (window, "ViewZoomOut",
				   !buoh_view_is_min_zoom (window->priv->view));
	buoh_window_set_sensitive (window, "ViewZoomNormal",
				   !buoh_view_is_normal_size (window->priv->view));

	gtk_widget_grab_focus (GTK_WIDGET (view));
}

static gboolean
buoh_window_comic_list_button_press_cb (GtkWidget *widget, GdkEventButton *event, gpointer gdata)
{
	BuohWindow       *window = BUOH_WINDOW (gdata);
	GtkWidget        *popup;
	GtkTreeSelection *selection;

	if (event->button == 3) {
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
		if (gtk_tree_selection_get_selected (selection, NULL, NULL)) {
			popup = gtk_ui_manager_get_widget (window->priv->ui_manager, "/ListPopup");
			
			gtk_menu_popup (GTK_MENU (popup), NULL, NULL, NULL,
					(gpointer) window,
					event->button, event->time);
			return TRUE;
		}
	}
	
	return FALSE;
}

