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
        GtkApplicationWindow parent;

        GtkWidget      *toolbar;
        GtkWidget      *comic_list;
        GtkWidget      *view;
        GtkWidget      *statusbar;
        GtkWidget      *list_popup;
        GtkWidget      *view_popup;
        guint           view_message_cid;
        guint           help_message_cid;

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
static void buoh_window_cmd_comic_add                   (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_comic_remove                (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_comic_save_a_copy           (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_comic_copy_location         (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_comic_properties            (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_comic_quit                  (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_toolbar                (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_statusbar              (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_zoom_in                (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_zoom_out               (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_zoom_normal            (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_view_zoom_mode              (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_go_previous                 (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_go_next                     (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_go_first                    (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_go_last                     (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);
static void buoh_window_cmd_help_about                  (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);

static void activate_toggle                             (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);

static void activate_radio                              (GSimpleAction    *action,
                                                         GVariant         *parameter,
                                                         gpointer          gdata);

static void buoh_window_update_title                    (BuohWindow       *window);
static void buoh_window_update_zoom_mode                (BuohWindow       *window);

static const GActionEntry menu_entries[] = {
        /* Comic menu */
        { "comic-add",
          buoh_window_cmd_comic_add },
        { "comic-remove",
          buoh_window_cmd_comic_remove },
        { "comic-save-a-copy",
          buoh_window_cmd_comic_save_a_copy },
        { "comic-copy-uri",
          buoh_window_cmd_comic_copy_location },
        { "comic-properties",
          buoh_window_cmd_comic_properties },
        { "comic-quit",
          buoh_window_cmd_comic_quit },

        /* View menu*/
        { "view-toolbar",
          activate_toggle, NULL, "true", buoh_window_cmd_view_toolbar },
        { "view-statusbar",
          activate_toggle, NULL, "true", buoh_window_cmd_view_statusbar },
        { "view-zoom-in",
          buoh_window_cmd_view_zoom_in },
        { "view-zoom-out",
          buoh_window_cmd_view_zoom_out },
        { "view-zoom-normal",
          buoh_window_cmd_view_zoom_normal },
        { "view-zoom-mode",
          activate_radio, "s", "'free'", buoh_window_cmd_view_zoom_mode },

        /* Go menu */
        { "go-previous",
          buoh_window_cmd_go_previous },
        { "go-next",
          buoh_window_cmd_go_next },
        { "go-first",
          buoh_window_cmd_go_first },
        { "go-last",
          buoh_window_cmd_go_last },

        /* Help menu */
        { "help-about",
          buoh_window_cmd_help_about }
};

G_DEFINE_TYPE (BuohWindow, buoh_window, GTK_TYPE_APPLICATION_WINDOW)

static void
buoh_window_init (BuohWindow *buoh_window)
{
        GtkWidget        *tree_view;
        GtkTreeModel     *model;
        GtkTreeSelection *selection;
        GActionMap       *action_map;
        GAction          *action;
        gboolean          visible_toolbar;
        gboolean          visible_statusbar;
        BuohViewZoomMode  zoom_mode;

        g_type_ensure (BUOH_TYPE_COMIC_LIST);
        g_type_ensure (BUOH_TYPE_VIEW);
        gtk_widget_init_template (GTK_WIDGET (buoh_window));

        buoh_window->properties = NULL;
        buoh_window->add_dialog = NULL;
        buoh_window->buoh_settings = g_settings_new (GS_BUOH_SCHEMA);
        buoh_window->lockdown_settings = g_settings_new (GS_LOCKDOWN_SCHEMA);

        /* Menu bar */
        action_map = G_ACTION_MAP (buoh_window);
        g_action_map_add_action_entries (action_map, menu_entries, G_N_ELEMENTS (menu_entries), buoh_window);

        /* Menu */
        /* Set the active status to the "View [toolbar | statusbar]" menu entry*/
        visible_toolbar = g_settings_get_boolean (buoh_window->buoh_settings,
                                                  GS_SHOW_TOOLBAR);
        action = g_action_map_lookup_action (action_map, "view-toolbar");
        g_action_change_state (G_ACTION (action), g_variant_new_boolean (visible_toolbar));

        visible_statusbar = g_settings_get_boolean (buoh_window->buoh_settings,
                                                    GS_SHOW_STATUSBAR);
        action = g_action_map_lookup_action (action_map, "view-statusbar");
        g_action_change_state (G_ACTION (action), g_variant_new_boolean (visible_statusbar));

        /* Toolbar */
        g_object_set (G_OBJECT (buoh_window->toolbar),
                      "visible", visible_toolbar,
                      NULL);

        /* buoh view */
        zoom_mode = g_settings_get_enum (buoh_window->buoh_settings,
                                         GS_ZOOM_MODE);
        buoh_view_set_zoom_mode (BUOH_VIEW (buoh_window->view), zoom_mode);
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

        /* buoh comic list */
        model = buoh_application_get_comics_model (buoh_application_get_instance ());
        buoh_comic_list_set_model (BUOH_COMIC_LIST (buoh_window->comic_list), model);
        buoh_comic_list_set_view (BUOH_COMIC_LIST (buoh_window->comic_list), BUOH_VIEW (buoh_window->view));

        /* Status bar */
        buoh_window->view_message_cid = gtk_statusbar_get_context_id
                (GTK_STATUSBAR (buoh_window->statusbar), "view_message");
        buoh_window->help_message_cid = gtk_statusbar_get_context_id
                (GTK_STATUSBAR (buoh_window->statusbar), "help_message");
        g_object_set (G_OBJECT (buoh_window->statusbar),
                      "visible", visible_statusbar, NULL);

        tree_view = buoh_comic_list_get_list (BUOH_COMIC_LIST (buoh_window->comic_list));
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
        buoh_window_set_sensitive (buoh_window, "comic-remove", FALSE);
        buoh_window_update_zoom_mode (buoh_window);
        GEnumClass *enum_class = g_type_class_ref (BUOH_TYPE_VIEW_ZOOM_MODE);
        action = g_action_map_lookup_action (action_map, "view-zoom-mode");
        g_type_class_unref (enum_class);
        g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_string (g_enum_get_value (enum_class, zoom_mode)->value_nick));

        buoh_window->list_popup = gtk_menu_new_from_model (G_MENU_MODEL (gtk_application_get_menu_by_id (GTK_APPLICATION (buoh_application_get_instance ()), "list-popup")));
        gtk_menu_attach_to_widget (GTK_MENU (buoh_window->list_popup),
                                   buoh_window->comic_list,
                                   NULL);
        buoh_window->view_popup = gtk_menu_new_from_model (G_MENU_MODEL (gtk_application_get_menu_by_id (GTK_APPLICATION (buoh_application_get_instance ()), "view-popup")));
        gtk_menu_attach_to_widget (GTK_MENU (buoh_window->view_popup),
                                   buoh_window->view,
                                   NULL);


        gtk_widget_grab_focus (buoh_window->view);
}

static void
buoh_window_class_init (BuohWindowClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        object_class->finalize = buoh_window_finalize;

        gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/buoh/ui/window.ui");

        gtk_widget_class_bind_template_child (widget_class, BuohWindow, comic_list);
        gtk_widget_class_bind_template_child (widget_class, BuohWindow, view);
        gtk_widget_class_bind_template_child (widget_class, BuohWindow, statusbar);
        gtk_widget_class_bind_template_child (widget_class, BuohWindow, toolbar);
}

static void
buoh_window_finalize (GObject *object)
{
        BuohWindow *buoh_window = BUOH_WINDOW (object);

        buoh_debug ("buoh-window finalize");

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
}

GtkWidget *
buoh_window_new (void)
{
        GtkWidget *buoh_window;

        buoh_window = GTK_WIDGET (g_object_new (BUOH_TYPE_WINDOW, NULL));
        return buoh_window;
}

static void
buoh_window_cmd_comic_add (GSimpleAction *action, GVariant *parameter, gpointer gdata)
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
buoh_window_cmd_comic_remove (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        GtkTreeModel     *model = buoh_application_get_comics_model (buoh_application_get_instance ());
        GtkTreeIter       iter;
        BuohComicManager *cm;
        BuohComicManager *current_cm;
        const gchar      *current_cm_id;
        const gchar      *cm_id;
        gboolean          valid;

        current_cm = buoh_comic_list_get_selected (BUOH_COMIC_LIST (window->comic_list));

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
                        buoh_comic_list_clear_selection (BUOH_COMIC_LIST (window->comic_list));
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
buoh_window_cmd_comic_save_a_copy (GSimpleAction *action, GVariant *parameter, gpointer gdata)
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

        comic  = buoh_view_get_comic (BUOH_VIEW (window->view));
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
buoh_window_cmd_comic_copy_location (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);
        BuohComic  *comic = buoh_view_get_comic (BUOH_VIEW (window->view));

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
buoh_window_cmd_comic_properties (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComicManager *cm  = buoh_comic_list_get_selected (BUOH_COMIC_LIST (window->comic_list));
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
buoh_window_cmd_comic_quit (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        gtk_widget_destroy (GTK_WIDGET (window));
}

static void
buoh_window_cmd_view_toolbar (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow  *window = BUOH_WINDOW (gdata);
        gboolean     visible;

        visible = g_variant_get_boolean (parameter);
        g_settings_set_boolean (window->buoh_settings,
                                GS_SHOW_TOOLBAR, visible);

        g_object_set (G_OBJECT (window->toolbar), "visible",
                      visible,
                      NULL);

        g_simple_action_set_state (action, parameter);
}

static void
buoh_window_cmd_view_statusbar (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow  *window = BUOH_WINDOW (gdata);
        gboolean     visible;

        visible = g_variant_get_boolean (parameter);
        g_settings_set_boolean (window->buoh_settings,
                                GS_SHOW_STATUSBAR, visible);

        g_object_set (G_OBJECT (window->statusbar),
                      "visible", visible,
                      NULL);

        g_simple_action_set_state (action, parameter);
}

static void
buoh_window_view_zoom_free (gpointer data)
{
        GAction *action = g_action_map_lookup_action (G_ACTION_MAP (data), "view-zoom-mode");
        g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_string ("free"));

        g_settings_set_enum (BUOH_WINDOW (data)->buoh_settings,
                             GS_ZOOM_MODE, VIEW_ZOOM_FREE);
}

static void
buoh_window_cmd_view_zoom_in (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        buoh_view_zoom_in (BUOH_VIEW (window->view));

        buoh_window_view_zoom_free (gdata);
}

static void
buoh_window_cmd_view_zoom_out (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        buoh_view_zoom_out (BUOH_VIEW (window->view));

        buoh_window_view_zoom_free (gdata);
}

static void
buoh_window_cmd_view_zoom_normal (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        buoh_view_zoom_normal_size (BUOH_VIEW (window->view));

        buoh_window_view_zoom_free (gdata);
}

static void
buoh_window_cmd_view_zoom_mode (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow  *window = BUOH_WINDOW (gdata);
        const gchar *zoom_mode = g_variant_get_string (parameter, NULL);

        if (strcmp(zoom_mode, "best-fit") == 0) {
                buoh_view_zoom_best_fit (BUOH_VIEW (window->view));
                g_settings_set_enum (window->buoh_settings,
                                     GS_ZOOM_MODE, VIEW_ZOOM_BEST_FIT);
        } else if (strcmp(zoom_mode, "fit-width") == 0) {
                buoh_view_zoom_fit_width (BUOH_VIEW (window->view));
                g_settings_set_enum (window->buoh_settings,
                                     GS_ZOOM_MODE, VIEW_ZOOM_FIT_WIDTH);
        } else {
                buoh_view_zoom_normal_size (BUOH_VIEW (window->view));
                g_settings_set_enum (window->buoh_settings,
                                     GS_ZOOM_MODE, VIEW_ZOOM_FREE);
        }

        g_simple_action_set_state (action, parameter);
}

static void
buoh_window_cmd_go_previous (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComicManager *comic_manager;
        BuohComic        *comic;

        comic_manager = buoh_comic_list_get_selected (BUOH_COMIC_LIST (window->comic_list));

        comic = buoh_comic_manager_get_previous (comic_manager);

        buoh_view_set_comic (BUOH_VIEW (window->view), comic);
}

static void
buoh_window_cmd_go_next (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComicManager *comic_manager;
        BuohComic        *comic;

        comic_manager = buoh_comic_list_get_selected (BUOH_COMIC_LIST (window->comic_list));

        comic = buoh_comic_manager_get_next (comic_manager);

        buoh_view_set_comic (BUOH_VIEW (window->view), comic);
}

static void
buoh_window_cmd_go_first (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComicManager *comic_manager;
        BuohComic        *comic;

        comic_manager = buoh_comic_list_get_selected (BUOH_COMIC_LIST (window->comic_list));

        comic = buoh_comic_manager_get_first (comic_manager);

        buoh_view_set_comic (BUOH_VIEW (window->view), comic);
}
static void
buoh_window_cmd_go_last (GSimpleAction *action, GVariant *parameter, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        BuohComicManager *comic_manager;
        BuohComic        *comic;

        comic_manager = buoh_comic_list_get_selected (BUOH_COMIC_LIST (window->comic_list));

        comic = buoh_comic_manager_get_last (comic_manager);

        buoh_view_set_comic (BUOH_VIEW (window->view), comic);
}

static void
buoh_window_cmd_help_about (GSimpleAction *action, GVariant *parameter, gpointer gdata)
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
activate_toggle (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
        GVariant *state;

        state = g_action_get_state (G_ACTION (action));
        g_action_change_state (G_ACTION (action), g_variant_new_boolean (!g_variant_get_boolean (state)));
        g_variant_unref (state);
}

static void
activate_radio (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
        g_action_change_state (G_ACTION (action), parameter);
}

static void
buoh_window_update_title (BuohWindow *window)
{
        BuohComicManager *cm;
        gchar            *title = NULL;

        cm = buoh_comic_list_get_selected (BUOH_COMIC_LIST (window->comic_list));

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
        GSimpleAction *action = G_SIMPLE_ACTION (g_action_map_lookup_action (G_ACTION_MAP (window), name));
        g_simple_action_set_enabled (action, sensitive);
}

static void
buoh_window_comic_browsing_actions_set_sensitive (BuohWindow *window, gboolean sensitive)
{
        BuohComicManager *cm;

        cm = buoh_comic_list_get_selected (BUOH_COMIC_LIST (window->comic_list));

        buoh_window_set_sensitive (window, "go-previous",
                                   sensitive ?
                                   !buoh_comic_manager_is_the_first (cm) :
                                   sensitive);
        buoh_window_set_sensitive (window, "go-next",
                                   sensitive ?
                                   !buoh_comic_manager_is_the_last (cm) :
                                   sensitive);
        buoh_window_set_sensitive (window, "go-first",
                                   sensitive ?
                                   !buoh_comic_manager_is_the_first (cm) :
                                   sensitive);
        buoh_window_set_sensitive (window, "go-last",
                                   sensitive ?
                                   !buoh_comic_manager_is_the_last (cm) :
                                   sensitive);
}

static void
buoh_window_comic_actions_set_sensitive (BuohWindow *window, gboolean sensitive)
{
        buoh_window_comic_browsing_actions_set_sensitive (window, sensitive);

        buoh_window_set_sensitive (window, "comic-properties", sensitive);
        buoh_window_set_sensitive (window, "comic-copy-uri",    sensitive);
        buoh_window_set_sensitive (window, "view-zoom-in",
                                   buoh_view_is_max_zoom (BUOH_VIEW (window->view)) ?
                                   FALSE : sensitive);
        buoh_window_set_sensitive (window, "view-zoom-out",
                                   buoh_view_is_min_zoom (BUOH_VIEW (window->view)) ?
                                   FALSE : sensitive);
        buoh_window_set_sensitive (window, "view-zoom-normal",
                                   buoh_view_is_normal_size (BUOH_VIEW (window->view)) ?
                                   FALSE : sensitive);
        buoh_window_set_sensitive (window, "view-zoom-mode", sensitive);
}

static void
buoh_window_comic_save_to_disk_set_sensitive (BuohWindow *window, gboolean sensitive)
{
        gboolean save_disabled = FALSE;

        if (g_settings_get_boolean (window->lockdown_settings,
                                    GS_LOCKDOWN_SAVE)) {
                save_disabled = TRUE;
        }

        buoh_window_set_sensitive (window, "comic-save-a-copy",
                                   (save_disabled) ?  FALSE : sensitive);
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

        cm = buoh_comic_list_get_selected (BUOH_COMIC_LIST (window->comic_list));

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
        GAction          *action;

        zoom_mode = buoh_view_get_zoom_mode (BUOH_VIEW (window->view));

        action = g_action_map_lookup_action (G_ACTION_MAP (window),
                                             "view-zoom-mode");
        GEnumClass *enum_class = g_type_class_ref (BUOH_TYPE_VIEW_ZOOM_MODE);
        g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_string (g_enum_get_value (enum_class, zoom_mode)->value_nick));
        g_type_class_unref (enum_class);
}

static void
buoh_window_view_zoom_change_cb (BuohView *view, gpointer gdata)
{
        BuohWindow *window = BUOH_WINDOW (gdata);

        buoh_window_set_sensitive (window, "view-zoom-in",
                                   !buoh_view_is_max_zoom (BUOH_VIEW (window->view)));
        buoh_window_set_sensitive (window, "view-zoom-out",
                                   !buoh_view_is_min_zoom (BUOH_VIEW (window->view)));
        buoh_window_set_sensitive (window, "view-zoom-normal",
                                   !buoh_view_is_normal_size (BUOH_VIEW (window->view)));

        buoh_window_update_zoom_mode (window);

        gtk_widget_grab_focus (GTK_WIDGET (view));
}

static gboolean
buoh_window_comic_list_button_press_cb (GtkWidget *widget, GdkEventButton *event, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        GtkTreeSelection *selection;

        if (event->button == 3) {
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
                if (gtk_tree_selection_get_selected (selection, NULL, NULL)) {
                        gtk_menu_popup_at_pointer (GTK_MENU (window->list_popup), (const GdkEvent *) event);
                        return TRUE;
                }
        }

        return FALSE;
}

static gboolean
buoh_window_comic_list_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer gdata)
{
        BuohWindow       *window = BUOH_WINDOW (gdata);
        GtkTreeSelection *selection;
        guint             state;

        state = event->state & GDK_SHIFT_MASK;

        if (state == GDK_SHIFT_MASK &&
            event->keyval == GDK_KEY_F10) {
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
                if (gtk_tree_selection_get_selected (selection, NULL, NULL)) {
                        gtk_menu_popup_at_pointer (GTK_MENU (window->list_popup), (const GdkEvent *) event);
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
                buoh_window_set_sensitive (window, "comic-remove", TRUE);
        } else {
                gtk_window_set_title (GTK_WINDOW (window), "Buoh");
                buoh_window_set_sensitive (window, "comic-remove", FALSE);
        }
}

static gboolean
buoh_window_comic_view_button_press_cb (GtkWidget *widget, GdkEventButton *event, gpointer gdata)
{
        BuohWindow     *window = BUOH_WINDOW (gdata);
        BuohViewStatus  view_status;

        view_status = buoh_view_get_status (BUOH_VIEW (window->view));
        if (view_status != STATE_COMIC_LOADING &&
            view_status != STATE_COMIC_LOADED) {
                return FALSE;
        }

        if (event->button == 3) {
                gtk_menu_popup_at_pointer (GTK_MENU (window->view_popup), (const GdkEvent *) event);
                return TRUE;
        }

        return FALSE;
}

static gboolean
buoh_window_comic_view_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer gdata)
{
        BuohWindow     *window = BUOH_WINDOW (gdata);
        guint           state;
        BuohViewStatus  view_status;

        view_status = buoh_view_get_status (BUOH_VIEW (window->view));
        if (view_status != STATE_COMIC_LOADING &&
            view_status != STATE_COMIC_LOADED) {
                return FALSE;
        }

        state = event->state & GDK_SHIFT_MASK;

        if (state == GDK_SHIFT_MASK &&
            event->keyval == GDK_KEY_F10) {
                gtk_menu_popup_at_pointer (GTK_MENU (window->view_popup), (const GdkEvent *) event);

                return TRUE;
        }

        return FALSE;
}
