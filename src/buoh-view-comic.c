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
 *  Authors: Carlos García Campos <carlosgc@gnome.org>
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

#include "buoh-application.h"
#include "buoh-view-comic.h"
#include "buoh-comic-loader.h"

enum {
        PROP_0,
        PROP_COMIC,
        PROP_ZOOM_MODE,
        PROP_SCALE
};

/**
 * BuohViewComic:
 * @view: The parent widget showing this one.
 * @image: #GtkImage widget that will display the comic strip.
 * @comic_data: Raw image data for the currently displayed comic strip.
 * @comic: An object representing the currently displayed comic strip.
 * @comic_loader: A class that fetches image data for given comic.
 * @pixbuf_loader: #GdkPixbufLoader responsible for parsing the raw data and returning #GdkPixbuf.
 *
 * A widget showing a comic strip.
 */

struct _BuohViewComic {
        GtkViewport      parent;

        BuohView        *view;
        GtkWidget       *image;
        GString         *comic_data;

        BuohComic       *comic;
        BuohViewZoomMode zoom_mode;
        gdouble          scale;

        guint update_zoom_source_id;
        GMutex set_image_from_loader_source_id_mutex;
        guint set_image_from_loader_source_id;

        BuohComicLoader *comic_loader;
        GdkPixbufLoader *pixbuf_loader;
};

static const GtkTargetEntry targets[] = {
        { "text/uri-list", 0, 0 },
        { "x-url/http",    0, 0 }
};

#define ZOOM_IN_FACTOR  1.2
#define ZOOM_OUT_FACTOR (1.0 / ZOOM_IN_FACTOR)

#define MIN_ZOOM_SCALE 0.6
#define MAX_ZOOM_SCALE 4

#define DATA_SIZE 61440 /* 60K */

static void     buoh_view_comic_init                  (BuohViewComic *m_view);
static void     buoh_view_comic_class_init            (BuohViewComicClass *klass);
static void     buoh_view_comic_finalize              (GObject          *object);
static void     buoh_view_comic_dispose               (GObject          *object);
static void     buoh_view_comic_set_property          (GObject          *object,
                                                       guint             prop_id,
                                                       const GValue     *value,
                                                       GParamSpec       *pspec);
static void     buoh_view_comic_get_property          (GObject          *object,
                                                       guint             prop_id,
                                                       GValue           *value,
                                                       GParamSpec       *pspec);
static gboolean buoh_view_comic_key_press_event       (GtkWidget        *widget,
                                                       GdkEventKey      *event);
static gboolean buoh_view_comic_scroll_event          (GtkWidget        *widget,
                                                       GdkEventScroll   *event);
static void     buoh_view_comic_size_allocate         (GtkWidget        *widget,
                                                       GtkAllocation    *allocation);
static void     buoh_view_comic_drag_begin            (GtkWidget        *widget,
                                                       GdkDragContext   *drag_context,
                                                       gpointer          gdata);
static void     buoh_view_comic_drag_data_get         (GtkWidget        *widget,
                                                       GdkDragContext   *drag_context,
                                                       GtkSelectionData *data,
                                                       guint             info,
                                                       guint             time,
                                                       gpointer          gdata);
static void     bouh_view_comic_changed_comic_cb      (GObject          *object,
                                                       GParamSpec       *arg,
                                                       gpointer          gdata);
static void     bouh_view_comic_view_status_changed   (GObject          *object,
                                                       GParamSpec       *arg,
                                                       gpointer          gdata);
static void     buoh_view_comic_prepare_load          (BuohViewComic    *c_view);
static void     buoh_view_comic_load_finished         (BuohViewComic    *c_view,
                                                       gpointer          gdata);
static void     buoh_view_comic_load                  (BuohViewComic    *c_view);
static gdouble  buoh_view_comic_get_scale_for_width   (BuohViewComic    *c_view,
                                                       gint              width);
static gdouble  buoh_view_comic_get_scale_for_height  (BuohViewComic    *c_view,
                                                       gint              height);
static void     buoh_view_comic_zoom                  (BuohViewComic    *c_view,
                                                       gdouble           factor,
                                                       gboolean          relative);

G_DEFINE_FINAL_TYPE (BuohViewComic, buoh_view_comic, GTK_TYPE_VIEWPORT)

static void
buoh_view_comic_init (BuohViewComic *c_view)
{
        c_view->zoom_mode = VIEW_ZOOM_FIT_WIDTH;
        c_view->scale = 1.0;
        c_view->comic_loader = buoh_comic_loader_new ();
        c_view->comic_data = g_string_sized_new (DATA_SIZE);

        c_view->update_zoom_source_id = 0;
        g_mutex_init (&c_view->set_image_from_loader_source_id_mutex);
        c_view->set_image_from_loader_source_id = 0;

        gtk_widget_init_template (GTK_WIDGET (c_view));

        g_signal_connect_swapped (G_OBJECT (c_view->comic_loader),
                                  "finished",
                                  G_CALLBACK (buoh_view_comic_load_finished),
                                  (gpointer) c_view);
}

static void
buoh_view_comic_class_init (BuohViewComicClass *klass)
{
        GObjectClass   *object_class = G_OBJECT_CLASS (klass);
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        object_class->set_property = buoh_view_comic_set_property;
        object_class->get_property = buoh_view_comic_get_property;

        widget_class->key_press_event = buoh_view_comic_key_press_event;
        widget_class->scroll_event = buoh_view_comic_scroll_event;
        widget_class->size_allocate = buoh_view_comic_size_allocate;

        /* Properties */
        g_object_class_install_property (object_class,
                                         PROP_COMIC,
                                         g_param_spec_pointer ("comic",
                                                               "Comic",
                                                               "The current comic",
                                                               G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_ZOOM_MODE,
                                         g_param_spec_enum ("zoom-mode",
                                                            "ZoomMode",
                                                            "The view zoom mode",
                                                            BUOH_TYPE_VIEW_ZOOM_MODE,
                                                            VIEW_ZOOM_FIT_WIDTH,
                                                            G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_SCALE,
                                         g_param_spec_double ("scale",
                                                              "Scale",
                                                              "Current view scale",
                                                              G_MINDOUBLE,
                                                              G_MAXDOUBLE,
                                                              1.0,
                                                              G_PARAM_READWRITE));

        object_class->finalize = buoh_view_comic_finalize;
        object_class->dispose = buoh_view_comic_dispose;

        gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/buoh/ui/view-comic.ui");

        gtk_widget_class_bind_template_child (widget_class, BuohViewComic, image);

        gtk_widget_class_bind_template_callback (widget_class, bouh_view_comic_changed_comic_cb);
        gtk_widget_class_bind_template_callback (widget_class, buoh_view_comic_drag_begin);
        gtk_widget_class_bind_template_callback (widget_class, buoh_view_comic_drag_data_get);
}

static void
buoh_view_comic_finalize (GObject *object)
{
        BuohViewComic *c_view = BUOH_VIEW_COMIC (object);

        buoh_debug ("buoh-view-comic finalize");

        if (c_view->comic_data) {
                g_string_free (c_view->comic_data, TRUE);
                c_view->comic_data = NULL;
        }

        if (c_view->pixbuf_loader) {
                gdk_pixbuf_loader_close (c_view->pixbuf_loader, NULL);
                g_clear_object (&c_view->pixbuf_loader);
        }

        g_mutex_clear (&c_view->set_image_from_loader_source_id_mutex);

        if (G_OBJECT_CLASS (buoh_view_comic_parent_class)->finalize) {
                (* G_OBJECT_CLASS (buoh_view_comic_parent_class)->finalize) (object);
        }
}

static void
buoh_view_comic_dispose (GObject *object)
{
        BuohViewComic *c_view = BUOH_VIEW_COMIC (object);

        if (c_view->comic_loader) {
                g_signal_handlers_disconnect_by_func (c_view->comic_loader,
                                                      buoh_view_comic_load_finished,
                                                      c_view);
                g_clear_object (&c_view->comic_loader);
        }

        if (G_OBJECT_CLASS (buoh_view_comic_parent_class)->dispose) {
                (* G_OBJECT_CLASS (buoh_view_comic_parent_class)->dispose) (object);
        }
}

static void
buoh_view_comic_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
        BuohViewComic *c_view = BUOH_VIEW_COMIC (object);

        switch (prop_id) {
        case PROP_COMIC:
                c_view->comic = BUOH_COMIC (g_value_get_pointer (value));

                break;
        case PROP_ZOOM_MODE:
                c_view->zoom_mode = g_value_get_enum (value);

                break;
        case PROP_SCALE:
                c_view->scale = g_value_get_double (value);

                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static void
buoh_view_comic_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
        BuohViewComic *c_view = BUOH_VIEW_COMIC (object);

        switch (prop_id) {
        case PROP_COMIC:
                g_value_set_pointer (value, c_view->comic);

                break;
        case PROP_ZOOM_MODE:
                g_value_set_enum (value, c_view->zoom_mode);

                break;
        case PROP_SCALE:
                g_value_set_double (value, c_view->scale);

                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static gboolean
buoh_view_comic_key_press_event (GtkWidget *widget, GdkEventKey *event)
{
        BuohViewComic *c_view = BUOH_VIEW_COMIC (widget);
        GtkAdjustment *adjustment;
        gdouble        value;

        switch (event->keyval) {
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
                g_object_get (G_OBJECT (c_view),
                              "vadjustment", &adjustment,
                              NULL);
                value = gtk_adjustment_get_value (adjustment) -
                        gtk_adjustment_get_step_increment (adjustment);

                break;
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
                g_object_get (G_OBJECT (c_view),
                              "vadjustment", &adjustment,
                              NULL);
                value = gtk_adjustment_get_value (adjustment) +
                        gtk_adjustment_get_step_increment (adjustment);

                break;
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
                g_object_get (G_OBJECT (c_view),
                              "hadjustment", &adjustment,
                              NULL);
                value = gtk_adjustment_get_value (adjustment) -
                        gtk_adjustment_get_step_increment (adjustment);

                break;
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
                g_object_get (G_OBJECT (c_view),
                              "hadjustment", &adjustment,
                              NULL);
                value = gtk_adjustment_get_value (adjustment) +
                        gtk_adjustment_get_step_increment (adjustment);

                break;
        default:
                return FALSE;
        }

        value = CLAMP (value,
                       gtk_adjustment_get_lower (adjustment),
                       gtk_adjustment_get_upper (adjustment) -
                       gtk_adjustment_get_page_size (adjustment));
        gtk_adjustment_set_value (adjustment, value);

        return TRUE;
}

static gboolean
buoh_view_comic_scroll_event (GtkWidget *widget, GdkEventScroll *event)
{
        BuohViewComic   *c_view = BUOH_VIEW_COMIC (widget);
        GdkModifierType  modifiers;

        modifiers = gtk_accelerator_get_default_mod_mask ();

        if ((event->state & modifiers) == GDK_CONTROL_MASK) {
                switch (event->direction) {
                case GDK_SCROLL_UP:
                case GDK_SCROLL_LEFT:
                        buoh_view_comic_zoom_in (c_view);
                        break;
                case GDK_SCROLL_DOWN:
                case GDK_SCROLL_RIGHT:
                        buoh_view_comic_zoom_out (c_view);
                        break;
                case GDK_SCROLL_SMOOTH:
                {
                        gdouble delta_x = 0;
                        gdouble delta_y = 0;
                        gdk_event_get_scroll_deltas ((GdkEvent *) event, &delta_x, &delta_y);
                        if (delta_y < 0) {
                                buoh_view_comic_zoom_in (c_view);
                        } else if (delta_y > 0) {
                                buoh_view_comic_zoom_out (c_view);
                        }
                        break;
                }
                }

                return TRUE;
        }

        return FALSE;
}

static gboolean
buoh_view_comic_update_zoom_cb (BuohViewComic *c_view)
{
        GdkPixbuf *pixbuf;
        gdouble    new_scale;

        if (!g_source_is_destroyed (g_main_current_source ())) {
                c_view->update_zoom_source_id = 0;
        }

        pixbuf = buoh_comic_get_pixbuf (c_view->comic);
        if (!pixbuf) {
                return G_SOURCE_REMOVE;
        }

        switch (c_view->zoom_mode) {
        case VIEW_ZOOM_FREE:
                new_scale = c_view->scale;
                break;
        case VIEW_ZOOM_BEST_FIT: {
                gdouble scale_width;
                gdouble scale_height;

                scale_width =
                        buoh_view_comic_get_scale_for_width (c_view,
                                                             gdk_pixbuf_get_width (pixbuf));
                scale_height =
                        buoh_view_comic_get_scale_for_height (c_view,
                                                              gdk_pixbuf_get_height (pixbuf));

                new_scale = MIN (scale_width, scale_height);
        }
                break;
        case VIEW_ZOOM_FIT_WIDTH:
                new_scale =
                        buoh_view_comic_get_scale_for_width (c_view,
                                                             gdk_pixbuf_get_width (pixbuf));
                break;
        default:
                g_assert_not_reached ();
        }

        if (new_scale != c_view->scale) {
                buoh_view_comic_zoom (c_view, new_scale, FALSE);
        }

        return G_SOURCE_REMOVE;
}

static void
buoh_view_comic_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
        BuohViewComic *c_view = BUOH_VIEW_COMIC (widget);

        if (c_view->comic) {
                if (c_view->update_zoom_source_id > 0) {
                        g_source_remove (c_view->update_zoom_source_id);
                }
                c_view->update_zoom_source_id = g_idle_add ((GSourceFunc) buoh_view_comic_update_zoom_cb,
                                                         c_view);
        }

        GTK_WIDGET_CLASS (buoh_view_comic_parent_class)->size_allocate (widget, allocation);
}

GtkWidget *
buoh_view_comic_new (void)
{
        GtkWidget *c_view;

        c_view = GTK_WIDGET (g_object_new (BUOH_TYPE_VIEW_COMIC, NULL));

        return c_view;
}

void
buoh_view_comic_setup (BuohViewComic *c_view, BuohView *view)
{
        c_view->view = view;
        g_signal_connect (G_OBJECT (c_view->view),
                          "notify::status",
                          G_CALLBACK (bouh_view_comic_view_status_changed),
                          (gpointer) c_view);
}

static void
buoh_view_comic_drag_begin (GtkWidget *widget, GdkDragContext *drag_context,
                            gpointer gdata)
{
        BuohViewComic *c_view = BUOH_VIEW_COMIC (widget);
        GdkPixbuf     *thumbnail = NULL;

        thumbnail = buoh_comic_get_thumbnail (c_view->comic);

        if (thumbnail) {
                gtk_drag_source_set_icon_pixbuf (widget, thumbnail);
                g_object_unref (thumbnail);
        }
}

static void
buoh_view_comic_drag_data_get (GtkWidget *widget, GdkDragContext *drag_context,
                               GtkSelectionData *data, guint info, guint time,
                               gpointer gdata)
{
        BuohViewComic  *c_view = BUOH_VIEW_COMIC (widget);
        const gchar    *uri;
        gchar          *uris[2];

        uri = buoh_comic_get_uri (c_view->comic);
        if (uri) {
                uris[0] = g_strdup (uri);
                uris[1] = NULL;
                gtk_selection_data_set_uris (data, uris);
        }
}

static void
buoh_view_comic_size_prepared (GdkPixbufLoader *loader,
                               gint             width,
                               gint             height,
                               BuohViewComic   *c_view)
{
        switch (c_view->zoom_mode) {
        case VIEW_ZOOM_BEST_FIT: {
                gdouble scale_width;
                gdouble scale_height;

                scale_width = buoh_view_comic_get_scale_for_width (c_view, width);

                scale_height = buoh_view_comic_get_scale_for_height (c_view, height);

                c_view->scale = MIN (scale_width, scale_height);
        }
                break;
        case VIEW_ZOOM_FIT_WIDTH: {
                GtkWidget *swindow;

                /* We have to predict if a vscrollbar will be needed
                 * so that we'll have to take it into account
                 */
                swindow = gtk_widget_get_parent (GTK_WIDGET (c_view));
                if (GTK_IS_SCROLLED_WINDOW (swindow)) {
                        GtkAllocation  allocation;
                        GtkStyleContext *style;
                        GtkBorder      padding;
                        gint           scrollbar_spacing;
                        gint           scrollbar_width;
                        gint           widget_width;
                        gint           widget_height;
                        gint           new_scale;

                        gtk_widget_get_allocation (GTK_WIDGET (c_view), &allocation);
                        widget_width = allocation.width;

                        style = gtk_widget_get_style_context (GTK_WIDGET (c_view));
                        gtk_style_context_get_padding (style,
                                                       GTK_STATE_FLAG_NORMAL,
                                                       &padding);
                        widget_width -= padding.left + padding.right;

                        new_scale = (gdouble)widget_width / (gdouble)width;

                        widget_height = allocation.height;

                        if ((height * new_scale) > widget_height) {
                                GtkWidget *vscrollbar;

                                vscrollbar = gtk_scrolled_window_get_vscrollbar (GTK_SCROLLED_WINDOW (swindow));

                                scrollbar_width = gtk_widget_get_allocated_width (vscrollbar);
                                gtk_widget_style_get (swindow,
                                                      "scrollbar_spacing", &scrollbar_spacing,
                                                      NULL);

                                allocation.width -= (scrollbar_width + scrollbar_spacing);
                                gtk_widget_set_allocation (GTK_WIDGET (c_view), &allocation);
                        }
                }

                c_view->scale =
                        buoh_view_comic_get_scale_for_width (c_view, width);
        }
                break;
        default:
                break;
        }
}

static void
buoh_view_comic_prepare_load (BuohViewComic *c_view)
{
        GtkAdjustment *hadjustment;
        GtkAdjustment *vadjustment;

        g_object_get (G_OBJECT (c_view),
                      "hadjustment", &hadjustment,
                      "vadjustment", &vadjustment,
                      NULL);

        gtk_adjustment_set_value (hadjustment, 0.0);
        gtk_adjustment_set_value (vadjustment, 0.0);

        if (gtk_widget_get_realized (GTK_WIDGET (c_view))) {
                gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (c_view)),
                                       NULL);
        }

        gtk_image_clear (GTK_IMAGE (c_view->image));
}

static void
bouh_view_comic_changed_comic_cb (GObject *object, GParamSpec *arg, gpointer gdata)
{
        BuohViewComic *c_view = BUOH_VIEW_COMIC (object);

        /* Cancel current load if needed */
        buoh_comic_loader_cancel (c_view->comic_loader);

        buoh_view_comic_prepare_load (c_view);

        buoh_view_comic_load (c_view);
}

static void
bouh_view_comic_view_status_changed (GObject *object, GParamSpec *arg, gpointer gdata)
{
        BuohViewComic *c_view = BUOH_VIEW_COMIC (gdata);

        if (buoh_view_get_status (c_view->view) == STATE_COMIC_LOADED) {
                gtk_drag_source_set (GTK_WIDGET (c_view),
                                     GDK_BUTTON1_MASK,
                                     targets,
                                     sizeof (targets) / sizeof (targets[0]),
                                     GDK_ACTION_COPY);
        } else {
                gtk_drag_source_unset (GTK_WIDGET (c_view));
        }
}

static void
buoh_view_comic_set_image_from_pixbuf (BuohViewComic *c_view,
                                       GdkPixbuf     *pixbuf)
{
        GdkPixbuf *new_pixbuf;

        g_assert (GDK_IS_PIXBUF (pixbuf));

        if (c_view->scale != 1.0) {
                new_pixbuf = gdk_pixbuf_scale_simple (pixbuf,
                                                      gdk_pixbuf_get_width (pixbuf) * c_view->scale,
                                                      gdk_pixbuf_get_height (pixbuf) * c_view->scale,
                                                      GDK_INTERP_BILINEAR);

                gtk_image_set_from_pixbuf (GTK_IMAGE (c_view->image), new_pixbuf);
                g_object_unref (new_pixbuf);
        } else {
                gtk_image_set_from_pixbuf (GTK_IMAGE (c_view->image), pixbuf);
        }
}

static void
buoh_view_comic_load_finished (BuohViewComic *c_view,
                               gpointer       gdata)
{
        GError    *error = NULL;
        GdkPixbuf *pixbuf;

        buoh_debug ("buoh-view-comic-load finished");

        gdk_pixbuf_loader_close (c_view->pixbuf_loader, NULL);

        gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (c_view)),
                               NULL);

        buoh_comic_loader_get_error (c_view->comic_loader, &error);
        if (error) {
                buoh_view_set_message_title (BUOH_VIEW (c_view->view),
                                             _("Error Loading Comic"));

                buoh_view_set_message_text (BUOH_VIEW (c_view->view),
                                            error->message);
                g_error_free (error);

                buoh_view_set_message_icon (BUOH_VIEW (c_view->view),
                                            "dialog-error");

                g_object_set (G_OBJECT (c_view->view),
                              "status", STATE_MESSAGE_ERROR,
                              NULL);

                g_clear_object (&c_view->pixbuf_loader);

                return;
        }

        pixbuf = gdk_pixbuf_loader_get_pixbuf (c_view->pixbuf_loader);
        if (pixbuf) {
                buoh_view_comic_set_image_from_pixbuf (c_view, pixbuf);

                g_object_set (G_OBJECT (c_view->view),
                              "status", STATE_COMIC_LOADED,
                              NULL);
        }

        if (c_view->comic_data->len > 0) {
                BuohComicImage *comic_image;

                comic_image = g_new0 (BuohComicImage, 1);

                comic_image->size = c_view->comic_data->len;
                comic_image->data = (guchar *) g_memdup2 (c_view->comic_data->str,
                                                         c_view->comic_data->len);

                buoh_comic_set_image (c_view->comic, comic_image);

                c_view->comic_data->len = 0;
        }

        g_clear_object (&c_view->pixbuf_loader);
}

static gboolean
buoh_view_comic_set_image_from_loader (BuohViewComic *c_view)
{
        GdkPixbuf *pixbuf;

        g_mutex_lock (&c_view->set_image_from_loader_source_id_mutex);
        if (!g_source_is_destroyed (g_main_current_source ())) {
                c_view->set_image_from_loader_source_id = 0;
        }
        g_mutex_unlock (&c_view->set_image_from_loader_source_id_mutex);

        pixbuf = gdk_pixbuf_loader_get_pixbuf (c_view->pixbuf_loader);
        if (pixbuf) {
                buoh_view_comic_set_image_from_pixbuf (c_view, pixbuf);
        }

        return G_SOURCE_REMOVE;
}

static void
buoh_view_comic_load_cb (const gchar   *data,
                         guint          len,
                         BuohViewComic *c_view)
{
        GError *error = NULL;

        gdk_pixbuf_loader_write (c_view->pixbuf_loader,
                                 (guchar *)data, len, &error);

        if (error) {
                g_warning ("%s", error->message);
                g_error_free (error);
                return;
        }

        c_view->comic_data = g_string_append_len (c_view->comic_data,
                                                        data, len);

        g_mutex_lock (&c_view->set_image_from_loader_source_id_mutex);
        if (c_view->set_image_from_loader_source_id > 0) {
                g_source_remove (c_view->set_image_from_loader_source_id);
        }
        c_view->set_image_from_loader_source_id = g_idle_add ((GSourceFunc) buoh_view_comic_set_image_from_loader,
                                                           (gpointer) c_view);
        g_mutex_unlock (&c_view->set_image_from_loader_source_id_mutex);
}

static void
buoh_view_comic_load (BuohViewComic *c_view)
{
        GdkPixbuf *pixbuf;

        pixbuf = buoh_comic_get_pixbuf (c_view->comic);
        if (pixbuf) {
                buoh_view_comic_update_zoom_cb (c_view);
                buoh_view_comic_set_image_from_pixbuf (c_view, pixbuf);
                g_object_set (G_OBJECT (c_view->view),
                              "status", STATE_COMIC_LOADED,
                              NULL);
        } else {
                GdkCursor *cursor;

                g_object_set (G_OBJECT (c_view->view),
                              "status", STATE_COMIC_LOADING,
                              NULL);

                cursor = gdk_cursor_new_for_display (gdk_display_get_default (), GDK_WATCH);
                gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (c_view)),
                                       cursor);
                g_object_unref (cursor);

                if (c_view->pixbuf_loader) {
                        gdk_pixbuf_loader_close (c_view->pixbuf_loader, NULL);
                        g_object_unref (c_view->pixbuf_loader);
                }
                c_view->pixbuf_loader = gdk_pixbuf_loader_new ();
                g_signal_connect (G_OBJECT (c_view->pixbuf_loader),
                                  "size-prepared",
                                  G_CALLBACK (buoh_view_comic_size_prepared),
                                  (gpointer) c_view);

                c_view->comic_data->len = 0;

                buoh_comic_loader_load_comic (c_view->comic_loader,
                                              c_view->comic,
                                              (BuohComicLoaderLoadFunc) buoh_view_comic_load_cb,
                                              (gpointer) c_view);
        }
}

static gdouble
buoh_view_comic_get_scale_for_width (BuohViewComic *c_view,
                                     gint           width)
{
        GtkWidget       *widget = GTK_WIDGET (c_view);
        GtkAllocation    allocation;
        GtkStyleContext *style;
        GtkBorder        padding;
        gint             widget_width;
        gdouble          new_scale;

        gtk_widget_get_allocation (widget, &allocation);
        widget_width = allocation.width;

        style = gtk_widget_get_style_context (widget);
        gtk_style_context_get_padding (style,
                                       GTK_STATE_FLAG_NORMAL,
                                       &padding);
        widget_width -= padding.left + padding.right;

        new_scale = (gdouble)widget_width / (gdouble)width;

        return new_scale;
}

static gdouble
buoh_view_comic_get_scale_for_height (BuohViewComic *c_view,
                                      gint           height)
{
        GtkWidget       *widget = GTK_WIDGET (c_view);
        GtkAllocation    allocation;
        GtkStyleContext *style;
        GtkBorder        padding;
        gint             widget_height;
        gdouble          new_scale;

        gtk_widget_get_allocation (widget, &allocation);
        widget_height = allocation.height;

        style = gtk_widget_get_style_context (widget);
        gtk_style_context_get_padding (style,
                                       GTK_STATE_FLAG_NORMAL,
                                       &padding);
        widget_height -= padding.top + padding.bottom;

        new_scale = (gdouble)widget_height / (gdouble)height;

        return new_scale;
}

static void
buoh_view_comic_zoom (BuohViewComic *c_view, gdouble factor, gboolean relative)
{
        gdouble    scale;
        GdkPixbuf *pixbuf = NULL;

        g_assert (BUOH_IS_VIEW_COMIC (c_view));
        g_assert (BUOH_IS_COMIC (c_view->comic));

        if (relative) {
                scale = c_view->scale * factor;
        } else {
                scale = factor;
        }

        if (scale == c_view->scale) {
                return;
        }

        g_object_set (G_OBJECT (c_view),
                      "scale", scale,
                      NULL);

        pixbuf = buoh_comic_get_pixbuf (c_view->comic);
        if (pixbuf) {
                buoh_view_comic_set_image_from_pixbuf (c_view, pixbuf);
        }
}

gboolean
buoh_view_comic_is_min_zoom (BuohViewComic *c_view)
{
        g_return_val_if_fail (BUOH_IS_VIEW_COMIC (c_view), FALSE);

        return c_view->scale <= MIN_ZOOM_SCALE;
}

gboolean
buoh_view_comic_is_max_zoom (BuohViewComic *c_view)
{
        g_return_val_if_fail (BUOH_IS_VIEW_COMIC (c_view), FALSE);

        return c_view->scale >= MAX_ZOOM_SCALE;
}

gboolean
buoh_view_comic_is_normal_size (BuohViewComic *c_view)
{
        g_return_val_if_fail (BUOH_IS_VIEW_COMIC (c_view), FALSE);

        return c_view->scale == 1.0;
}

void
buoh_view_comic_zoom_in (BuohViewComic *c_view)
{
        g_return_if_fail (BUOH_IS_VIEW_COMIC (c_view));

        buoh_view_comic_set_zoom_mode (c_view, VIEW_ZOOM_FREE);
        buoh_view_comic_zoom (c_view, ZOOM_IN_FACTOR, TRUE);
}

void
buoh_view_comic_zoom_out (BuohViewComic *c_view)
{
        g_return_if_fail (BUOH_IS_VIEW_COMIC (c_view));

        buoh_view_comic_set_zoom_mode (c_view, VIEW_ZOOM_FREE);
        buoh_view_comic_zoom (c_view, ZOOM_OUT_FACTOR, TRUE);
}

void
buoh_view_comic_normal_size (BuohViewComic *c_view)
{
        g_return_if_fail (BUOH_IS_VIEW_COMIC (c_view));

        buoh_view_comic_set_zoom_mode (c_view, VIEW_ZOOM_FREE);
        buoh_view_comic_zoom (c_view, 1.0, FALSE);
}

void
buoh_view_comic_best_fit (BuohViewComic *c_view)
{
        g_return_if_fail (BUOH_IS_VIEW_COMIC (c_view));

        buoh_view_comic_set_zoom_mode (c_view, VIEW_ZOOM_BEST_FIT);
        gtk_widget_queue_resize (GTK_WIDGET (c_view));
}

void
buoh_view_comic_fit_width (BuohViewComic *c_view)
{
        g_return_if_fail (BUOH_IS_VIEW_COMIC (c_view));

        buoh_view_comic_set_zoom_mode (c_view, VIEW_ZOOM_FIT_WIDTH);
        gtk_widget_queue_resize (GTK_WIDGET (c_view));
}

BuohViewZoomMode
buoh_view_comic_get_zoom_mode (BuohViewComic *c_view)
{
        g_return_val_if_fail (BUOH_IS_VIEW_COMIC (c_view), 0);

        return c_view->zoom_mode;
}

void
buoh_view_comic_set_zoom_mode (BuohViewComic   *c_view,
                               BuohViewZoomMode mode)
{
        g_return_if_fail (BUOH_IS_VIEW_COMIC (c_view));

        g_object_set (G_OBJECT (c_view),
                      "zoom-mode", mode,
                      NULL);
}
