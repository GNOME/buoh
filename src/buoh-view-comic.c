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
 *  Authors : Carlos Garc�a Campos <carlosgc@gnome.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "buoh-view-comic.h"
#include "buoh-comic-loader.h"

enum {
	PROP_0,
	PROP_COMIC,
	PROP_SCALE
};

struct _BuohViewComicPrivate {
	BuohView        *view;
	GtkWidget       *image;
	
	BuohComic       *comic;
	gdouble          scale;

	guint            load_monitor;
	BuohComicLoader *comic_loader;
};

#define BUOH_VIEW_COMIC_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_VIEW_COMIC, BuohViewComicPrivate))

#define ZOOM_IN_FACTOR  1.5
#define ZOOM_OUT_FACTOR (1.0 / ZOOM_IN_FACTOR)

#define MIN_ZOOM_SCALE (ZOOM_OUT_FACTOR * ZOOM_OUT_FACTOR)
#define MAX_ZOOM_SCALE (ZOOM_IN_FACTOR * ZOOM_IN_FACTOR)

static GtkViewportClass *parent_class = NULL;

static void buoh_view_comic_init             (BuohViewComic *m_view);
static void buoh_view_comic_class_init       (BuohViewComicClass *klass);
static void buoh_view_comic_finalize         (GObject       *object);
static void buoh_view_comic_set_property     (GObject       *object,
					      guint          prop_id,
					      const GValue  *value,
					      GParamSpec    *pspec);
static void buoh_view_comic_get_property     (GObject       *object,
					      guint          prop_id,
					      GValue        *value,
					      GParamSpec    *pspec);
static void bouh_view_comic_changed_comic_cb (GObject       *object,
					      GParamSpec    *arg,
					      gpointer       gdata);

static void     buoh_view_comic_set_image_from_pixbuf (BuohViewComic *c_view,
						       GdkPixbuf     *pixbuf);
static gboolean buoh_view_comic_load_monitor          (gpointer       gdata);
static void     buoh_view_comic_load                  (BuohViewComic *c_view);
static void     buoh_view_comic_zoom                  (BuohViewComic *c_view,
						       gdouble        factor,
						       gboolean       relative);

GType
buoh_view_comic_get_type ()
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (BuohViewComicClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) buoh_view_comic_class_init,
			NULL,
			NULL,
			sizeof (BuohViewComic),
			0,
			(GInstanceInitFunc) buoh_view_comic_init
		};

		type = g_type_register_static (GTK_TYPE_VIEWPORT, "BuohViewComic",
					       &info, 0);
	}

	return type;
}

static void
buoh_view_comic_init (BuohViewComic *c_view)
{
	c_view->priv = BUOH_VIEW_COMIC_GET_PRIVATE (c_view);

	c_view->priv->view = NULL;
	c_view->priv->scale = 1.0;
	c_view->priv->comic = NULL;
	c_view->priv->load_monitor = 0;
	c_view->priv->comic_loader = buoh_comic_loader_new ();
	
	c_view->priv->image = gtk_image_new ();
	gtk_container_add (GTK_CONTAINER (c_view),
			   c_view->priv->image);
	gtk_widget_show (c_view->priv->image);

	/* Callbacks */
	g_signal_connect (G_OBJECT (c_view),
			  "notify::comic",
			  G_CALLBACK (bouh_view_comic_changed_comic_cb),
			  NULL);

	gtk_widget_show (GTK_WIDGET (c_view));
}

static void
buoh_view_comic_class_init (BuohViewComicClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = buoh_view_comic_set_property;
	object_class->get_property = buoh_view_comic_get_property;
	
	parent_class = g_type_class_peek_parent (klass);

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
	
	g_type_class_add_private (klass, sizeof (BuohViewComicPrivate));

	object_class->finalize = buoh_view_comic_finalize;
}

static void
buoh_view_comic_finalize (GObject *object)
{
	BuohViewComic *c_view = BUOH_VIEW_COMIC (object);

	g_debug ("buoh-view-comic finalize\n");

	if (c_view->priv->load_monitor > 0) {
		g_source_remove (c_view->priv->load_monitor);
		c_view->priv->load_monitor = 0;
	}

	if (c_view->priv->comic) {
		g_object_unref (c_view->priv->comic);
		c_view->priv->comic = NULL;
	}

	if (c_view->priv->comic_loader) {
		g_object_unref (c_view->priv->comic_loader);
		c_view->priv->comic_loader = NULL;
	}
	
	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
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
		if (c_view->priv->comic) {
			g_object_unref (c_view->priv->comic);
		}

		c_view->priv->comic = BUOH_COMIC (g_value_dup_object (value));

		break;
	case PROP_SCALE:
		c_view->priv->scale = g_value_get_double (value);

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
		g_value_set_object (value, c_view->priv->comic);

		break;
	case PROP_SCALE:
		g_value_set_double (value, c_view->priv->scale);

		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

GtkWidget *
buoh_view_comic_new (BuohView *view)
{
	GtkWidget *c_view;

	c_view = GTK_WIDGET (g_object_new (BUOH_TYPE_VIEW_COMIC,
					   "shadow-type", GTK_SHADOW_IN,
					   NULL));
	BUOH_VIEW_COMIC (c_view)->priv->view = view;
	
	return c_view;
}

static void
bouh_view_comic_changed_comic_cb (GObject *object, GParamSpec *arg, gpointer gdata)
{
	BuohViewComic *c_view = BUOH_VIEW_COMIC (object);

	gtk_image_clear (GTK_IMAGE (c_view->priv->image));
	
	buoh_view_comic_load (c_view);
}

static void
buoh_view_comic_set_image_from_pixbuf (BuohViewComic *c_view, GdkPixbuf *pixbuf)
{
	GdkPixbuf *new_pixbuf;

	g_return_if_fail (GDK_IS_PIXBUF (pixbuf));

	if (c_view->priv->scale != 1.0) {
		new_pixbuf = gdk_pixbuf_scale_simple (pixbuf,
						      gdk_pixbuf_get_width (pixbuf) * c_view->priv->scale,
						      gdk_pixbuf_get_height (pixbuf) * c_view->priv->scale,
						      GDK_INTERP_BILINEAR);

		gtk_image_set_from_pixbuf (GTK_IMAGE (c_view->priv->image), new_pixbuf);

		g_object_unref (new_pixbuf);
	} else {
		gtk_image_set_from_pixbuf (GTK_IMAGE (c_view->priv->image), pixbuf);
	}

	gtk_widget_show (c_view->priv->image);
}

static gboolean
buoh_view_comic_load_monitor (gpointer gdata)
{
	BuohViewComic *c_view = BUOH_VIEW_COMIC (gdata);
	static GdkCursor *cursor = NULL;

	switch (c_view->priv->comic_loader->status) {
	case LOADER_STATE_READY:
		return TRUE;
	case LOADER_STATE_RUNNING:
		/* Watch cursor */
		if (!cursor) {
			cursor = gdk_cursor_new (GDK_WATCH);
			if (!GTK_WIDGET_REALIZED (c_view))
				gtk_widget_realize (GTK_WIDGET (c_view));
			gdk_window_set_cursor (GTK_WIDGET (c_view)->window, cursor);
		}

		if (GDK_IS_PIXBUF (c_view->priv->comic_loader->pixbuf)) {
			g_mutex_lock (c_view->priv->comic_loader->pixbuf_mutex);
			buoh_view_comic_set_image_from_pixbuf (
				c_view,
				g_object_ref (c_view->priv->comic_loader->pixbuf));
			g_mutex_unlock (c_view->priv->comic_loader->pixbuf_mutex);
		}

		return TRUE;
	case LOADER_STATE_FINISHED:
		if (GDK_IS_PIXBUF (c_view->priv->comic_loader->pixbuf)) {
			g_mutex_lock (c_view->priv->comic_loader->pixbuf_mutex);
			buoh_view_comic_set_image_from_pixbuf (c_view,
							       c_view->priv->comic_loader->pixbuf);
			buoh_comic_set_pixbuf (c_view->priv->comic,
					       c_view->priv->comic_loader->pixbuf);
			g_mutex_unlock (c_view->priv->comic_loader->pixbuf_mutex);

			g_object_set (G_OBJECT (c_view->priv->view),
				      "status", STATE_COMIC_LOADED,
				      NULL);
		}
	case LOADER_STATE_STOPPING:
		if (cursor) {
			gdk_window_set_cursor (GTK_WIDGET (c_view)->window, NULL);
			gdk_cursor_unref (cursor);
			cursor = NULL;
		}

		g_debug ("Monitor exit (stopping/finished)");

		return FALSE;
	case LOADER_STATE_FAILED:
		if (cursor) {
			gdk_window_set_cursor (GTK_WIDGET (c_view)->window, NULL);
			gdk_cursor_unref (cursor);
			cursor = NULL;
		}
		
		buoh_view_set_message_title (BUOH_VIEW (c_view->priv->view),
					     _("Error Loading Comic"));
		buoh_view_set_message_text (BUOH_VIEW (c_view->priv->view),
					    _("There has been an error when loading the comic. "
					      "It use to be due to an error on the remote server. "
					      "Please, try again later."));
		buoh_view_set_message_icon (BUOH_VIEW (c_view->priv->view),
					    GTK_STOCK_DIALOG_ERROR);

		g_object_set (G_OBJECT (c_view->priv->view),
			      "status", STATE_MESSAGE_ERROR,
			      NULL);

		g_debug ("Monitor exit (failed)");

		return FALSE;
	default:
		g_debug ("Monitor exit (unknown)");

		return FALSE;
	}
}

static void
buoh_view_comic_load (BuohViewComic *c_view)
{
	GdkPixbuf *pixbuf;
	gchar     *uri;

	if (c_view->priv->comic_loader->status == LOADER_STATE_RUNNING) {
		g_debug ("Load already running");
		buoh_comic_loader_stop (c_view->priv->comic_loader);
		g_debug ("waiting thread");
		g_thread_join (c_view->priv->comic_loader->thread);
		g_debug ("died");
	}

	/* Finish the load monitor */
	while (gtk_events_pending ())
		gtk_main_iteration ();

	pixbuf = buoh_comic_get_pixbuf (c_view->priv->comic);
	if (pixbuf) {
		buoh_view_comic_set_image_from_pixbuf (c_view, pixbuf);
		g_object_set (G_OBJECT (c_view->priv->view),
			      "status", STATE_COMIC_LOADED,
			      NULL);
	} else {
		g_object_set (G_OBJECT (c_view->priv->view),
			      "status", STATE_COMIC_LOADING,
			      NULL);
		uri = buoh_comic_get_uri (c_view->priv->comic);
		buoh_comic_loader_run (c_view->priv->comic_loader, uri);
		g_free (uri);
		c_view->priv->load_monitor =
			g_timeout_add (60,
				       (GSourceFunc)buoh_view_comic_load_monitor,
				       (gpointer)c_view);
	}
}

static void
buoh_view_comic_zoom (BuohViewComic *c_view, gdouble factor, gboolean relative)
{
	gdouble    scale;
	GdkPixbuf *pixbuf = NULL;

	g_return_if_fail (BUOH_IS_VIEW_COMIC (c_view));
	g_return_if_fail (BUOH_IS_COMIC (c_view->priv->comic));

	if (relative)
		scale = c_view->priv->scale * factor;
	else
		scale = factor;

	g_object_set (G_OBJECT (c_view),
		      "scale", CLAMP (scale, MIN_ZOOM_SCALE, MAX_ZOOM_SCALE),
		      NULL);
	
	pixbuf = buoh_comic_get_pixbuf (c_view->priv->comic);
	if (pixbuf)
		buoh_view_comic_set_image_from_pixbuf (c_view, pixbuf);
}

gboolean
buoh_view_comic_is_min_zoom (BuohViewComic *c_view)
{
	return c_view->priv->scale == MIN_ZOOM_SCALE;
}

gboolean
buoh_view_comic_is_max_zoom (BuohViewComic *c_view)
{
	return c_view->priv->scale == MAX_ZOOM_SCALE;
}

gboolean
buoh_view_comic_is_normal_size (BuohViewComic *c_view)
{
	return c_view->priv->scale == 1.0;
}

void
buoh_view_comic_zoom_in (BuohViewComic *c_view)
{
	buoh_view_comic_zoom (c_view, ZOOM_IN_FACTOR, TRUE);
}

void
buoh_view_comic_zoom_out (BuohViewComic *c_view)
{
	buoh_view_comic_zoom (c_view, ZOOM_OUT_FACTOR, TRUE);
}

void
buoh_view_comic_normal_size (BuohViewComic *c_view)
{
	buoh_view_comic_zoom (c_view, 1.0, FALSE);
}