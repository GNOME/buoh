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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

#include "buoh-application.h"
#include "buoh-window.h"
#include "buoh-properties-dialog.h"
#include "buoh-add-comic-dialog.h"
#include "buoh-view.h"
#include "buoh-comic-list.h"

struct _BuohWindow {
        GtkWindow       parent;

        GtkActionGroup *action_group;
        GtkUIManager   *ui_manager;

        GtkWidget      *statusbar;
        guint           view_message_cid;
        guint           help_message_cid;

        BuohView       *view;
        BuohComicList  *comic_list;

        GList          *properties;
        GtkWidget      *add_dialog;

        GSettings      *buoh_settings;
        GSettings      *lockdown_settings;
};

#define GS_BUOH_SCHEMA     "org.gnome.buoh"
#define GS_SHOW_TOOLBAR    "show-toolbar"
#define GS_SHOW_STATUSBAR  "show-statusbar"
#define GS_ZOOM_MODE       "zoom-mode"

#define GS_LOCKDOWN_SCHEMA "org.gnome.desktop.lockdown"
#define GS_LOCKDOWN_SAVE   "disable-save-to-disk"

static void buoh_window_init                             (BuohWindow      *buoh_window);
static void buoh_window_class_init                       (BuohWindowClass *klass);
static void buoh_window_finalize                         (GObject         *object);

/* Sensitivity */
static void buoh_window_set_sensitive                    (BuohWindow      *window,
                                                          const gchar     *name,
                                                          gboolean         sensitive);
static void buoh_window_comic_actions_set_sensitive      (BuohWindow      *window,
                                                          gboolean         sensitive);
static void buoh_window_comic_save_to_disk_set_sensitive (BuohWindow      *window,
                                                          gboolean         sensitive);

/* Callbacks */
static void buoh_window_ui_manager_connect_proxy_cb     (GtkUIManager     *manager,
                                                         GtkAction        *action,
                                                         GtkWidget        *proxy,
                                                         gpointer          gdata);
static void buoh_window_ui_manager_disconnect_proxy_cb  (GtkUIManager     *manager,
                                                         GtkAction        *action,
                                                         GtkWidget        *proxy,
                                                         gpointer          gdata);
static void buoh_window_menu_item_select_cb             (GtkMenuItem      *proxy,
                                                         gpointer          gdata);
static void buoh_window_menu_item_deselect_cb           (GtkMenuItem      *proxy,
                                                         gpointer          gdata);
static void buoh_window_view_status_change_cb           (GObject          *object,
                                                         GParamSpec       *arg,
                                                         gpointer          gdata);
static void buoh_window_view_zoom_change_cb             (BuohView         *view,
                                                         gpointer          gdata);
static gboolean buoh_window_comic_list_button_press_cb  (GtkWidget        *widget,
                                                         GdkEventButton   *event,
                                                         gpointer          gdata);
static gboolean buoh_window_comic_list_key_press_cb     (GtkWidget        *widget,
                                                         GdkEventKey      *event,
                                                         gpointer          gdata);
static void  buoh_window_comic_list_selection_change_cb (GtkTreeSelection *selection,
                                                         gpointer          gdata);
static gboolean buoh_window_comic_view_button_press_cb  (GtkWidget        *widget,
                                                         GdkEventButton   *event,
                                                         gpointer          gdata);
static gboolean buoh_window_comic_view_key_press_cb     (GtkWidget        *widget,
                                                         GdkEventKey      *event,
                                                         gpointer          gdata);
static void     buoh_window_properties_dialog_destroyed (GtkWidget        *dialog,
                                                         gpointer          gdata);

/* Action callbacks */
static void buoh_window_cmd_comic_add                   (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_comic_remove                (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_comic_save_a_copy           (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_comic_copy_location         (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_comic_properties            (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_comic_quit                  (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_toolbar                (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_statusbar              (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_zoom_in                (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_zoom_out               (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_zoom_normal            (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_zoom_best_fit          (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_zoom_fit_width         (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_go_previous                 (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_go_next                     (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_go_first                    (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_go_last                     (GtkAction        *action,
                                                         gpointer          gdata);
static void buoh_window_cmd_help_about                  (GtkAction        *action,
                                                         gpointer          gdata);

static void buoh_window_update_title                    (BuohWindow       *window);
static void buoh_window_update_zoom_mode                (BuohWindow       *window);

static const GtkActionEntry menu_entries[] = {

        /* Top Level */
        { "Comic", NULL, N_("_Comic") },
        { "View", NULL, N_("_View") },
        { "Go", NULL, N_("_Go") },
        { "Help", NULL, N_("_Help") },

        /* Comic menu */
        { "ComicAdd", GTK_STOCK_ADD, N_("_Add…"), NULL,
          N_("Add a comic to the comic list"),
          G_CALLBACK (buoh_window_cmd_comic_add) },
        { "ComicRemove", GTK_STOCK_REMOVE, N_("_Remove"), NULL,
          N_("Remove this comic from the comic list"),
          G_CALLBACK (buoh_window_cmd_comic_remove) },
        { "ComicSaveACopy", NULL, N_("_Save a Copy…"), NULL,
          N_("Save the current comic with a new filename"),
          G_CALLBACK (buoh_window_cmd_comic_save_a_copy) },
        { "ComicCopyURI", NULL, N_("_Copy Location"), NULL,
          N_("Copy the location of this comic to clipboard"),
          G_CALLBACK (buoh_window_cmd_comic_copy_location) },
        { "ComicProperties", GTK_STOCK_PROPERTIES, N_("_Properties…"), "<alt>Return",
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
        { "GoPrevious", GTK_STOCK_GO_BACK, N_("_Previous Comic"), "<alt>Left",
          N_("Go to the previous comic"),
          G_CALLBACK (buoh_window_cmd_go_previous) },
        { "GoNext", GTK_STOCK_GO_FORWARD, N_("_Next Comic"), "<alt>Right",
          N_("Go to the next comic"),
          G_CALLBACK (buoh_window_cmd_go_next) },
        { "GoFirst", GTK_STOCK_GOTO_FIRST, N_("_First Comic"), "<control>Home",
          N_("Go to the first comic"),
          G_CALLBACK (buoh_window_cmd_go_first) },
        { "GoLast", GTK_STOCK_GOTO_LAST, N_("_Last Comic"), "<control>End",
          N_("Go to the last comic"),
          G_CALLBACK (buoh_window_cmd_go_last) },

        /* Help menu */
        { "HelpAbout", GTK_STOCK_ABOUT, N_("_About"), NULL,
          N_("Display credits for the Buoh online comic reader creators"),
          G_CALLBACK (buoh_window_cmd_help_about) }
};

static const GtkToggleActionEntry menu_toggle_entries[] = {

        /* View menu*/
        { "ViewToolbar", NULL, N_("_Toolbar"), NULL,
          N_("Changes the visibility of the toolbar"),
          G_CALLBACK (buoh_window_cmd_view_toolbar), TRUE },
        { "ViewStatusbar", NULL, N_("St_atusbar"), NULL,
          N_("Changes the visibility of the statusbar"),
          G_CALLBACK (buoh_window_cmd_view_statusbar), TRUE },
        { "ViewZoomBestFit", GTK_STOCK_ZOOM_FIT, N_("_Best Fit"), NULL,
          N_("Make the current comic fill the window"),
          G_CALLBACK (buoh_window_cmd_view_zoom_best_fit) },
        { "ViewZoomFitWidth", GTK_STOCK_ZOOM_FIT, N_("Fit Comic _Width"), NULL,
          N_("Make the current comic fill the window width"),
          G_CALLBACK (buoh_window_cmd_view_zoom_fit_width) }
};

G_DEFINE_TYPE (BuohWindow, buoh_window, GTK_TYPE_WINDOW)

static void
buoh_window_init (BuohWindow *buoh_window)
{
        GtkWidget        *tree_view;
        GtkTreeModel     *model;
        GtkTreeSelection *selection;
        GtkWidget        *vbox, *paned, *menubar;
        GtkWidget        *toolbar;
        GtkActionGroup   *action_group;
        GtkAction        *action;
        GtkAccelGroup    *accel_group;
        GError           *error = NULL;
        gboolean          visible_toolbar;
        gboolean          visible_statusbar;
        BuohViewZoomMode  zoom_mode;

        gtk_window_set_title (GTK_WINDOW (buoh_window), "Buoh");
        gtk_window_set_icon_name (GTK_WINDOW (buoh_window), "buoh");

        buoh_window->properties = NULL;
        buoh_window->add_dialog = NULL;
        buoh_window->buoh_settings = g_settings_new (GS_BUOH_SCHEMA);
        buoh_window->lockdown_settings = g_settings_new (GS_LOCKDOWN_SCHEMA);

        vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

        /* Menu bar */
        action_group = gtk_action_group_new ("MenuActions");
        buoh_window->action_group = action_group;
        gtk_action_group_set_translation_domain (action_group, NULL);
        gtk_action_group_add_actions (action_group, menu_entries,
                                      G_N_ELEMENTS (menu_entries),
                                      (gpointer) buoh_window);

        gtk_action_group_add_toggle_actions (action_group, menu_toggle_entries,
                                             G_N_ELEMENTS (menu_toggle_entries),
                                             (gpointer) buoh_window);

        buoh_window->ui_manager = gtk_ui_manager_new ();
        gtk_ui_manager_insert_action_group (buoh_window->ui_manager,
                                            action_group, 0);

        accel_group = gtk_ui_manager_get_accel_group (buoh_window->ui_manager);
        gtk_window_add_accel_group (GTK_WINDOW (buoh_window), accel_group);

        if (!gtk_ui_manager_add_ui_from_resource (buoh_window->ui_manager,
                                                  "/org/gnome/buoh/buoh-ui.xml",
                                                  &error)) {
                buoh_debug ("Could not merge buoh-ui.xml: %s", error->message);
                g_error_free (error);
        }

        g_signal_connect (buoh_window->ui_manager, "connect_proxy",
                          G_CALLBACK (buoh_window_ui_manager_connect_proxy_cb),
                          (gpointer) buoh_window);
        g_signal_connect (buoh_window->ui_manager, "disconnect_proxy",
                          G_CALLBACK (buoh_window_ui_manager_disconnect_proxy_cb),
                          (gpointer) buoh_window);

        /* Menu */
        menubar = gtk_ui_manager_get_widget (buoh_window->ui_manager,
                                             "/MainMenu");
        gtk_box_pack_start (GTK_BOX (vbox), menubar,
                            FALSE, FALSE, 0);
        gtk_widget_show (menubar);

        /* Set the active status to the "View [toolbar | statusbar]" menu entry*/
        visible_toolbar = g_settings_get_boolean (buoh_window->buoh_settings,
                                                  GS_SHOW_TOOLBAR);
        action = gtk_action_group_get_action (action_group, "ViewToolbar");
        gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
                                      visible_toolbar);

        visible_statusbar = g_settings_get_boolean (buoh_window->buoh_settings,
                                                    GS_SHOW_STATUSBAR);
        action = gtk_action_group_get_action (action_group, "ViewStatusbar");
        gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
                                      visible_statusbar);

        /* Toolbar */
        toolbar = gtk_ui_manager_get_widget (buoh_window->ui_manager,
                                             "/Toolbar");
        gtk_box_pack_start (GTK_BOX (vbox), toolbar,
                            FALSE, FALSE, 0);
        gtk_widget_show (toolbar);
        g_object_set (G_OBJECT (toolbar),
                      "visible", visible_toolbar,
                      NULL);

        /* Pane */
        paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
        /* FIXME: Remember side position */
        gtk_paned_set_position (GTK_PANED (paned), 230);

        /* buoh view */
        buoh_window->view = BUOH_VIEW (buoh_view_new ());
        zoom_mode = g_settings_get_enum (buoh_window->buoh_settings,
                                         GS_ZOOM_MODE);
        buoh_view_set_zoom_mode (buoh_window->view, zoom_mode);
        g_signal_connect (G_OBJECT (buoh_window->view), "notify::status",
                          G_CALLBACK (buoh_window_view_status_change_cb),
                          (gpointer) buoh_window);
        g_signal_connect (G_OBJECT (buoh_window->view), "scale-changed",
                          G_CALLBACK (buoh_window_view_zoom_change_cb),
                          (gpointer) buoh_window);
        g_signal_connect (G_OBJECT (buoh_window->view), "button-press-event",
                          G_CALLBACK (buoh_window_comic_view_button_press_cb),
                          (gpointer) buoh_window);
        g_signal_connect (G_OBJECT (buoh_window->view), "key-press-event",
                          G_CALLBACK (buoh_window_comic_view_key_press_cb),
                          (gpointer) buoh_window);
        gtk_paned_pack2 (GTK_PANED (paned), GTK_WIDGET (buoh_window->view),
                         TRUE, FALSE);
        gtk_widget_show (GTK_WIDGET (buoh_window->view));

        /* buoh comic list */
        buoh_window->comic_list = BUOH_COMIC_LIST (buoh_comic_list_new ());
        model = buoh_application_get_comics_model (buoh_application_get_instance ());
        buoh_comic_list_set_model (buoh_window->comic_list, model);
        buoh_comic_list_set_view (buoh_window->comic_list, buoh_window->view);
        gtk_paned_pack1 (GTK_PANED (paned), GTK_WIDGET (buoh_window->comic_list),
                         TRUE, FALSE);
        gtk_widget_show (GTK_WIDGET (buoh_window->comic_list));

        gtk_box_pack_start (GTK_BOX (vbox), paned,
                            TRUE, TRUE, 0);
        gtk_widget_show (paned);

        /* Status bar */
        buoh_window->statusbar = gtk_statusbar_new ();
        buoh_window->view_message_cid = gtk_statusbar_get_context_id
                (GTK_STATUSBAR (buoh_window->statusbar), "view_message");
        buoh_window->help_message_cid = gtk_statusbar_get_context_id
                (GTK_STATUSBAR (buoh_window->statusbar), "help_message");
        gtk_box_pack_end (GTK_BOX (vbox), buoh_window->statusbar,
                          FALSE, TRUE, 0);
        gtk_widget_show (buoh_window->statusbar);
        g_object_set (G_OBJECT (buoh_window->statusbar),
                      "visible", visible_statusbar, NULL);

        gtk_container_add (GTK_CONTAINER (buoh_window), vbox);
        gtk_widget_show (vbox);

        tree_view = buoh_comic_list_get_list (buoh_window->comic_list);
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
        g_signal_connect (G_OBJECT (tree_view), "button-press-event",
                          G_CALLBACK (buoh_window_comic_list_button_press_cb),
                          (gpointer) buoh_window);
        g_signal_connect (G_OBJECT (tree_view), "key-press-event",
                          G_CALLBACK (buoh_window_comic_list_key_press_cb),
                          (gpointer) buoh_window);
        g_signal_connect (G_OBJECT (selection), "changed",
                          G_CALLBACK (buoh_window_comic_list_selection_change_cb),
                          (gpointer) buoh_window);

        buoh_window_comic_actions_set_sensitive (buoh_window, FALSE);
        buoh_window_comic_save_to_disk_set_sensitive (buoh_window, FALSE);
        buoh_window_set_sensitive (buoh_window, "ComicRemove", FALSE);
        buoh_window_update_zoom_mode (buoh_window);

        gtk_widget_grab_focus (GTK_WIDGET (buoh_window->view));

        gtk_widget_show (GTK_WIDGET (buoh_window));
}

static void
buoh_window_class_init (BuohWindowClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = buoh_window_finalize;
}

static void
buoh_window_finalize (GObject *object)
{
        BuohWindow *buoh_window = BUOH_WINDOW (object);

        buoh_debug ("buoh-window finalize");

        g_clear_object (&buoh_window->ui_manager);

        g_clear_object (&buoh_window->action_group);

        g_clear_object (&buoh_window->buoh_settings);

        g_clear_object (&buoh_window->lockdown_settings);

        if (buoh_window->properties) {
                g_list_foreach (buoh_window->properties,
                                (GFunc) g_object_unref, NULL);
                g_clear_pointer (&buoh_window->properties, g_list_free);
        }

        g_clear_object (&buoh_window->add_dialog);

        if (G_OBJECT_CLASS (buoh_window_parent_class)->finalize) {
                (* G_OBJECT_CLASS (buoh_window_parent_class)->finalize) (object);
        }

        buoh_application_exit (buoh_application_get_instance ());
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

        if (!window->add_dialog) {
                window->add_dialog = buoh_add_comic_dialog_new ();
                g_object_add_weak_pointer (G_OBJECT (window->add_dialog),
                                           (gpointer *) &(window->add_dialog));
                gtk_window_set_transient_for (GTK_WINDOW (window->add_dialog),
                                              GTK_WINDOW (window));
        }

        gtk_widget_show (window->add_dialog);
}

static void
buoh_window_cmd_comic_remove (GtkAction *action, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        GtkTreeModel     *model = buoh_application_get_comics_model (buoh_application_get_instance ());
        GtkTreeIter       iter;
        BuohComicManager *cm;
        BuohComicManager *current_cm;
        const gchar      *current_cm_id;
        const gchar      *cm_id;
        gboolean          valid;

        current_cm = buoh_comic_list_get_selected (window->comic_list);

        if (!current_cm) {
                return;
        }

        current_cm_id = buoh_comic_manager_get_id (current_cm);

        valid = gtk_tree_model_get_iter_first (model, &iter);

        while (valid) {
                gtk_tree_model_get (model, &iter,
                                    COMIC_LIST_COMIC_MANAGER, &cm,
                                    -1);
                cm_id = buoh_comic_manager_get_id (cm);
                g_object_unref (cm);

                if (g_ascii_strcasecmp (current_cm_id, cm_id) == 0) {
                        buoh_comic_list_clear_selection (window->comic_list);
                        gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                                            COMIC_LIST_VISIBLE, FALSE,
                                            -1);
                        valid = FALSE;
                } else {
                        valid = gtk_tree_model_iter_next (model, &iter);
                }
        }
}

static void
buoh_window_cmd_comic_save_a_copy (GtkAction *action, gpointer gdata)
{
        GtkWidget        *chooser;
        GtkFileFilter    *filter;
        gchar            *suggested;
        static gchar     *folder = NULL;
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComic        *comic;
        BuohComicImage   *image;
        GtkWidget        *dialog;
        gboolean          successful;

        filter = gtk_file_filter_new ();
        gtk_file_filter_set_name (filter, _("Images"));
        gtk_file_filter_add_pixbuf_formats (filter);

        chooser = gtk_file_chooser_dialog_new (_("Save Comic"),
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

        comic  = buoh_view_get_comic (window->view);
        image = buoh_comic_get_image (comic);

        suggested = buoh_comic_get_filename (comic);
        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (chooser),
                                           suggested);
        g_free (suggested);

        do {
                if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT) {
                        gchar  *filename;
                        GError *error = NULL;

                        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

                        if (folder != NULL) {
                                g_free (folder);
                        }

                        folder = gtk_file_chooser_get_current_folder_uri (GTK_FILE_CHOOSER (chooser));

                        if (!buoh_comic_image_save (image, filename, &error)) {
                                successful = FALSE;

                                dialog = gtk_message_dialog_new (GTK_WINDOW (chooser),
                                                                 GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                                                                 GTK_MESSAGE_ERROR,
                                                                 GTK_BUTTONS_CLOSE,
                                                                 _("Unable to save comic"));

                                gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                                                          "%s", error->message);
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

        gtk_widget_destroy (chooser);
}

static void
buoh_window_cmd_comic_copy_location (GtkAction *action, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);
        BuohComic  *comic = buoh_view_get_comic (window->view);

        if (comic) {
                const gchar *uri = buoh_comic_get_uri (comic);

                buoh_debug ("Copy %s to clipboard", uri);

                gtk_clipboard_set_text (gtk_clipboard_get (GDK_NONE), uri,
                                        g_utf8_strlen (uri, -1));

                gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_PRIMARY), uri,
                                        g_utf8_strlen (uri, -1));
        }
}

static void
buoh_window_properties_dialog_destroyed (GtkWidget *dialog, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        buoh_debug ("porperties-dialog destroyed");

        window->properties = g_list_remove (window->properties, dialog);
}

static void
buoh_window_cmd_comic_properties (GtkAction *action, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComicManager *cm  = buoh_comic_list_get_selected (window->comic_list);
        BuohComicManager *cm2 = NULL;
        GtkWidget        *dialog;
        const gchar      *id1, *id2;
        GList            *l = NULL;

        if (!cm) {
                return;
        }

        id1 = buoh_comic_manager_get_id (cm);

        for (l = window->properties; l; l = g_list_next (l)) {
                cm2 = buoh_properties_dialog_get_comic_manager (
                        BUOH_PROPERTIES_DIALOG (l->data));
                id2 = buoh_comic_manager_get_id (cm2);

                if (g_ascii_strcasecmp (id1, id2) == 0) {
                        gtk_window_present (GTK_WINDOW (l->data));
                        return;
                }
        }

        dialog = buoh_properties_dialog_new ();
        buoh_properties_dialog_set_comic_manager (BUOH_PROPERTIES_DIALOG (dialog), cm);
        g_signal_connect (G_OBJECT (dialog), "destroy",
                          G_CALLBACK (buoh_window_properties_dialog_destroyed),
                          (gpointer) window);
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));

        gtk_widget_show (dialog);

        window->properties = g_list_append (window->properties, dialog);
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
        BuohWindow  *window = BUOH_WINDOW (gdata);
        GtkWidget   *toolbar;
        gboolean     visible;

        visible = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
        g_settings_set_boolean (window->buoh_settings,
                                GS_SHOW_TOOLBAR, visible);

        toolbar = gtk_ui_manager_get_widget (window->ui_manager, "/Toolbar");
        g_object_set (G_OBJECT (toolbar), "visible",
                      visible,
                      NULL);
}

static void
buoh_window_cmd_view_statusbar (GtkAction *action, gpointer gdata)
{
        BuohWindow  *window = BUOH_WINDOW (gdata);
        gboolean     visible;

        visible = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
        g_settings_set_boolean (window->buoh_settings,
                                GS_SHOW_STATUSBAR, visible);

        g_object_set (G_OBJECT (window->statusbar),
                      "visible", visible,
                      NULL);
}

static void
buoh_window_cmd_view_zoom_in (GtkAction *action, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        buoh_view_zoom_in (window->view);
}

static void
buoh_window_cmd_view_zoom_out (GtkAction *action, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        buoh_view_zoom_out (window->view);
}

static void
buoh_window_cmd_view_zoom_normal (GtkAction *action, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        buoh_view_zoom_normal_size (window->view);
}

static void
buoh_window_cmd_view_zoom_best_fit (GtkAction *action, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action))) {
                buoh_view_zoom_best_fit (window->view);
                g_settings_set_enum (window->buoh_settings,
                                     GS_ZOOM_MODE, VIEW_ZOOM_BEST_FIT);
        } else {
                buoh_view_zoom_normal_size (window->view);
                g_settings_set_enum (window->buoh_settings,
                                     GS_ZOOM_MODE, VIEW_ZOOM_FREE);
        }
}

static void
buoh_window_cmd_view_zoom_fit_width (GtkAction *action, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action))) {
                buoh_view_zoom_fit_width (window->view);
                g_settings_set_enum (window->buoh_settings,
                                     GS_ZOOM_MODE, VIEW_ZOOM_FIT_WIDTH);
        } else {
                buoh_view_zoom_normal_size (window->view);
                g_settings_set_enum (window->buoh_settings,
                                     GS_ZOOM_MODE, VIEW_ZOOM_FREE);
        }
}

static void
buoh_window_cmd_go_previous (GtkAction *action, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComicManager *comic_manager;
        BuohComic        *comic;

        comic_manager = buoh_comic_list_get_selected (window->comic_list);

        comic = buoh_comic_manager_get_previous (comic_manager);

        buoh_view_set_comic (window->view, comic);
}

static void
buoh_window_cmd_go_next (GtkAction *action, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComicManager *comic_manager;
        BuohComic        *comic;

        comic_manager = buoh_comic_list_get_selected (window->comic_list);

        comic = buoh_comic_manager_get_next (comic_manager);

        buoh_view_set_comic (window->view, comic);
}

static void
buoh_window_cmd_go_first (GtkAction *action, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComicManager *comic_manager;
        BuohComic        *comic;

        comic_manager = buoh_comic_list_get_selected (window->comic_list);

        comic = buoh_comic_manager_get_first (comic_manager);

        buoh_view_set_comic (window->view, comic);
}
static void
buoh_window_cmd_go_last (GtkAction *action, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComicManager *comic_manager;
        BuohComic        *comic;

        comic_manager = buoh_comic_list_get_selected (window->comic_list);

        comic = buoh_comic_manager_get_last (comic_manager);

        buoh_view_set_comic (window->view, comic);
}

static void
buoh_window_cmd_help_about (GtkAction *action, gpointer gdata)
{
        BuohWindow         *window = BUOH_WINDOW (gdata);
        static const gchar *authors[] = {
                "Esteban Sanchez Muñoz <esteban@steve-o.org>",
                "Pablo Arroyo Loma <zioma@linups.org>",
                "Carlos García Campos <carlosgc@gnome.org>",
                NULL
        };

        gtk_show_about_dialog (GTK_WINDOW (window),
                               "authors", authors,
                               "comments", _("Online comic strips reader"),
                               "copyright", "Copyright © 2004 Esteban Sanchez Muñoz, Pablo Arroyo Loma",
                               "logo-icon-name", "buoh",
                               "translator-credits", _("translator-credits"),
                               "version", VERSION,
                               "website", "http://buoh.steve-o.org/",
                               NULL);
}

static void
buoh_window_update_title (BuohWindow *window)
{
        BuohComicManager *cm;
        gchar            *title = NULL;

        cm = buoh_comic_list_get_selected (window->comic_list);

        if (cm) {
                title = g_strdup_printf ("%s — Buoh",
                                         buoh_comic_manager_get_title (cm));
        } else {
                title = g_strdup_printf ("Buoh");
        }

        gtk_window_set_title (GTK_WINDOW (window), title);
        g_free (title);
}

static void
buoh_window_set_sensitive (BuohWindow *window, const gchar *name, gboolean sensitive)
{
        GtkAction *action = gtk_action_group_get_action (window->action_group,
                                                         name);
        gtk_action_set_sensitive (action, sensitive);
}

static void
buoh_window_comic_browsing_actions_set_sensitive (BuohWindow *window, gboolean sensitive)
{
        BuohComicManager *cm;

        cm = buoh_comic_list_get_selected (window->comic_list);

        buoh_window_set_sensitive (window, "GoPrevious",
                                      sensitive ?
                                   !buoh_comic_manager_is_the_first (cm) :
                                   sensitive);
        buoh_window_set_sensitive (window, "GoNext",
                                      sensitive ?
                                   !buoh_comic_manager_is_the_last (cm) :
                                   sensitive);
        buoh_window_set_sensitive (window, "GoFirst",
                                   sensitive ?
                                   !buoh_comic_manager_is_the_first (cm) :
                                   sensitive);
        buoh_window_set_sensitive (window, "GoLast",
                                   sensitive ?
                                   !buoh_comic_manager_is_the_last (cm) :
                                   sensitive);
}

static void
buoh_window_comic_actions_set_sensitive (BuohWindow *window, gboolean sensitive)
{
        buoh_window_comic_browsing_actions_set_sensitive (window, sensitive);

        buoh_window_set_sensitive (window, "ComicProperties", sensitive);
        buoh_window_set_sensitive (window, "ComicCopyURI",    sensitive);
        buoh_window_set_sensitive (window, "ViewZoomIn",
                                   buoh_view_is_max_zoom (window->view) ?
                                   FALSE : sensitive);
        buoh_window_set_sensitive (window, "ViewZoomOut",
                                   buoh_view_is_min_zoom (window->view) ?
                                   FALSE : sensitive);
        buoh_window_set_sensitive (window, "ViewZoomNormal",
                                   buoh_view_is_normal_size (window->view) ?
                                   FALSE : sensitive);
        buoh_window_set_sensitive (window, "ViewZoomBestFit", sensitive);
        buoh_window_set_sensitive (window, "ViewZoomFitWidth", sensitive);
}

static void
buoh_window_comic_save_to_disk_set_sensitive (BuohWindow *window, gboolean sensitive)
{
        gboolean save_disabled = FALSE;

        if (g_settings_get_boolean (window->lockdown_settings,
                                    GS_LOCKDOWN_SAVE)) {
                save_disabled = TRUE;
        }

        buoh_window_set_sensitive (window, "ComicSaveACopy",
                                   (save_disabled) ?  FALSE : sensitive);
}

static void
buoh_window_ui_manager_connect_proxy_cb (GtkUIManager *manager,
                                         GtkAction    *action,
                                         GtkWidget    *proxy,
                                         gpointer      gdata)
{
        if (GTK_IS_MENU_ITEM (proxy)) {
                g_signal_connect (proxy, "select",
                                  G_CALLBACK (buoh_window_menu_item_select_cb),
                                  gdata);
                g_signal_connect (proxy, "deselect",
                                  G_CALLBACK (buoh_window_menu_item_deselect_cb),
                                  gdata);
        }
}

static void
buoh_window_ui_manager_disconnect_proxy_cb (GtkUIManager *manager,
                                            GtkAction    *action,
                                            GtkWidget    *proxy,
                                            gpointer      gdata)
{
        if (GTK_IS_MENU_ITEM (proxy)) {
                g_signal_handlers_disconnect_by_func
                        (proxy,
                         G_CALLBACK (buoh_window_menu_item_select_cb),
                         gdata);
                g_signal_handlers_disconnect_by_func
                        (proxy,
                         G_CALLBACK (buoh_window_menu_item_deselect_cb),
                         gdata);
        }
}

static void
buoh_window_menu_item_select_cb (GtkMenuItem *proxy, gpointer gdata)

{
        BuohWindow *window = BUOH_WINDOW (gdata);
        GtkAction  *action;
        gchar      *message = NULL;

        action = g_object_get_data (G_OBJECT (proxy),  "gtk-action");
        g_assert (action != NULL);

        g_object_get (G_OBJECT (action), "tooltip", &message, NULL);
        if (message) {
                gtk_statusbar_push (GTK_STATUSBAR (window->statusbar),
                                    window->help_message_cid, message);
                g_free (message);
        }
}

static void
buoh_window_menu_item_deselect_cb (GtkMenuItem *proxy, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        gtk_statusbar_pop (GTK_STATUSBAR (window->statusbar),
                           window->help_message_cid);
}

static void
buoh_window_view_status_change_cb (GObject *object, GParamSpec *arg, gpointer gdata)
{
        BuohView         *view = BUOH_VIEW (object);
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComic        *comic = NULL;
        BuohComicManager *cm;
        GtkStatusbar     *statusbar = GTK_STATUSBAR (window->statusbar);
        gchar            *message = NULL;

        cm = buoh_comic_list_get_selected (window->comic_list);

        switch (buoh_view_get_status (view)) {
        case STATE_MESSAGE_WELCOME:
        case STATE_EMPTY:
                buoh_window_comic_actions_set_sensitive (window, FALSE);
                buoh_window_comic_save_to_disk_set_sensitive (window, FALSE);
                break;
        case STATE_MESSAGE_ERROR:
                buoh_window_comic_actions_set_sensitive (window, FALSE);
                buoh_window_comic_browsing_actions_set_sensitive (window, TRUE);
                break;
        case STATE_COMIC_LOADING:
                comic = buoh_view_get_comic (view);

                message = g_strdup (_("Getting comic…"));

                buoh_window_comic_actions_set_sensitive (window,
                                                         (comic) ? TRUE : FALSE);
                buoh_window_comic_save_to_disk_set_sensitive (window, FALSE);
                break;
        case STATE_COMIC_LOADED:
                comic = buoh_view_get_comic (view);

                message = g_strdup_printf ("%s — %s",
                                           buoh_comic_manager_get_title (cm),
                                           buoh_comic_get_id (comic));

                buoh_window_comic_actions_set_sensitive (window,
                                                         (comic) ? TRUE : FALSE);
                buoh_window_comic_save_to_disk_set_sensitive (window,
                                                              (comic) ? TRUE : FALSE);
                break;
        default:
                break;
        }

        gtk_statusbar_pop (statusbar, window->view_message_cid);
        if (message) {
                gtk_statusbar_push (statusbar, window->view_message_cid,
                                    message);
                g_free (message);
        }

        gtk_widget_grab_focus (GTK_WIDGET (view));
}

static void
buoh_window_update_zoom_mode (BuohWindow *window)
{
        BuohViewZoomMode  zoom_mode;
        GtkAction        *action;

        zoom_mode = buoh_view_get_zoom_mode (window->view);

        action = gtk_action_group_get_action (window->action_group,
                                              "ViewZoomBestFit");
        g_signal_handlers_block_by_func
                (action, G_CALLBACK (buoh_window_cmd_view_zoom_best_fit), window);
        gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
                                      zoom_mode == VIEW_ZOOM_BEST_FIT);
        g_signal_handlers_unblock_by_func
                (action, G_CALLBACK (buoh_window_cmd_view_zoom_best_fit), window);

        action = gtk_action_group_get_action (window->action_group,
                                              "ViewZoomFitWidth");
        g_signal_handlers_block_by_func
                (action, G_CALLBACK (buoh_window_cmd_view_zoom_fit_width), window);
        gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
                                      zoom_mode == VIEW_ZOOM_FIT_WIDTH);
        g_signal_handlers_unblock_by_func
                (action, G_CALLBACK (buoh_window_cmd_view_zoom_fit_width), window);
}

static void
buoh_window_view_zoom_change_cb (BuohView *view, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        buoh_window_set_sensitive (window, "ViewZoomIn",
                                   !buoh_view_is_max_zoom (window->view));
        buoh_window_set_sensitive (window, "ViewZoomOut",
                                   !buoh_view_is_min_zoom (window->view));
        buoh_window_set_sensitive (window, "ViewZoomNormal",
                                   !buoh_view_is_normal_size (window->view));

        buoh_window_update_zoom_mode (window);

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
                        popup = gtk_ui_manager_get_widget (window->ui_manager, "/ListPopup");

                        gtk_menu_popup_at_pointer (GTK_MENU (popup),
                                                   (const GdkEvent *) event);
                        return TRUE;
                }
        }

        return FALSE;
}

static gboolean
buoh_window_comic_list_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        GtkWidget        *popup;
        GtkTreeSelection *selection;
        guint             state;

        state = event->state & GDK_SHIFT_MASK;

        if (state == GDK_SHIFT_MASK &&
            event->keyval == GDK_KEY_F10) {
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
                if (gtk_tree_selection_get_selected (selection, NULL, NULL)) {
                        popup = gtk_ui_manager_get_widget (window->ui_manager, "/ListPopup");

                        gtk_menu_popup_at_pointer (GTK_MENU (popup),
                                                   (const GdkEvent *) event);
                        return TRUE;
                }
        }

        return FALSE;
}

static void
buoh_window_comic_list_selection_change_cb (GtkTreeSelection *selection, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        if (gtk_tree_selection_get_selected (selection, NULL, NULL)) {
                buoh_window_update_title (window);
                buoh_window_set_sensitive (window, "ComicRemove", TRUE);
        } else {
                gtk_window_set_title (GTK_WINDOW (window), "Buoh");
                buoh_window_set_sensitive (window, "ComicRemove", FALSE);
        }
}

static gboolean
buoh_window_comic_view_button_press_cb (GtkWidget *widget, GdkEventButton *event, gpointer gdata)
{
        BuohWindow     *window = BUOH_WINDOW (gdata);
        GtkWidget      *popup;
        BuohViewStatus  view_status;

        view_status = buoh_view_get_status (window->view);
        if (view_status != STATE_COMIC_LOADING &&
            view_status != STATE_COMIC_LOADED) {
                return FALSE;
        }

        if (event->button == 3) {
                popup = gtk_ui_manager_get_widget (window->ui_manager, "/ViewPopup");

                gtk_menu_popup_at_pointer (GTK_MENU (popup),
                                           (const GdkEvent *) event);
                return TRUE;
        }

        return FALSE;
}

static gboolean
buoh_window_comic_view_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer gdata)
{
        BuohWindow     *window = BUOH_WINDOW (gdata);
        GtkWidget      *popup;
        guint           state;
        BuohViewStatus  view_status;

        view_status = buoh_view_get_status (window->view);
        if (view_status != STATE_COMIC_LOADING &&
            view_status != STATE_COMIC_LOADED) {
                return FALSE;
        }

        state = event->state & GDK_SHIFT_MASK;

        if (state == GDK_SHIFT_MASK &&
            event->keyval == GDK_KEY_F10) {
                popup = gtk_ui_manager_get_widget (window->ui_manager, "/ViewPopup");

                gtk_menu_popup_at_pointer (GTK_MENU (popup),
                                           (const GdkEvent *) event);

                return TRUE;
        }

        return FALSE;
}
