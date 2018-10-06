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

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "buoh.h"
#include "buoh-view.h"
#include "buoh-view-comic.h"
#include "buoh-view-message.h"

enum {
        PROP_0,
        PROP_STATUS
};

enum {
        VIEW_PAGE_IMAGE,
        VIEW_PAGE_MESSAGE,
        VIEW_PAGE_EMPTY
};

enum {
        SCALE_CHANGED,
        N_SIGNALS
};

struct _BuohView {
        GtkNotebook      parent;

        GtkWidget       *message;
        GtkWidget       *comic;

        BuohViewStatus   status;
};

static guint buoh_view_signals[N_SIGNALS];

static void     buoh_view_init               (BuohView       *buoh_view);
static void     buoh_view_class_init         (BuohViewClass  *klass);
static void     buoh_view_set_property       (GObject        *object,
                                              guint           prop_id,
                                              const GValue   *value,
                                              GParamSpec     *pspec);
static void     buoh_view_get_property       (GObject        *object,
                                              guint           prop_id,
                                              GValue         *value,
                                              GParamSpec     *pspec);
static void     buoh_view_grab_focus         (GtkWidget      *widget);
static gboolean buoh_view_button_press_event (GtkWidget      *widget,
                                              GdkEventButton *event);
static void     buoh_view_status_changed_cb  (GObject        *object,
                                              GParamSpec     *arg,
                                              gpointer        gdata);
static void     buoh_view_scale_changed_cb   (GObject        *object,
                                              GParamSpec     *arg,
                                              gpointer        gdata);

G_DEFINE_TYPE (BuohView, buoh_view, GTK_TYPE_NOTEBOOK)

GType
buoh_view_status_get_type (void)
{
        static GType etype = 0;

        if (G_UNLIKELY (etype == 0)) {
                static const GEnumValue values[] = {
                        { STATE_MESSAGE_WELCOME, "STATE_MESSAGE_WELCOME", "welcome" },
                        { STATE_MESSAGE_ERROR,   "STATE_MESSAGE_ERROR",   "error"   },
                        { STATE_COMIC_LOADING,   "STATE_COMIC_LOADING",   "loading" },
                        { STATE_COMIC_LOADED,    "STATE_COMIC_LOADED",    "loaded"  },
                        { STATE_EMPTY,           "STATE_EMPTY",           "empty"   },
                        { 0, NULL, NULL }
                };

                etype = g_enum_register_static ("BuohViewStatus", values);
        }

        return etype;
}

GType
buoh_view_zoom_mode_get_type (void)
{
        static GType etype = 0;

        if (G_UNLIKELY (etype == 0)) {
                static const GEnumValue values[] = {
                        { VIEW_ZOOM_FREE,      "VIEW_ZOOM_FREE",      "free" },
                        { VIEW_ZOOM_BEST_FIT,  "VIEW_ZOOM_BEST_FIT",  "best-fit" },
                        { VIEW_ZOOM_FIT_WIDTH, "VIEW_ZOOM_FIT_WIDTH", "fit-width" },
                        { 0, NULL, NULL }
                };

                etype = g_enum_register_static ("BuohViewZoomMode", values);
        }

        return etype;
}

static void
buoh_view_init (BuohView *buoh_view)
{
        GtkWidget *label;
        GtkWidget *swindow;

        gtk_widget_set_can_focus (GTK_WIDGET (buoh_view), TRUE);

        buoh_view->status = STATE_MESSAGE_WELCOME;

        /* Image view */
        swindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
        buoh_view->comic = buoh_view_comic_new (buoh_view);
        gtk_container_add (GTK_CONTAINER (swindow),
                           buoh_view->comic);
        gtk_widget_show (buoh_view->comic);

        gtk_notebook_insert_page (GTK_NOTEBOOK (buoh_view), swindow,
                                  NULL, VIEW_PAGE_IMAGE);
        gtk_widget_show (swindow);

        /* Message view */
        swindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                        GTK_POLICY_NEVER,
                                        GTK_POLICY_NEVER);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swindow),
                                             GTK_SHADOW_NONE);
        buoh_view->message = buoh_view_message_new ();
        buoh_view_message_set_title (BUOH_VIEW_MESSAGE (buoh_view->message),
                                     _("Buoh online comic strips reader"));
        buoh_view_message_set_text (BUOH_VIEW_MESSAGE (buoh_view->message),
                                    _("Welcome to <b>Buoh</b>, the online comics reader for GNOME Desktop.\n"
                                      "The list on the left panel contains your favorite comic strips "
                                      "to add or remove comics to the list click on Comic → Add. "
                                      "Just select a comic from the list, and it will be displayed "
                                      "on the right side. Thanks for using Buoh."));
        buoh_view_message_set_icon (BUOH_VIEW_MESSAGE (buoh_view->message), "buoh");
        gtk_container_add (GTK_CONTAINER (swindow),
                           buoh_view->message);
        gtk_widget_show (buoh_view->message);

        gtk_notebook_insert_page (GTK_NOTEBOOK (buoh_view), swindow,
                                  NULL, VIEW_PAGE_MESSAGE);
        gtk_widget_show (swindow);

        /* Empty view */
        label = gtk_label_new (NULL);
        gtk_notebook_insert_page (GTK_NOTEBOOK (buoh_view), label,
                                  NULL, VIEW_PAGE_EMPTY);
        gtk_widget_show (label);


        gtk_notebook_set_current_page (GTK_NOTEBOOK (buoh_view), VIEW_PAGE_MESSAGE);

        /* Callbacks */
        g_signal_connect (G_OBJECT (buoh_view), "notify::status",
                          G_CALLBACK (buoh_view_status_changed_cb),
                          NULL);
        g_signal_connect (G_OBJECT (buoh_view->comic),
                          "notify::scale",
                          G_CALLBACK (buoh_view_scale_changed_cb),
                          (gpointer) buoh_view);

        gtk_widget_show (GTK_WIDGET (buoh_view));
}

static void
buoh_view_class_init (BuohViewClass *klass)
{
        GObjectClass   *object_class = G_OBJECT_CLASS (klass);
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        object_class->set_property = buoh_view_set_property;
        object_class->get_property = buoh_view_get_property;

        widget_class->grab_focus = buoh_view_grab_focus;
        widget_class->button_press_event = buoh_view_button_press_event;

        /* Properties */
        g_object_class_install_property (object_class,
                                         PROP_STATUS,
                                         g_param_spec_enum ("status",
                                                            "Status",
                                                            "The current view status",
                                                            BUOH_TYPE_VIEW_STATUS,
                                                            STATE_MESSAGE_WELCOME,
                                                            G_PARAM_READWRITE));

        /* Signals */
        buoh_view_signals[SCALE_CHANGED] =
                g_signal_new ("scale-changed",
                              G_TYPE_FROM_CLASS (object_class),
                              G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                              NULL,
                              NULL,
                              NULL,
                              g_cclosure_marshal_VOID__VOID,
                              G_TYPE_NONE,
                              0);
}

static void
buoh_view_set_property (GObject       *object,
                        guint          prop_id,
                        const GValue  *value,
                        GParamSpec    *pspec)
{
        BuohView *view = BUOH_VIEW (object);

        switch (prop_id) {
        case PROP_STATUS:
                view->status = g_value_get_enum (value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static void
buoh_view_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
        BuohView *view = BUOH_VIEW (object);

        switch (prop_id) {
        case PROP_STATUS:
                g_value_set_enum (value, view->status);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static void
buoh_view_grab_focus (GtkWidget *widget)
{
        BuohView *view = BUOH_VIEW (widget);

        switch (view->status) {
        case STATE_MESSAGE_WELCOME:
        case STATE_MESSAGE_ERROR:
                gtk_widget_grab_focus (view->message);
                break;
        case STATE_COMIC_LOADING:
        case STATE_COMIC_LOADED:
                gtk_widget_grab_focus (view->comic);
                break;
        default:
                break;
        }
}

static gboolean
buoh_view_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
        if (!gtk_widget_has_focus (widget)) {
                gtk_widget_grab_focus (widget);
        }

        return FALSE;
}

GtkWidget *
buoh_view_new (void)
{
        GtkWidget *buoh_view;

        buoh_view = GTK_WIDGET (g_object_new (BUOH_TYPE_VIEW,
                                              "show-tabs", FALSE,
                                              NULL));
        return buoh_view;
}

static void
buoh_view_status_changed_cb (GObject *object, GParamSpec *arg, gpointer gdata)
{
        BuohView *view = BUOH_VIEW (object);

        switch (view->status) {
        case STATE_MESSAGE_WELCOME:
        case STATE_MESSAGE_ERROR:
                gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_MESSAGE);
                break;
        case STATE_COMIC_LOADING:
        case STATE_COMIC_LOADED:
                gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_IMAGE);
                break;
        case STATE_EMPTY:
                gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_EMPTY);
                break;
        default:
                break;
        }
}

static void
buoh_view_scale_changed_cb (GObject *object, GParamSpec *arg, gpointer gdata)
{
        BuohView *view = BUOH_VIEW (gdata);

        g_signal_emit (view, buoh_view_signals[SCALE_CHANGED], 0);
}

gboolean
buoh_view_is_min_zoom (BuohView *view)
{
        g_return_val_if_fail (BUOH_IS_VIEW (view), FALSE);

        return buoh_view_comic_is_min_zoom (BUOH_VIEW_COMIC (view->comic));
}

gboolean
buoh_view_is_max_zoom (BuohView *view)
{
        g_return_val_if_fail (BUOH_IS_VIEW (view), FALSE);

        return buoh_view_comic_is_max_zoom (BUOH_VIEW_COMIC (view->comic));
}

gboolean
buoh_view_is_normal_size (BuohView *view)
{
        g_return_val_if_fail (BUOH_IS_VIEW (view), FALSE);

        return buoh_view_comic_is_normal_size (BUOH_VIEW_COMIC (view->comic));
}

void
buoh_view_zoom_in (BuohView *view)
{
        g_return_if_fail (BUOH_IS_VIEW (view));

        buoh_view_comic_zoom_in (BUOH_VIEW_COMIC (view->comic));
}

void
buoh_view_zoom_out (BuohView *view)
{
        g_return_if_fail (BUOH_IS_VIEW (view));

        buoh_view_comic_zoom_out (BUOH_VIEW_COMIC (view->comic));
}

void
buoh_view_zoom_normal_size (BuohView *view)
{
        g_return_if_fail (BUOH_IS_VIEW (view));

        buoh_view_comic_normal_size (BUOH_VIEW_COMIC (view->comic));
}

void
buoh_view_zoom_best_fit (BuohView *view)
{
        g_return_if_fail (BUOH_IS_VIEW (view));

        buoh_view_comic_best_fit (BUOH_VIEW_COMIC (view->comic));
}

void
buoh_view_zoom_fit_width (BuohView *view)
{
        g_return_if_fail (BUOH_IS_VIEW (view));

        buoh_view_comic_fit_width (BUOH_VIEW_COMIC (view->comic));
}

BuohViewZoomMode
buoh_view_get_zoom_mode (BuohView *view)
{
        g_return_val_if_fail (BUOH_IS_VIEW (view), 0);

        return buoh_view_comic_get_zoom_mode (BUOH_VIEW_COMIC (view->comic));
}

void
buoh_view_set_zoom_mode (BuohView        *view,
                         BuohViewZoomMode mode)
{
        g_return_if_fail (BUOH_IS_VIEW (view));

        buoh_view_comic_set_zoom_mode (BUOH_VIEW_COMIC (view->comic),
                                       mode);
}

BuohViewStatus
buoh_view_get_status (BuohView *view)
{
        g_return_val_if_fail (BUOH_IS_VIEW (view), 0);

        return view->status;
}

void
buoh_view_set_comic (BuohView *view, const BuohComic *comic)
{
        g_return_if_fail (BUOH_IS_VIEW (view));
        g_return_if_fail (BUOH_IS_COMIC (comic));

        g_object_set (G_OBJECT (view->comic),
                      "comic", comic,
                      NULL);
}

BuohComic *
buoh_view_get_comic (BuohView *view)
{
        BuohComic *comic = NULL;

        g_return_val_if_fail (BUOH_IS_VIEW (view), NULL);

        g_object_get (G_OBJECT (view->comic),
                      "comic", &comic,
                      NULL);

        return comic;
}

void
buoh_view_set_message_title (BuohView *view, const gchar *title)
{
        g_return_if_fail (BUOH_IS_VIEW (view));
        g_return_if_fail (title != NULL);

        buoh_view_message_set_title (BUOH_VIEW_MESSAGE (view->message),
                                     title);
}

void
buoh_view_set_message_text (BuohView *view, const gchar *text)
{
        g_return_if_fail (BUOH_IS_VIEW (view));
        g_return_if_fail (text != NULL);

        buoh_view_message_set_text (BUOH_VIEW_MESSAGE (view->message),
                                    text);
}

void
buoh_view_set_message_icon (BuohView *view, const gchar *icon)
{
        g_return_if_fail (BUOH_IS_VIEW (view));
        g_return_if_fail (icon != NULL);

        buoh_view_message_set_icon (BUOH_VIEW_MESSAGE (view->message),
                                    icon);
}

void
buoh_view_clear (BuohView *view)
{
        g_return_if_fail (BUOH_IS_VIEW (view));

        g_object_set (G_OBJECT (view),
                      "status", STATE_EMPTY,
                      NULL);
}
