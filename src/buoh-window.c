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
#include <glade/glade.h>
#include <gconf/gconf-client.h>

#include "buoh.h"
#include "buoh-window.h"
#include "buoh-properties-dialog.h"
#include "buoh-add-comic-dialog.h"
#include "buoh-view.h"
#include "buoh-comic-list.h"

struct _BuohWindowPrivate {
	GladeXML      *gui;

	BuohView      *view;
	BuohComicList *comic_list;

	GtkWidget     *properties;
	GtkWidget     *add_dialog;
	GtkUIManager  *popup_ui;
};

#define BUOH_WINDOW_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_WINDOW, BuohWindowPrivate))

#define GCONF_LOCKDOWN_SAVE "/desktop/gnome/lockdown/disable_save_to_disk"

static GtkWindowClass *parent_class = NULL;

static void buoh_window_init                        (BuohWindow *buoh_window);
static void buoh_window_class_init                  (BuohWindowClass *klass);
static void buoh_window_finalize                    (GObject *object);

/* Callbacks */
static void buoh_window_menu_quit_cb                    (GtkMenuItem    *menuitem,
							 gpointer        gdata);
static void buoh_window_menu_add_cb                     (GtkMenuItem    *menuitem,
							 gpointer        gdata);
static void buoh_window_menu_save_cb                    (GtkMenuItem    *menuitem,
							 gpointer        gdata);
static void buoh_window_menu_properties_cb              (GtkMenuItem    *menuitem,
							 gpointer        gdata);
static void buoh_window_menu_zoom_in_cb                 (GtkMenuItem    *menuitem,
							 gpointer        gdata);
static void buoh_window_menu_zoom_out_cb                (GtkMenuItem    *menuitem,
							 gpointer        gdata);
static void buoh_window_menu_normal_size_cb             (GtkMenuItem    *menuitem,
							 gpointer        gdata);
static void buoh_window_menu_about_cb                   (GtkMenuItem    *menuitem,
							 gpointer        gdata);
static void buoh_window_toolbar_zoom_in_cb              (GtkToolButton  *toolbutton,
							 gpointer        gdata);
static void buoh_window_toolbar_zoom_out_cb             (GtkToolButton  *toolbutton,
							 gpointer        gdata);
static void buoh_window_toolbar_normal_size_cb          (GtkToolButton  *toolbutton,
							 gpointer        gdata);
static void buoh_window_toolbar_previous_cb             (GtkToolButton  *toolbutton,
							 gpointer        gdata);
static void buoh_window_toolbar_next_cb                 (GtkToolButton  *toolbutton,
							 gpointer        gdata);
static void buoh_window_view_status_change_cb           (GObject        *object,
							 GParamSpec     *arg,
							 gpointer        gdata);
static void buoh_window_view_zoom_change_cb             (BuohView       *view,
							 gpointer        gdata);
static gboolean buoh_window_comic_list_button_press_cb  (GtkWidget      *widget,
							 GdkEventButton *event,
							 gpointer        gdata);
static void     buoh_window_popup_properties_cb         (GtkWidget      *widget,
							 gpointer        gdata);
static void     buoh_window_popup_delete_cb             (GtkWidget      *widget,
							 gpointer        gdata);
static void     buoh_window_popup_copy_uri_cb           (GtkWidget      *widget,
							 gpointer        gdata);

/* Sensitivity */
static void buoh_window_set_sensitive                   (BuohWindow     *window,
							 const gchar    *name,
							 gboolean        sensitive);
static void buoh_window_previous_set_sensitive           (BuohWindow    *window,
							  gboolean       sensitive);
static void buoh_window_next_set_sensitive               (BuohWindow    *window,
							  gboolean       sensitive);
static void buoh_window_zoom_in_set_sensitive            (BuohWindow    *window,
							  gboolean       sensitive);
static void buoh_window_zoom_out_set_sensitive           (BuohWindow    *window,
							  gboolean       sensitive);
static void buoh_window_normal_size_set_sensitive        (BuohWindow    *window,
							  gboolean       sensitive);
static void buoh_window_comic_actions_set_sensitive      (BuohWindow    *window,
							  gboolean       sensitive);
static void buoh_window_comic_save_to_disk_set_sensitive (BuohWindow    *window,
							  gboolean       sensitive);

/* popup menu values */
static GtkActionEntry popup_menu_items [] = {
	{ "Properties",  GTK_STOCK_PROPERTIES, N_("_Properties"),
	  NULL, NULL, G_CALLBACK (buoh_window_popup_properties_cb) },
	{ "CopyURI",     GTK_STOCK_COPY,       N_("_Copy comic URI"),
	  NULL, NULL, G_CALLBACK (buoh_window_popup_copy_uri_cb) },
	{ "Delete",      GTK_STOCK_DELETE,     N_("_Delete"),
	  NULL, NULL, G_CALLBACK (buoh_window_popup_delete_cb) }
};

static const gchar *popup_ui_description =
"<ui>"
"  <popup name='MainMenu'>"
"    <menuitem action='Properties'/>"
"    <menuitem action='CopyURI'/>"
"    <separator/>"
"    <menuitem action='Delete'/>"
"  </popup>"
"</ui>";

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
	gchar          *glade_path, *icon_path;
	GtkWidget      *widget, *parent, *tree_view;
	GtkActionGroup *action_group;
      
	g_return_if_fail (BUOH_IS_WINDOW (buoh_window));

	buoh_window->priv = BUOH_WINDOW_GET_PRIVATE (buoh_window);

	glade_path = g_build_filename (INTERFACES_DIR, "buoh.glade", NULL);
	buoh_window->priv->gui = glade_xml_new (glade_path, NULL, NULL);
	g_free (glade_path);

	gtk_window_set_title (GTK_WINDOW (buoh_window), _("Buoh Comics Reader"));
	icon_path = g_build_filename (PIXMAPS_DIR, "buoh16x16.png", NULL);
	gtk_window_set_icon_from_file (GTK_WINDOW (buoh_window), icon_path, NULL);
	g_free (icon_path);

	/* Reparent window */
	widget = glade_xml_get_widget (buoh_window->priv->gui, "main_box");
	parent = glade_xml_get_widget (buoh_window->priv->gui, "main_window");
	gtk_widget_ref (widget);
	gtk_container_remove (GTK_CONTAINER (parent), widget);
	gtk_container_add (GTK_CONTAINER (buoh_window), widget);
	gtk_widget_unref (widget);
	gtk_widget_show_all (widget);

	buoh_window->priv->properties = NULL;
	buoh_window->priv->add_dialog = NULL;
	
	widget = glade_xml_get_widget (buoh_window->priv->gui, "paned");
	gtk_paned_set_position (GTK_PANED (widget), 230);

	/* buoh view */
	buoh_window->priv->view = BUOH_VIEW (buoh_view_new ());
	g_signal_connect (G_OBJECT (buoh_window->priv->view), "notify::status",
			  G_CALLBACK (buoh_window_view_status_change_cb),
			  (gpointer) buoh_window);
	g_signal_connect (G_OBJECT (buoh_window->priv->view), "scale-changed",
			  G_CALLBACK (buoh_window_view_zoom_change_cb),
			  (gpointer) buoh_window);
	gtk_paned_pack2 (GTK_PANED (widget), GTK_WIDGET (buoh_window->priv->view),
			 TRUE, FALSE);
	gtk_widget_show (GTK_WIDGET (buoh_window->priv->view));

	/* buoh comic list */
	buoh_window->priv->comic_list = BUOH_COMIC_LIST (buoh_comic_list_new ());
	buoh_comic_list_set_view (buoh_window->priv->comic_list, buoh_window->priv->view);
	gtk_paned_pack1 (GTK_PANED (widget), GTK_WIDGET (buoh_window->priv->comic_list),
			 FALSE, TRUE);
	gtk_widget_show (GTK_WIDGET (buoh_window->priv->comic_list));

	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_add_actions (action_group, popup_menu_items,
				      G_N_ELEMENTS (popup_menu_items),
				      (gpointer) buoh_window);
	buoh_window->priv->popup_ui = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (buoh_window->priv->popup_ui,
					    action_group, 0);
	gtk_ui_manager_add_ui_from_string (buoh_window->priv->popup_ui,
					   popup_ui_description, -1, NULL);
	tree_view = buoh_comic_list_get_list (buoh_window->priv->comic_list);
	g_signal_connect (G_OBJECT (tree_view), "button-press-event",
			  G_CALLBACK (buoh_window_comic_list_button_press_cb),
			  (gpointer) buoh_window);

	/* Callbacks */
	/* Menu Items */
	widget = glade_xml_get_widget (buoh_window->priv->gui, "menu_quit");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_window_menu_quit_cb),
			  (gpointer) buoh_window);
	widget = glade_xml_get_widget (buoh_window->priv->gui, "menu_add");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_window_menu_add_cb),
			  (gpointer) buoh_window);
	widget = glade_xml_get_widget (buoh_window->priv->gui, "menu_save");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_window_menu_save_cb),
			  (gpointer) buoh_window);
	widget = glade_xml_get_widget (buoh_window->priv->gui, "menu_properties");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_window_menu_properties_cb),
			  (gpointer) buoh_window);
	widget = glade_xml_get_widget (buoh_window->priv->gui, "menu_zoom_in");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_window_menu_zoom_in_cb),
			  (gpointer) buoh_window);
	widget = glade_xml_get_widget (buoh_window->priv->gui, "menu_zoom_out");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_window_menu_zoom_out_cb),
			  (gpointer) buoh_window);
	widget = glade_xml_get_widget (buoh_window->priv->gui, "menu_normal_size");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_window_menu_normal_size_cb),
			  (gpointer) buoh_window);
	widget = glade_xml_get_widget (buoh_window->priv->gui, "menu_about");
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (buoh_window_menu_about_cb),
			  (gpointer) buoh_window);

	/* Toolbar buttons */
	widget = glade_xml_get_widget (buoh_window->priv->gui, "zoom_in_tb_button");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_window_toolbar_zoom_in_cb),
			  (gpointer) buoh_window);

	widget = glade_xml_get_widget (buoh_window->priv->gui, "zoom_out_tb_button");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_window_toolbar_zoom_out_cb),
			  (gpointer) buoh_window);

	widget = glade_xml_get_widget (buoh_window->priv->gui, "normal_size_tb_button");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_window_toolbar_normal_size_cb),
			  (gpointer) buoh_window);

	widget = glade_xml_get_widget (buoh_window->priv->gui, "previous_tb_button");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_window_toolbar_previous_cb),
			  (gpointer) buoh_window);
	widget = glade_xml_get_widget (buoh_window->priv->gui, "next_tb_button");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (buoh_window_toolbar_next_cb),
			  (gpointer) buoh_window);

	buoh_window_comic_actions_set_sensitive (buoh_window, FALSE);
	buoh_window_comic_save_to_disk_set_sensitive (buoh_window, FALSE);
	
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

	g_debug ("buoh-window finalize\n");

	if (buoh_window->priv->gui) {
		g_object_unref (buoh_window->priv->gui);
		buoh_window->priv->gui = NULL;
	}

	if (buoh_window->priv->popup_ui) {
		g_object_unref (buoh_window->priv->popup_ui);
		buoh_window->priv->popup_ui = NULL;
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
buoh_window_menu_quit_cb (GtkMenuItem *menuitem, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	gtk_widget_destroy (GTK_WIDGET (window));
}

static void
buoh_window_menu_add_cb (GtkMenuItem *menuitem, gpointer gdata)
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
buoh_window_menu_save_cb (GtkMenuItem *menuitem, gpointer gdata)
{
	GtkWidget     *chooser;
	GtkFileFilter *filter;
	gchar         *suggested;
	gchar         *name;
	gchar         *page;
	gchar         *filename = NULL;
	static gchar  *folder = NULL;
	BuohWindow    *window = BUOH_WINDOW (gdata);
	BuohComic     *comic;
	GdkPixbuf     *pixbuf;
	GtkWidget     *dialog;
	gboolean      successful;
	GError        *error;
	
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
	name      = buoh_comic_get_title (comic);
	page      = buoh_comic_get_page (comic);
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
buoh_window_menu_properties_cb (GtkMenuItem *menuitem, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);
	BuohComic  *comic = buoh_view_get_comic (window->priv->view);

	if (comic) {
		if (!window->priv->properties) {
			window->priv->properties = buoh_properties_dialog_new ();
			buoh_properties_dialog_set_comic (BUOH_PROPERTIES_DIALOG (window->priv->properties),
							  comic);
			g_object_add_weak_pointer (G_OBJECT (window->priv->properties),
						   (gpointer *) &(window->priv->properties));
			gtk_window_set_transient_for (GTK_WINDOW (window->priv->properties),
						      GTK_WINDOW (window));
		}

		gtk_widget_show (window->priv->properties);
	}
}

static void
buoh_window_menu_zoom_in_cb (GtkMenuItem *menuitem, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	buoh_view_zoom_in (window->priv->view);
}

static void
buoh_window_menu_zoom_out_cb (GtkMenuItem *menuitem, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	buoh_view_zoom_out (window->priv->view);
}

static void
buoh_window_menu_normal_size_cb (GtkMenuItem *menuitem, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	buoh_view_normal_size (window->priv->view);
}

static void
buoh_window_menu_about_cb (GtkMenuItem *menuitem, gpointer gdata)
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
buoh_window_toolbar_zoom_in_cb (GtkToolButton *toolbutton, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	buoh_view_zoom_in (window->priv->view);
}

static void
buoh_window_toolbar_zoom_out_cb (GtkToolButton *toolbutton, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	buoh_view_zoom_out (window->priv->view);
}

static void
buoh_window_toolbar_normal_size_cb (GtkToolButton *toolbutton, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	buoh_view_normal_size (window->priv->view);
}

static void
buoh_window_toolbar_previous_cb (GtkToolButton *toolbutton, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	/* TODO */
}

static void
buoh_window_toolbar_next_cb (GtkToolButton *toolbutton, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);

	/* TODO */
}

static void
buoh_window_set_sensitive (BuohWindow *window, const gchar *name, gboolean sensitive)
{
	GtkWidget *widget = glade_xml_get_widget (window->priv->gui, name);

	gtk_widget_set_sensitive (widget, sensitive);
}

static void
buoh_window_previous_set_sensitive (BuohWindow *window, gboolean sensitive)
{
	buoh_window_set_sensitive (window, "previous_tb_button", sensitive);
	buoh_window_set_sensitive (window, "menu_previous", sensitive);
	buoh_window_set_sensitive (window, "menu_first", sensitive);
}

static void
buoh_window_next_set_sensitive (BuohWindow *window, gboolean sensitive)
{
	buoh_window_set_sensitive (window, "next_tb_button", sensitive);
	buoh_window_set_sensitive (window, "menu_next", sensitive);
	buoh_window_set_sensitive (window, "menu_last", sensitive);
}

static void
buoh_window_zoom_in_set_sensitive (BuohWindow *window, gboolean sensitive)
{
	buoh_window_set_sensitive (window, "zoom_in_tb_button",
				   buoh_view_is_max_zoom (window->priv->view) ? FALSE : sensitive);
	buoh_window_set_sensitive (window, "menu_zoom_in",
				   buoh_view_is_max_zoom (window->priv->view) ? FALSE : sensitive);
}

static void
buoh_window_zoom_out_set_sensitive (BuohWindow *window, gboolean sensitive)
{
	buoh_window_set_sensitive (window, "zoom_out_tb_button",
				   buoh_view_is_min_zoom (window->priv->view) ? FALSE : sensitive);
	buoh_window_set_sensitive (window, "menu_zoom_out",
				   buoh_view_is_min_zoom (window->priv->view) ? FALSE : sensitive);
}

static void
buoh_window_normal_size_set_sensitive (BuohWindow *window, gboolean sensitive)
{
	buoh_window_set_sensitive (window, "normal_size_tb_button",
				   buoh_view_is_normal_size (window->priv->view) ? FALSE : sensitive);
	buoh_window_set_sensitive (window, "menu_normal_size",
				   buoh_view_is_normal_size (window->priv->view) ? FALSE : sensitive);
}

static void
buoh_window_comic_actions_set_sensitive (BuohWindow *window, gboolean sensitive)
{
	buoh_window_previous_set_sensitive (window, sensitive);
	buoh_window_next_set_sensitive (window, sensitive);
	buoh_window_zoom_in_set_sensitive (window, sensitive);
	buoh_window_zoom_out_set_sensitive (window, sensitive);
	buoh_window_normal_size_set_sensitive (window, sensitive);
	buoh_window_set_sensitive (window, "menu_properties", sensitive);
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

	buoh_window_set_sensitive (window, "menu_save", (save_disabled) ?  FALSE : sensitive);
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
}

static void
buoh_window_view_zoom_change_cb (BuohView *view, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);
	
	buoh_window_zoom_in_set_sensitive (window, TRUE);
	buoh_window_zoom_out_set_sensitive (window, TRUE);
	buoh_window_normal_size_set_sensitive (window,TRUE);
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
			popup = gtk_ui_manager_get_widget (window->priv->popup_ui, "/MainMenu");
			
			gtk_menu_popup (GTK_MENU (popup), NULL, NULL, NULL,
					(gpointer) window,
					event->button, event->time);
		}
	}
	
	return FALSE;
}

static void
buoh_window_popup_properties_cb (GtkWidget *widget, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);
	BuohComic  *comic = buoh_view_get_comic (window->priv->view);

	if (comic) {
		if (!window->priv->properties) {
			window->priv->properties = buoh_properties_dialog_new ();
			buoh_properties_dialog_set_comic (BUOH_PROPERTIES_DIALOG (window->priv->properties),
							  comic);
			g_object_add_weak_pointer (G_OBJECT (window->priv->properties),
						   (gpointer *) &(window->priv->properties));
			gtk_window_set_transient_for (GTK_WINDOW (window->priv->properties),
						      GTK_WINDOW (window));
		}

		gtk_widget_show (window->priv->properties);
	}
}

static void
buoh_window_popup_delete_cb (GtkWidget *widget, gpointer gdata)
{
	BuohWindow   *window = BUOH_WINDOW (gdata);
	GtkTreeModel *model = buoh_get_comics_model (BUOH);
	GtkTreeIter   iter;
	BuohComic    *comic, *current_comic;
	gchar        *comic_id, *id;
	gboolean      valid;

	current_comic = buoh_view_get_comic (window->priv->view);

	if (current_comic) {
		comic_id = buoh_comic_get_id (current_comic);

		valid = gtk_tree_model_get_iter_first (model, &iter);

		while (valid) {
			gtk_tree_model_get (model, &iter,
					    COMIC_LIST_COMIC, &comic,
					    -1);
			id = buoh_comic_get_id (comic);

			if (g_ascii_strcasecmp (comic_id, id) == 0) {
				buoh_comic_list_clear_selection (window->priv->comic_list);
				gtk_list_store_set (GTK_LIST_STORE (model), &iter,
						    COMIC_LIST_VISIBLE, FALSE,
						    -1);
				valid = FALSE;
			} else {
				valid = gtk_tree_model_iter_next (model, &iter);
			}

			g_free (id);
		}

		g_free (comic_id);
	}
}

static void
buoh_window_popup_copy_uri_cb (GtkWidget *widget, gpointer gdata)
{
	BuohWindow *window = BUOH_WINDOW (gdata);
	BuohComic  *comic = buoh_view_get_comic (window->priv->view);
	gchar      *uri;

	if (comic) {
		uri = buoh_comic_get_uri (comic);
		g_debug ("Copy %s to clipboard\n", uri);

		gtk_clipboard_set_text (gtk_clipboard_get (GDK_NONE), uri,
					g_utf8_strlen (uri, -1));

		gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_PRIMARY), uri,
					g_utf8_strlen (uri, -1));

		g_free (uri);
	}
}
