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

#include "buoh-application.h"
#include "buoh-view.h"
#include "buoh-view-comic.h"
#include "buoh-view-message.h"

enum {
        PROP_0,
        PROP_STATUS
};

enum {
        SCALE_CHANGED,
        N_SIGNALS
};

/**
 * BuohView:
 *
 * A widget controlling the main application view. Will either display a comic strip or a message page.
 */

struct _BuohView {
        GtkStack         parent;

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

G_DEFINE_TYPE (BuohView, buoh_view, GTK_TYPE_STACK)

static void
buoh_view_init (BuohView *buoh_view)
{
        buoh_view->status = STATE_MESSAGE_WELCOME;

        g_type_ensure (BUOH_TYPE_VIEW_COMIC);
        g_type_ensure (BUOH_TYPE_VIEW_MESSAGE);
        gtk_widget_init_template (GTK_WIDGET (buoh_view));

        /* Comic view */
        buoh_view_comic_setup (BUOH_VIEW_COMIC (buoh_view->comic), buoh_view);

        /* Message view */
        buoh_view_message_set_title (BUOH_VIEW_MESSAGE (buoh_view->message),
                                     _("Buoh online comic strips reader"));
        buoh_view_message_set_text (BUOH_VIEW_MESSAGE (buoh_view->message),
                                    _("Welcome to <b>Buoh</b>, the online comics reader for GNOME Desktop.\n"
                                      "The list on the left panel contains your favorite comic strips "
                                      "to add or remove comics to the list click on Comic → Add. "
                                      "Just select a comic from the list, and it will be displayed "
                                      "on the right side. Thanks for using Buoh."));
        buoh_view_message_set_icon (BUOH_VIEW_MESSAGE (buoh_view->message), "buoh");

        gtk_stack_set_visible_child_name (GTK_STACK (buoh_view), "message");

        /* Callbacks */
        g_signal_connect (G_OBJECT (buoh_view->comic),
                          "notify::scale",
                          G_CALLBACK (buoh_view_scale_changed_cb),
                          (gpointer) buoh_view);
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
                              0,
                              NULL,
                              NULL,
                              g_cclosure_marshal_VOID__VOID,
                              G_TYPE_NONE,
                              0);

        gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/buoh/ui/view.ui");

        gtk_widget_class_bind_template_child (widget_class, BuohView, message);
        gtk_widget_class_bind_template_child (widget_class, BuohView, comic);

        gtk_widget_class_bind_template_callback (widget_class, buoh_view_status_changed_cb);
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
                gtk_stack_set_visible_child_name (GTK_STACK (view), "message");
                break;
        case STATE_COMIC_LOADING:
        case STATE_COMIC_LOADED:
                gtk_stack_set_visible_child_name (GTK_STACK (view), "image");
                break;
        case STATE_EMPTY:
                gtk_stack_set_visible_child_name (GTK_STACK (view), "empty");
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
buoh_view_set_comic (BuohView *view, BuohComic *comic)
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
