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
 *           Esteban Sanchez Munoz (steve-o) <esteban@steve-o.org>
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "buoh.h"
#include "buoh-comic.h"
#include "buoh-comic-cache.h"

enum {
	PROP_0,
	PROP_ID,
	PROP_URI,
	PROP_PIXBUF,
	PROP_IMAGE,
	PROP_DATE
};

struct _BuohComicPrivate {
	gchar          *id;
	gchar          *uri;
	GDate          *date;

	BuohComicCache *cache;
};

#define BUOH_COMIC_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_COMIC, BuohComicPrivate))

static GObjectClass *parent_class = NULL;

static void buoh_comic_init         (BuohComic      *buoh_comic);
static void buoh_comic_class_init   (BuohComicClass *klass);
static void buoh_comic_finalize     (GObject        *object);
static void buoh_comic_get_property (GObject        *object,
				     guint           prop_id,
				     GValue         *value,
				     GParamSpec     *pspec);
static void buoh_comic_set_property (GObject        *object,
				     guint           prop_id,
				     const GValue   *value,
				     GParamSpec     *pspec);

GType
buoh_comic_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (BuohComicClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) buoh_comic_class_init,
			NULL,
			NULL,
			sizeof (BuohComic),
			0,
			(GInstanceInitFunc) buoh_comic_init
		};

		type = g_type_register_static (G_TYPE_OBJECT, "BuohComic",
					       &info, 0);
	}
	
	return type;
}

static void
buoh_comic_init (BuohComic *buoh_comic)
{
	buoh_comic->priv = BUOH_COMIC_GET_PRIVATE (buoh_comic);

	buoh_comic->priv->id = NULL;
	buoh_comic->priv->uri = NULL;
	buoh_comic->priv->date = NULL;

	buoh_comic->priv->cache = buoh_comic_cache_new ();
}

static void
buoh_comic_class_init (BuohComicClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (BuohComicPrivate));

	object_class->set_property = buoh_comic_set_property;
	object_class->get_property = buoh_comic_get_property;
	   
	g_object_class_install_property (object_class,
					 PROP_ID,
					 g_param_spec_string ("id",
							      "Id",
							      "Identificator of the comic",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_URI,
					 g_param_spec_string ("uri",
							      "URI",
							      "URI of the comic",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_PIXBUF,
					 g_param_spec_pointer ("pixbuf",
							       "Pixbuf",
							       "Pixbuf of the comic",
							       G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_IMAGE,
					 g_param_spec_pointer ("image",
							       "Image",
							       "Compressed image of the comic",
							       G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_DATE,
					 g_param_spec_pointer ("date",
							       "Date",
							       "Date release of the comic",
							       G_PARAM_READWRITE));

	object_class->finalize = buoh_comic_finalize;
}

static void
buoh_comic_finalize (GObject *object)
{
	BuohComic *comic = BUOH_COMIC (object);
	
	buoh_debug ("buoh-comic-finalize");

	if (comic->priv->id) {
		g_free (comic->priv->id);
		comic->priv->id = NULL;
	}

	if (comic->priv->uri) {
		g_free (comic->priv->uri);
		comic->priv->uri = NULL;
	}

	if (comic->priv->date) {
		g_date_free (comic->priv->date);
		comic->priv->date = NULL;
	}

	if (comic->priv->cache) {
		g_object_unref (comic->priv->cache);
		comic->priv->cache = NULL;
	}
	
	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

BuohComic *
buoh_comic_new (void)
{
	BuohComic *comic;

	comic = BUOH_COMIC (g_object_new (BUOH_TYPE_COMIC, NULL));

	return comic;
}

BuohComic *
buoh_comic_new_with_info (const gchar *id, const gchar *uri,
			  const GDate *date)
{
	BuohComic *comic;

	g_return_val_if_fail (id != NULL && uri != NULL, NULL);
	g_return_val_if_fail (date != NULL, NULL);

	comic = BUOH_COMIC (g_object_new (BUOH_TYPE_COMIC,
					  "id", id,
		      			  "uri", uri,
					  "date", date, NULL));
	
	return comic;
}


static void
buoh_comic_set_property (GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	BuohComic *comic = BUOH_COMIC (object);
	GDate     *date;	

	switch (prop_id) {
	case PROP_ID:
		g_free (comic->priv->id);
		comic->priv->id = g_value_dup_string (value);
		
		break;
	case PROP_URI:
		g_free (comic->priv->uri);
		comic->priv->uri = g_value_dup_string (value);
		
		break;
	case PROP_PIXBUF: {
		GdkPixbuf *pixbuf;

		pixbuf = GDK_PIXBUF (g_value_get_pointer (value));
		buoh_comic_cache_set_pixbuf (comic->priv->cache,
					     comic->priv->uri, pixbuf);
	}
		break;
	case PROP_IMAGE: {
		BuohComicImage *image;

		image = (BuohComicImage *) g_value_get_pointer (value);
		buoh_comic_cache_set_image (comic->priv->cache,
					    comic->priv->uri, image);
	}
		
		break;
	case PROP_DATE:
		if (comic->priv->date) {
			g_date_free (comic->priv->date);
		}
		date = g_value_get_pointer (value);
		
		comic->priv->date = g_date_new_dmy (g_date_get_day (date),
						    g_date_get_month (date),
						    g_date_get_year (date));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
buoh_comic_get_property (GObject      *object,
			 guint         prop_id,
			 GValue       *value,
			 GParamSpec   *pspec)
{
	BuohComic *comic = BUOH_COMIC (object);

	switch (prop_id) {
	case PROP_ID:
		g_value_set_string (value, comic->priv->id);
		
		break;
	case PROP_URI:
		g_value_set_string (value, comic->priv->uri);
		
		break;
	case PROP_PIXBUF: {
		GdkPixbuf *pixbuf;

		pixbuf = buoh_comic_cache_get_pixbuf (comic->priv->cache,
						      comic->priv->uri);
		g_value_set_pointer (value, pixbuf);
	}
		break;
	case PROP_IMAGE: {
		BuohComicImage *image;

		image = buoh_comic_cache_get_image (comic->priv->cache,
						    comic->priv->uri);
		g_value_set_pointer (value, image);
	}
		break;
	case PROP_DATE:
		g_value_set_pointer (value, comic->priv->date);
		
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

void
buoh_comic_set_id (BuohComic *comic, const gchar *id)
{
	g_return_if_fail (BUOH_IS_COMIC (comic));
	g_return_if_fail (id != NULL);

	g_object_set (G_OBJECT (comic), "id", id, NULL);
}

void
buoh_comic_set_pixbuf (BuohComic *comic, GdkPixbuf *pixbuf)
{
	g_return_if_fail (BUOH_IS_COMIC (comic));
	g_return_if_fail (GDK_IS_PIXBUF (pixbuf));

	g_object_set (G_OBJECT (comic), "pixbuf", pixbuf, NULL);
}

void
buoh_comic_set_image (BuohComic *comic, BuohComicImage *image)
{
	g_return_if_fail (BUOH_IS_COMIC (comic));
	g_return_if_fail (image != NULL);

	g_object_set (G_OBJECT (comic), "image", image, NULL);
}

void
buoh_comic_set_date (BuohComic *comic, GDate *date)
{
	g_return_if_fail (BUOH_IS_COMIC (comic));
	g_return_if_fail (date != NULL);

	g_object_set (G_OBJECT (comic), "date", date, NULL);
}

void
buoh_comic_set_pixbuf_from_file (BuohComic *comic, const gchar *filename)
{
	GdkPixbuf *pixbuf = NULL;
	
	g_return_if_fail (BUOH_IS_COMIC (comic));
	g_return_if_fail (filename != NULL);

	pixbuf = gdk_pixbuf_new_from_file (filename, NULL);

	buoh_comic_set_pixbuf (comic, pixbuf);
}

gchar *
buoh_comic_get_id (BuohComic *comic)
{
	gchar *id = NULL;
	   
	g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "id", &id, NULL);

	return id;
}

gchar *
buoh_comic_get_uri (BuohComic *comic)
{
	gchar *uri = NULL;
	   
	g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "uri", &uri, NULL);

	return uri;
}

GdkPixbuf *
buoh_comic_get_pixbuf (BuohComic *comic)
{
	GdkPixbuf *pixbuf = NULL;

	g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "pixbuf", &pixbuf, NULL);

	return pixbuf;
}

BuohComicImage *
buoh_comic_get_image (BuohComic *comic)
{
	BuohComicImage *image = NULL;

	g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "image", &image, NULL);

	return image;
}

GDate *
buoh_comic_get_date (BuohComic *comic)
{
	GDate *date = NULL;
	   
	g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "date", &date, NULL);

	return date;
}

GdkPixbuf *
buoh_comic_get_thumbnail (BuohComic *comic)
{
	GdkPixbuf *pixbuf = NULL;
	GdkPixbuf *thumbnail = NULL;
	gint       c_width, c_height;
	gint       d_width, d_height;

	g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);
	
	g_object_get (G_OBJECT (comic), "pixbuf", &pixbuf, NULL);

	if (pixbuf) {
		c_width = gdk_pixbuf_get_width (pixbuf);
		c_height = gdk_pixbuf_get_height (pixbuf);

		if (c_width > c_height) {
			d_width = 96;
			d_height = c_height - (((c_width - d_width) * c_height) / c_width);
		} else {
			d_height = 96;
			d_width = c_width - (((c_height - d_height) * c_width) / c_height);
		}

		thumbnail = gdk_pixbuf_scale_simple (pixbuf, d_width, d_height, GDK_INTERP_BILINEAR);

		return thumbnail;
	}

	return NULL;
}

gchar *
buoh_comic_get_filename (BuohComic *comic)
{
	g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);
	
	gchar *filename;
	
	filename = g_path_get_basename (comic->priv->uri);
	
	return filename;
}

void
buoh_comic_image_free (BuohComicImage *image)
{
	if (image) {
		g_free (image->data);
		g_free (image);
	}
}
