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
#include "buoh-view-message.h"
#include "buoh-comic-loader.h"

enum {
        PROP_0,
        PROP_COMIC,
        PROP_SCALE
};

struct _BuohViewPrivate {
        GtkWidget       *viewport;

	GtkWidget       *message;
	
        BuohComic       *comic;
        gdouble          scale;
	
	guint            load_monitor;
	BuohComicLoader *comic_loader;
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
static void buoh_view_update_image           (BuohView      *view);
static void buoh_view_update_message         (BuohView      *view);

static void buoh_view_zoom                   (BuohView      *view,
                                              gdouble        factor,
                                              gboolean       relative);

static void buoh_view_clear_image            (BuohView      *view);
static void buoh_view_set_image_from_pixbuf  (BuohView      *view,
                                              GdkPixbuf     *pixbuf);
static void buoh_view_load_comic             (BuohView      *view);

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

static void
buoh_view_init (BuohView *buoh_view)
{
        GtkWidget *image;
        GtkWidget *label;
        GtkWidget *swindow;
        
        buoh_view->priv = BUOH_VIEW_GET_PRIVATE (buoh_view);
        
        buoh_view->priv->scale = 1.0;
        buoh_view->priv->comic = NULL;
	buoh_view->priv->load_monitor = 0;
	buoh_view->priv->comic_loader = buoh_comic_loader_new ();

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
	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_NEVER,
					GTK_POLICY_NEVER);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swindow),
					     GTK_SHADOW_NONE);
	buoh_view->priv->message = buoh_view_message_new ();
	buoh_view_message_set_title (BUOH_VIEW_MESSAGE (buoh_view->priv->message),
				     _("Buoh Comics Reader"));
	buoh_view_message_set_text (BUOH_VIEW_MESSAGE (buoh_view->priv->message),
				    _("Welcome to <b>Buoh</b>, the comics browser for GNOME Desktop.\n"
				      "The list on the left panel contains your favourite comics, "
				      "to add or remove comics to the list click on Comic -> Add. "
				      "Just select a comic from the list, and it will be displayed "
				      "on the right side. Thanks for using Buoh."));
	buoh_view_message_set_icon (BUOH_VIEW_MESSAGE (buoh_view->priv->message),
				    "buoh64x64.png");
	gtk_container_add (GTK_CONTAINER (swindow),
			   buoh_view->priv->message);
	gtk_widget_show (buoh_view->priv->message);
	
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

        object_class->finalize = buoh_view_finalize;
}

static void
buoh_view_finalize (GObject *object)
{
        BuohView *view = BUOH_VIEW (object);
        
        g_return_if_fail (BUOH_IS_VIEW (object));

        g_debug ("buoh-view finalize\n");

	if (view->priv->load_monitor > 0) {
		g_source_remove (view->priv->load_monitor);
		view->priv->load_monitor = 0;
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

	if (view->priv->comic_loader) {
		g_object_unref (view->priv->comic_loader);
		view->priv->comic_loader = NULL;
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
        BuohView  *view = BUOH_VIEW (object);

        gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_IMAGE);
        
        buoh_view_update (view);
}

static void
buoh_view_clear_image (BuohView *view)
{
	GtkWidget *image;

	image = gtk_bin_get_child (GTK_BIN (view->priv->viewport));
	gtk_image_clear (GTK_IMAGE (image));
	gtk_widget_show (image);
}

static void
buoh_view_set_image_from_pixbuf (BuohView *view, GdkPixbuf *pixbuf)
{
        GdkPixbuf *new_pixbuf;
	GtkWidget *image;

	g_return_if_fail (GDK_IS_PIXBUF (pixbuf));
	
	image = gtk_bin_get_child (GTK_BIN (view->priv->viewport));
	
	if (view->priv->scale != 1.0) {
		new_pixbuf = gdk_pixbuf_scale_simple (pixbuf,
						      gdk_pixbuf_get_width (pixbuf) * view->priv->scale,
						      gdk_pixbuf_get_height (pixbuf) * view->priv->scale,
						      GDK_INTERP_BILINEAR);
		
		gtk_image_set_from_pixbuf (GTK_IMAGE (image), new_pixbuf);

		g_object_unref (new_pixbuf);
	} else {
		gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
	}

        gtk_widget_show (image);
}

static gboolean 
buoh_view_comic_load_monitor (gpointer gdata)
{
	BuohView *view = BUOH_VIEW (gdata);
	static GdkCursor *cursor = NULL;
	
	switch (view->priv->comic_loader->status) {
	case LOADER_STATE_READY:
		return TRUE;
	case LOADER_STATE_RUNNING:
		/* Watch cursor */
		if (!cursor) {
			cursor = gdk_cursor_new (GDK_WATCH);
			if (!GTK_WIDGET_REALIZED (view->priv->viewport))
				gtk_widget_realize (GTK_WIDGET (view->priv->viewport));
			gdk_window_set_cursor (view->priv->viewport->window, cursor);
		}
	
		if (GDK_IS_PIXBUF (view->priv->comic_loader->pixbuf)) {
			g_mutex_lock (view->priv->comic_loader->pixbuf_mutex);
			buoh_view_set_image_from_pixbuf (view, g_object_ref (view->priv->comic_loader->pixbuf));
			g_mutex_unlock (view->priv->comic_loader->pixbuf_mutex);
		}

		return TRUE;
	case LOADER_STATE_FINISHED:
		if (GDK_IS_PIXBUF (view->priv->comic_loader->pixbuf)) {
			g_mutex_lock (view->priv->comic_loader->pixbuf_mutex);
			buoh_view_set_image_from_pixbuf (view, view->priv->comic_loader->pixbuf);
			buoh_comic_set_pixbuf (view->priv->comic, view->priv->comic_loader->pixbuf);
			g_mutex_unlock (view->priv->comic_loader->pixbuf_mutex);
		}
	case LOADER_STATE_STOPPING:
		if (cursor) {
			gdk_window_set_cursor (view->priv->viewport->window, NULL);
			gdk_cursor_unref (cursor);
			cursor = NULL;
		}

		g_debug ("Monitor exit");
		
		return FALSE;
	case LOADER_STATE_FAILED:
		buoh_view_message_set_title (BUOH_VIEW_MESSAGE (view->priv->message),
					     _("Error Loading Comic"));
		buoh_view_message_set_text (BUOH_VIEW_MESSAGE (view->priv->message),
					    _("There has been an error when loading the comic. "
					      "It use to be due to an error on the remote server. "
					      "Please, try again later."));
		buoh_view_message_set_icon (BUOH_VIEW_MESSAGE (view->priv->message),
					    GTK_STOCK_DIALOG_ERROR);
		
		gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_MESSAGE);
		buoh_view_update_message (view);

		g_debug ("Monitor exit");
		
		return FALSE;
	default:
		g_debug ("Monitor exit");
		
		return FALSE;
		
	}		
}

static void
buoh_view_load_comic (BuohView *view)
{
	GdkPixbuf *pixbuf;
	gchar     *uri;

	if (view->priv->comic_loader->status == LOADER_STATE_RUNNING) {
		g_debug ("Load already running");
		buoh_comic_loader_stop (view->priv->comic_loader);
		g_debug ("waiting thread");
		g_thread_join (view->priv->comic_loader->thread);
		g_debug ("died");
	}

	/* Finish the load monitor */
	while (gtk_events_pending ())
		gtk_main_iteration ();

	pixbuf = buoh_comic_get_pixbuf (view->priv->comic);
	if (pixbuf) {
		buoh_view_set_image_from_pixbuf (view, pixbuf);
	} else {
		uri = buoh_comic_get_uri (view->priv->comic);
		buoh_comic_loader_run (view->priv->comic_loader, uri);
		g_free (uri);
		view->priv->load_monitor =
			g_timeout_add (60,
				       (GSourceFunc)buoh_view_comic_load_monitor,
				       (gpointer)view);
	}
}

static void
buoh_view_update_image (BuohView *view)
{
        GtkWidget *widget;

        widget = gtk_bin_get_child (GTK_BIN (view->priv->viewport));
        if (GTK_IS_IMAGE (widget)) {
                if (view->priv->comic) {
			buoh_view_clear_image (view);
			buoh_view_load_comic (view);
                }
        }
}

static void
buoh_view_update_message (BuohView *view)
{
        GtkWidget *widget;

        if (view->priv->message) {
                widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (view), VIEW_PAGE_MESSAGE);
                if (BUOH_IS_VIEW_MESSAGE (widget)) {
                        gtk_widget_show (widget);
                }
        }
}

static void
buoh_view_update (BuohView *view)
{
        g_return_if_fail (BUOH_IS_VIEW (view));

        if (gtk_notebook_get_current_page (GTK_NOTEBOOK (view)) == VIEW_PAGE_IMAGE) {
		buoh_view_update_image (view);
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

void
buoh_view_clear (BuohView *view)
{
        gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_EMPTY);
}
