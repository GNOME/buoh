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

/*#ifdef HAVE_CONFIG_H
#include <config.h>
#endif*/

#include <glib.h>
#include <glib/gi18n.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "buoh-comic.h"

enum {
	PROP_0,
	PROP_TITLE,
	PROP_AUTHOR,
	PROP_ID,
	        PROP_PIXBUF
};

struct _BuohComicPrivate {
	gchar     *author;
	gchar     *title;
	gchar     *id;
	GdkPixbuf *pixbuf;
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
buoh_comic_get_type ()
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
	
	buoh_comic->priv->author = NULL;
	buoh_comic->priv->title  = NULL;
	buoh_comic->priv->id     = NULL;
	buoh_comic->priv->pixbuf = NULL;
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
					 PROP_TITLE,
					 g_param_spec_string ("title",
							      "Title",
							      "Title of the comic",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_AUTHOR,
					 g_param_spec_string ("author",
							      "Author",
							      "Name of the author",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_ID,
					 g_param_spec_string ("id",
							      "Id",
							      "Identificator of the comic",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_PIXBUF,
					 g_param_spec_object ("pixbuf",
							      "Pixbuf",
							      "Pixbuf of the comic",
							      GDK_TYPE_PIXBUF,
							      G_PARAM_READWRITE));
	   
	object_class->finalize = buoh_comic_finalize;
}

static void
buoh_comic_finalize (GObject *object)
{
	BuohComic *comic = BUOH_COMIC (object);
	
	g_return_if_fail (BUOH_IS_COMIC (comic));

	if (comic->priv->author) {
		g_free (comic->priv->author);
		comic->priv->author = NULL;
	}

	if (comic->priv->title) {
		g_free (comic->priv->title);
		comic->priv->title = NULL;
	}

	if (comic->priv->pixbuf) {
		g_object_unref (comic->priv->pixbuf);
		comic->priv->pixbuf = NULL;
	}
					   
	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
buoh_comic_set_property (GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	BuohComic *comic = BUOH_COMIC (object);

	switch (prop_id) {
	case PROP_TITLE:
		g_free (comic->priv->title);
		comic->priv->title = g_value_dup_string (value);
		
		break;
	case PROP_AUTHOR:
		g_free (comic->priv->author);
		comic->priv->author = g_value_dup_string (value);
		
		break;
	case PROP_ID:
		g_free (comic->priv->id);
		comic->priv->id = g_value_dup_string (value);
		
		break;
	case PROP_PIXBUF:
		if (comic->priv->pixbuf) {
			g_object_unref (comic->priv->pixbuf);
		}
		comic->priv->pixbuf = GDK_PIXBUF (g_value_dup_object (value));
		
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
	case PROP_TITLE:
		g_value_set_string (value, comic->priv->title);
		
		break;
	case PROP_AUTHOR:
		g_value_set_string (value, comic->priv->author);
		
		break;
	case PROP_ID:
		g_value_set_string (value, comic->priv->id);
		
		break;
	case PROP_PIXBUF:
		g_value_set_object (value, comic->priv->pixbuf);
		
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

BuohComic *
buoh_comic_new ()
{
	BuohComic *comic;

	comic = BUOH_COMIC (g_object_new (BUOH_TYPE_COMIC, NULL));

	return comic;
}

BuohComic *
buoh_comic_new_with_info (const gchar *id,
			  const gchar *title,
			  const gchar *author)
{
	BuohComic *comic;

	comic = buoh_comic_new ();

	g_object_set (G_OBJECT (comic),
		      "id", id,
		      "title", title,
		      "author", author,
		      NULL);

	return comic;
}

gchar *
buoh_comic_get_uri (BuohComic *comic)
{
	if (BUOH_COMIC_GET_CLASS (comic)->get_uri)
		return (BUOH_COMIC_GET_CLASS (comic)->get_uri)(comic);
	else
		return NULL;
}

gchar *
buoh_comic_get_page (BuohComic *comic)
{
	if (BUOH_COMIC_GET_CLASS (comic)->get_uri)
		return (BUOH_COMIC_GET_CLASS (comic)->get_page)(comic);
	else
		return NULL;
}

/*void
buoh_comic_go_next (BuohComic *comic)
{
	if (BUOH_COMIC_GET_CLASS (comic)->go_next)
		(BUOH_COMIC_GET_CLASS (comic)->go_next)(comic);
	else
		return;
}

void
buoh_comic_go_previous (BuohComic *comic)
{
	if (BUOH_COMIC_GET_CLASS (comic)->go_previous)
		(BUOH_COMIC_GET_CLASS (comic)->go_previous)(comic);
	else
		return;
}

gboolean
buoh_comic_is_the_last (BuohComic *comic)
{
	if (BUOH_COMIC_GET_CLASS (comic)->is_the_last)
		return (BUOH_COMIC_GET_CLASS (comic)->is_the_last)(comic);
	else
		return FALSE;
}*/

void
buoh_comic_set_title (BuohComic *comic, const gchar *title)
{
	g_return_if_fail (BUOH_IS_COMIC (comic));

	g_object_set (G_OBJECT (comic), "title", title, NULL);
}

void
buoh_comic_set_author (BuohComic *comic, const gchar *author)
{
	g_return_if_fail (BUOH_IS_COMIC (comic));

	g_object_set (G_OBJECT (comic), "author", author, NULL);
}

void
buoh_comic_set_id (BuohComic *comic, const gchar *id)
{
	g_return_if_fail (BUOH_IS_COMIC (comic));

	g_object_set (G_OBJECT (comic), "id", id, NULL);
}

void
buoh_comic_set_pixbuf (BuohComic *comic, GdkPixbuf *pixbuf)
{
	g_return_if_fail (BUOH_IS_COMIC (comic));

	g_object_set (G_OBJECT (comic), "pixbuf", pixbuf, NULL);
}

void
buoh_comic_set_pixbuf_from_file (BuohComic *comic, const gchar *filename)
{
	GdkPixbuf *pixbuf = NULL;
	
	g_return_if_fail (BUOH_IS_COMIC (comic));

	pixbuf = gdk_pixbuf_new_from_file (filename, NULL);

	if (pixbuf) {
		buoh_comic_set_pixbuf (comic, pixbuf);
		g_object_unref (pixbuf);
	}
}

gchar *
buoh_comic_get_title (BuohComic *comic)
{
	gchar *title = NULL;
	   
	g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "title", &title, NULL);

	return title;
}

gchar *
buoh_comic_get_author (BuohComic *comic)
{
	gchar *author = NULL;
	   
	g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "author", &author, NULL);

	return author;
}

gchar *
buoh_comic_get_id (BuohComic *comic)
{
	gchar *id = NULL;
	   
	g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "id", &id, NULL);

	return id;
}

GdkPixbuf *
buoh_comic_get_pixbuf (BuohComic *comic)
{
	GdkPixbuf *pixbuf = NULL;

	g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "pixbuf", &pixbuf, NULL);

	return pixbuf;
}

