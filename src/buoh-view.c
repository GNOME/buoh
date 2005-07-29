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

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>

#include "buoh-view.h"

enum {
        PROP_0,
        PROP_COMIC,
        PROP_SCALE,
        PROP_STATUS
};

enum {
        VIEW_PAGE_IMAGE,
        VIEW_PAGE_MESSAGE,
        VIEW_PAGE_EMPTY
};

struct _BuohViewPrivate {
        GtkWidget       *viewport;
        BuohComic       *comic;
        gdouble          scale;
        GdkPixbufLoader *pixbuf_loader;
        gchar           *message;
        BuohViewStatus   status;
};

#define BUOH_VIEW_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_VIEW, BuohViewPrivate))

#define ZOOM_IN_FACTOR  1.5
#define ZOOM_OUT_FACTOR (1.0 / ZOOM_IN_FACTOR)

#define MIN_ZOOM_SCALE (ZOOM_OUT_FACTOR * ZOOM_OUT_FACTOR)
#define MAX_ZOOM_SCALE (ZOOM_IN_FACTOR * ZOOM_IN_FACTOR)

static GtkNotebookClass *parent_class = NULL;

static void buoh_view_init                   (BuohView      *buoh_view);
static void buoh_view_class_init             (BuohViewClass *klass);
static void buoh_view_finalize               (GObject       *object);
static void buoh_view_set_property           (GObject       *object,
                                              guint          prop_id,
                                              const GValue  *value,
                                              GParamSpec    *pspec);
static void buoh_view_get_property           (GObject       *object,
                                              guint          prop_id,
                                              GValue        *value,
                                              GParamSpec    *pspec);

static void buoh_view_destroy                (GtkObject     *object);

static void bouh_view_comic_changed_cb       (GObject       *object,
                                              GParamSpec    *arg,
                                              gpointer       gdata);

static void buoh_view_update                 (BuohView      *view);
static gboolean buoh_view_update_image       (gpointer       gdata);
static void buoh_view_update_message         (BuohView      *view);

static void buoh_view_zoom                   (BuohView      *view,
                                              gdouble        factor,
                                              gboolean       relative);

static void buoh_view_set_image_from_pixbuf  (BuohView      *view,
                                              GtkWidget     *image,
                                              GdkPixbuf     *pixbuf);
static void buoh_view_loading_comic          (GtkWidget     *widget,
                                              gint           x,
                                              gint           y,
                                              gint           width,
                                              gint           height,
                                              gpointer      *gdata);
static gboolean buoh_view_load_comic         (BuohView      *view);

GType
buoh_view_get_type ()
{
        static GType type = 0;

        if (!type) {
                static const GTypeInfo info = {
                        sizeof (BuohViewClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) buoh_view_class_init,
                        NULL,
                        NULL,
                        sizeof (BuohView),
                        0,
                        (GInstanceInitFunc) buoh_view_init
                };

                type = g_type_register_static (GTK_TYPE_NOTEBOOK, "BuohView",
                                               &info, 0);
        }

        return type;
}

GType
buoh_view_status_get_type (void)
{
        static GType type = 0;
        
        if (!type) {
                static const GEnumValue values[] = {
                        { VIEW_STATE_INIT,    "VIEW_STATE_INIT",    "init" },
                        { VIEW_STATE_ERROR,   "VIEW_STATE_ERROR",   "error" },
                        { VIEW_STATE_EMPTY,   "VIEW_STATE_EMPTY",   "empty" },
                        { VIEW_STATE_LOADED,  "VIEW_STATE_LOADED",  "loaded" },
                        { VIEW_STATE_LOADING, "VIEW_STATE_LOADING", "loading" },
                        { 0, NULL, NULL }
                };
                
                type = g_enum_register_static ("BuohViewStatus", values);
        }
        
        return type;
}

static void
buoh_view_init (BuohView *buoh_view)
{
        GtkWidget *image;
        GtkWidget *label;
        GtkWidget *swindow;
        
        buoh_view->priv = BUOH_VIEW_GET_PRIVATE (buoh_view);
        
        buoh_view->priv->scale = 1.0;
        buoh_view->priv->comic = NULL;
        buoh_view->priv->status = VIEW_STATE_INIT;
        buoh_view->priv->pixbuf_loader = NULL;
        
        /* View port */
        swindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
        buoh_view->priv->viewport = gtk_viewport_new (NULL, NULL);
        gtk_container_add (GTK_CONTAINER (swindow),
                           buoh_view->priv->viewport);
        gtk_widget_show (buoh_view->priv->viewport);

        /* Image view */
        image = gtk_image_new ();
        gtk_container_add (GTK_CONTAINER (buoh_view->priv->viewport),
                           GTK_WIDGET (image));
        gtk_widget_show (image);

        gtk_notebook_insert_page (GTK_NOTEBOOK (buoh_view), swindow,
                                  NULL, VIEW_PAGE_IMAGE);
        gtk_widget_show (swindow);

        /* Message view */
        buoh_view->priv->message = g_strdup (_("Welcome to <b>Buoh</b>, a comics reader for GNOME.\n"
                                               "The panel on the left shows your favourite comics. In "
                                               "the right the comic will be displayed when a comic is "
                                               "selected. To add or remove a comic select Add on the menu."));
        label = gtk_label_new (NULL);
        gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
        g_object_set (G_OBJECT (label),
                      "xalign", 0.5,
                      "yalign", 0.5,
                      NULL);
        gtk_label_set_markup (GTK_LABEL (label), buoh_view->priv->message);
        gtk_notebook_insert_page (GTK_NOTEBOOK (buoh_view), label,
                                  NULL, VIEW_PAGE_MESSAGE);
        gtk_widget_show (label);

        /* Empty view */
        label = gtk_label_new (NULL);
        gtk_notebook_insert_page (GTK_NOTEBOOK (buoh_view), label,
                                  NULL, VIEW_PAGE_EMPTY);
        gtk_widget_show (label);

        gtk_notebook_set_current_page (GTK_NOTEBOOK (buoh_view), VIEW_PAGE_MESSAGE);

        /* Callbacks */
        g_signal_connect (G_OBJECT (buoh_view), "notify::comic",
                          G_CALLBACK (bouh_view_comic_changed_cb),
                          NULL);

        gtk_widget_show (GTK_WIDGET (buoh_view));
}

static void
buoh_view_class_init (BuohViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (klass);

        gtk_object_class->destroy = buoh_view_destroy;

        object_class->set_property = buoh_view_set_property;
        object_class->get_property = buoh_view_get_property;
        
        parent_class = g_type_class_peek_parent (klass);

        g_type_class_add_private (klass, sizeof (BuohViewPrivate));

        /* Properties */
        g_object_class_install_property (object_class,
                                         PROP_COMIC,
                                         g_param_spec_object ("comic",
                                                              "Comic",
                                                              "The current comic",
                                                              BUOH_TYPE_COMIC,
                                                              G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_SCALE,
                                         g_param_spec_double ("scale",
                                                              "Scale",
                                                              "Current view scale",
                                                              MIN_ZOOM_SCALE,
                                                              MAX_ZOOM_SCALE,
                                                              1.0,
                                                              G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_STATUS,
                                         g_param_spec_enum ("status",
                                                            "Status",
                                                            "Current View status",
                                                            BUOH_TYPE_VIEW_STATUS,
                                                            VIEW_STATE_INIT,
                                                            G_PARAM_READWRITE));

        object_class->finalize = buoh_view_finalize;
}

static void
buoh_view_finalize (GObject *object)
{
        BuohView *view = BUOH_VIEW (object);
        
        g_return_if_fail (BUOH_IS_VIEW (object));

        g_debug ("buoh-view finalize\n");

        if (view->priv->message) {
                g_free (view->priv->message);
                view->priv->message = NULL;
        }

        if (G_OBJECT_CLASS (parent_class)->finalize)
                (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
buoh_view_destroy (GtkObject *object)
{
        BuohView *view = BUOH_VIEW (object);

        if (view->priv->comic) {
                g_object_unref (view->priv->comic);
                view->priv->comic = NULL;
        }

        if (view->priv->pixbuf_loader) {
                g_object_unref (view->priv->pixbuf_loader);
                view->priv->pixbuf_loader = NULL;
        }

        if (GTK_OBJECT_CLASS (parent_class)->destroy)
                (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
buoh_view_set_property (GObject       *object,
                        guint          prop_id,
                        const GValue  *value,
                        GParamSpec    *pspec)
{
        BuohView *view = BUOH_VIEW (object);
        
        switch (prop_id) {
        case PROP_COMIC:
                if (view->priv->comic) {
                        g_object_unref (view->priv->comic);
                }

                view->priv->comic = BUOH_COMIC (g_value_dup_object (value));

                break;
        case PROP_SCALE:
                view->priv->scale = g_value_get_double (value);

                break;
        case PROP_STATUS:
                view->priv->status = g_value_get_enum (value);

                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static void
buoh_view_get_property (GObject       *object,
                        guint          prop_id,
                        GValue        *value,
                        GParamSpec    *pspec)
{
        BuohView *view = BUOH_VIEW (object);

        switch (prop_id) {
        case PROP_COMIC:
                g_value_set_object (value, view->priv->comic);
                
                break;
        case PROP_SCALE:
                g_value_set_double (value, view->priv->scale);

                break;
        case PROP_STATUS:
                g_value_set_enum (value, view->priv->status);

                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
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
bouh_view_comic_changed_cb (GObject *object, GParamSpec *arg, gpointer gdata)
{
        BuohView *view = BUOH_VIEW (object);

        gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_IMAGE);
        
        buoh_view_update (view);
}

static void
buoh_view_set_image_from_pixbuf (BuohView *view, GtkWidget *image, GdkPixbuf *pixbuf)
{
        GdkPixbuf *new_pixbuf;

        new_pixbuf = gdk_pixbuf_scale_simple (pixbuf,
                                              gdk_pixbuf_get_width (pixbuf) * view->priv->scale,
                                              gdk_pixbuf_get_height (pixbuf) * view->priv->scale,
                                              GDK_INTERP_BILINEAR);
        g_object_unref (pixbuf);

        gtk_image_set_from_pixbuf (GTK_IMAGE (image), new_pixbuf);

        g_object_unref (new_pixbuf);

        gtk_widget_show (image);
}

static void
buoh_view_loading_comic (GtkWidget *widget, gint x, gint y,
                         gint width, gint height, gpointer *gdata)
{
        BuohView  *view = BUOH_VIEW (gdata);
        GtkWidget *image;
        GdkPixbuf *pixbuf;

        image = gtk_bin_get_child (GTK_BIN (view->priv->viewport));
        pixbuf = gdk_pixbuf_loader_get_pixbuf (view->priv->pixbuf_loader);

        if (view->priv->scale != 1.0) {
                buoh_view_set_image_from_pixbuf (view, image, pixbuf);
        } else {
                gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
                /* FIXME; unref pixbuf??? */
                gtk_widget_show (image);
        }
}

static gboolean
buoh_view_load_comic (BuohView *view)
{
        GnomeVFSHandle   *read_handle;
        GnomeVFSResult    result;
        GnomeVFSFileSize  bytes_read;
        guint             buffer[2048];
        GdkCursor        *cursor;
        gchar            *uri;

        uri = buoh_comic_get_uri (view->priv->comic);

        /* Open the comic from URI */
        result = gnome_vfs_open (&read_handle, uri, GNOME_VFS_OPEN_READ);

        if (result != GNOME_VFS_OK) {
                g_print ("Error %d - %s\n", result,
                         gnome_vfs_result_to_string (result));
                
                g_free (uri);

                return FALSE;
        } else {
                g_object_set (G_OBJECT (view),
                              "status", VIEW_STATE_LOADING,
                              NULL);
                
                /* Watch cursor */
                cursor = gdk_cursor_new (GDK_WATCH);
                if (!GTK_WIDGET_REALIZED (view->priv->viewport))
                        gtk_widget_realize (GTK_WIDGET (view->priv->viewport));
                gdk_window_set_cursor (view->priv->viewport->window, cursor);

                if (GDK_IS_PIXBUF_LOADER (view->priv->pixbuf_loader)) {
                        gdk_pixbuf_loader_close (view->priv->pixbuf_loader, NULL);
                        g_object_unref (view->priv->pixbuf_loader);
                        view->priv->pixbuf_loader = NULL;
                }
                view->priv->pixbuf_loader = gdk_pixbuf_loader_new ();
                
                /* Connect callback to signal of pixbuf update */
                g_signal_connect (G_OBJECT (view->priv->pixbuf_loader), "area-updated",
                                  G_CALLBACK (buoh_view_loading_comic),
                                  (gpointer) view);
                /* TODO */
                /*g_signal_connect (G_OBJECT (view->priv->pixbuf_loader), "size-prepared",
                  G_CALLBACK (buoh_view_loading_comic_resize),
                  (gpointer) view);*/

                /* Reading the file */
                result = gnome_vfs_read (read_handle, buffer,
                                         2048, &bytes_read);
                
                while ((result != GNOME_VFS_ERROR_EOF) &&
                       (result == GNOME_VFS_OK)) {

                        gdk_pixbuf_loader_write (view->priv->pixbuf_loader,
                                                 (guchar *) buffer,
                                                 bytes_read, NULL);

                        while (gtk_events_pending ())
                                gtk_main_iteration ();

                        result = gnome_vfs_read (read_handle, buffer,
                                                 2048, &bytes_read);
                }

                if (result != GNOME_VFS_ERROR_EOF)
                        g_print ("Error %d - %s\n", result,
                                 gnome_vfs_result_to_string (result));

                gdk_pixbuf_loader_close (view->priv->pixbuf_loader, NULL);
                gnome_vfs_close (read_handle);

                buoh_comic_set_pixbuf (view->priv->comic,
                                       gdk_pixbuf_loader_get_pixbuf (view->priv->pixbuf_loader));
                
                g_object_unref (view->priv->pixbuf_loader);
                view->priv->pixbuf_loader = NULL;
                
                gdk_window_set_cursor (view->priv->viewport->window, NULL);
                gdk_cursor_unref (cursor);

                g_object_set (G_OBJECT (view),
                              "status", VIEW_STATE_LOADED,
                              NULL);
        }

        g_free (uri);
        
        return TRUE;
}

static gboolean
buoh_view_update_image (gpointer gdata)
{
        BuohView  *view = BUOH_VIEW (gdata);
        GtkWidget *widget;
        GdkPixbuf *pixbuf;

        widget = gtk_bin_get_child (GTK_BIN (view->priv->viewport));
        if (GTK_IS_IMAGE (widget)) {
                if (view->priv->comic) {
                        pixbuf = buoh_comic_get_pixbuf (view->priv->comic);

                        if (!pixbuf) {
                                if (!buoh_view_load_comic (view)) {
                                        if (view->priv->message) {
                                                g_free (view->priv->message);
                                        }
                                        
                                        view->priv->message = g_strdup ("Error loading comic");
                                        gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_MESSAGE);
                                        buoh_view_update_message (view);
                                        g_object_set (G_OBJECT (view),
                                                      "status", VIEW_STATE_ERROR,
                                                      NULL);
                                }
                        } else {
                                buoh_view_set_image_from_pixbuf (view, widget, pixbuf);

                                g_object_set (G_OBJECT (view),
                                              "status", VIEW_STATE_LOADED,
                                              NULL);
                        }
                }
        }

        return FALSE;
}

static void
buoh_view_update_message (BuohView *view)
{
        GtkWidget *widget;

        if (view->priv->message) {
                widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (view), VIEW_PAGE_MESSAGE);
                if (GTK_IS_LABEL (widget)) {
                        gtk_label_set_text (GTK_LABEL (widget),
                                            view->priv->message);
                        gtk_widget_show (widget);
                }
        }
}

static void
buoh_view_update (BuohView *view)
{
        g_return_if_fail (BUOH_IS_VIEW (view));

        if (gtk_notebook_get_current_page (GTK_NOTEBOOK (view)) == VIEW_PAGE_IMAGE) {
                g_idle_add (buoh_view_update_image, (gpointer) view);
        } else if (gtk_notebook_get_current_page (GTK_NOTEBOOK (view)) == VIEW_PAGE_MESSAGE) {
                buoh_view_update_message (view);
        }
}

static void
buoh_view_zoom (BuohView *view, gdouble factor, gboolean relative)
{
        gdouble scale;

        g_return_if_fail (BUOH_IS_VIEW (view));
        g_return_if_fail (BUOH_IS_COMIC (view->priv->comic));

        if (relative)
                scale = view->priv->scale * factor;
        else
                scale = factor;

        g_object_set (G_OBJECT (view),
                      "scale", CLAMP (scale, MIN_ZOOM_SCALE, MAX_ZOOM_SCALE),
                      NULL);

        buoh_view_update (view);
}

gboolean
buoh_view_is_min_zoom (BuohView *view)
{
        return view->priv->scale == MIN_ZOOM_SCALE;
}

gboolean
buoh_view_is_max_zoom (BuohView *view)
{
        return view->priv->scale == MAX_ZOOM_SCALE;
}

gboolean
buoh_view_is_normal_size (BuohView *view)
{
        return view->priv->scale == 1.0;
}

void
buoh_view_zoom_in (BuohView *view)
{
        buoh_view_zoom (view, ZOOM_IN_FACTOR, TRUE);
}

void
buoh_view_zoom_out (BuohView *view)
{
        buoh_view_zoom (view, ZOOM_OUT_FACTOR, TRUE);
}

void
buoh_view_normal_size (BuohView *view)
{
        buoh_view_zoom (view, 1.0, FALSE);
}

void
buoh_view_set_comic (BuohView *view, BuohComic *comic)
{
        g_return_if_fail (BUOH_IS_COMIC (comic));

        g_object_set (G_OBJECT (view),
                      "comic", comic,
                      NULL);
}

BuohComic *
buoh_view_get_comic (BuohView *view)
{
        BuohComic *comic = NULL;

        g_object_get (G_OBJECT (view),
                      "comic", &comic,
                      NULL);

        return comic;
}

BuohViewStatus
buoh_view_get_status (BuohView *view)
{
        return view->priv->status;
}

void
buoh_view_clear (BuohView *view)
{
        gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_EMPTY);
        
        g_object_set (G_OBJECT (view),
                      "status", VIEW_STATE_EMPTY,
                      NULL);
}
