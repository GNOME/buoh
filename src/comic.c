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
 *x
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Authors: Pablo Arroyo Loma (zioma) <zioma@linups.org>
 *          Esteban Sanchez Munoz (steve-o) <steve-o@linups.org>
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <glade/glade.h>
#include <glib.h>
#include <glib/gi18n-lib.h>

#include "comic.h"

#define PARENT_TYPE G_TYPE_OBJECT

#define COMIC_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), TYPE_COMIC, ComicPrivate))

static void comic_init         (Comic *);
static void comic_class_init   (ComicClass *);
static void comic_finalize     (GObject *);
static void comic_get_property (GObject      *object,
				guint         prop_id,
				GValue       *value,
				GParamSpec   *pspec);

static void comic_set_property (GObject      *object,
				guint         prop_id,
				const GValue *value,
				GParamSpec   *pspec);

static GObjectClass *parent_class = NULL;

typedef struct _ComicPrivate ComicPrivate;

enum {
	PROP_0,
	PROP_TITLE,
	PROP_AUTHOR,
	PROP_ID,
	PROP_PIXBUF
};

struct _ComicPrivate
{
	gchar     *author;
	gchar     *title;
	gchar     *id;
	GdkPixbuf *pixbuf;
};

GType
comic_get_type ()
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (ComicClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) comic_class_init,
			NULL,
			NULL,
			sizeof (Comic),
			0,
			(GInstanceInitFunc) comic_init
		};

		type = g_type_register_static (PARENT_TYPE, "Comic",
					       &info, 0);
	}

	return type;
}

static void
comic_init (Comic *comic)
{
	ComicPrivate *private;
	   
	g_return_if_fail (IS_COMIC (comic));

	private = COMIC_GET_PRIVATE (comic);
	private->author = NULL;
	private->title  = NULL;
	private->id     = NULL;
	private->pixbuf = NULL;
}

static void
comic_class_init (ComicClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (ComicPrivate));

	object_class->set_property = comic_set_property;
	object_class->get_property = comic_get_property;
	   
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
	   
	object_class->finalize = comic_finalize;
}

static void
comic_finalize (GObject *object)
{
	ComicPrivate *private;
	   
	g_return_if_fail (IS_COMIC (object));

	private = COMIC_GET_PRIVATE (object);

	if (private->author != NULL) {
		g_free (private->author);
		private->author = NULL;
	}

	if (private->title != NULL) {
		g_free (private->title);
		private->title = NULL;
	}

	if (private->pixbuf) {
		g_object_unref (private->pixbuf);
		private->pixbuf = NULL;
	}
					   
	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

Comic *
comic_new ()
{
	Comic *comic;

	comic = g_object_new (TYPE_COMIC, NULL);

	return COMIC (comic);
}

Comic *
comic_new_with_info (const gchar *id, const gchar *title, const gchar *author)
{
	Comic *comic;

	comic = comic_new ();

	g_object_set (G_OBJECT (comic), "id", id,
		      "title", title,
		      "author", author, NULL);

	return COMIC (comic);
}

gchar *
comic_get_uri (Comic *comic)
{
	if (COMIC_GET_CLASS (comic)->get_uri)
		return (COMIC_GET_CLASS (comic)->get_uri)(comic);
	else
		return NULL;
}

gchar *
comic_get_page (Comic *comic)
{
	if (COMIC_GET_CLASS (comic)->get_uri)
		return (COMIC_GET_CLASS (comic)->get_page)(comic);
	else
		return NULL;
}
void
comic_go_next (Comic *comic)
{
	if (COMIC_GET_CLASS (comic)->go_next)
		(COMIC_GET_CLASS (comic)->go_next)(comic);
	else
		return;
}

void
comic_go_previous (Comic *comic)
{
	if (COMIC_GET_CLASS (comic)->go_previous)
		(COMIC_GET_CLASS (comic)->go_previous)(comic);
	else
		return;
}

gboolean
comic_is_the_last (Comic *comic)
{
	if (COMIC_GET_CLASS (comic)->is_the_last)
		return (COMIC_GET_CLASS (comic)->is_the_last)(comic);
	else
		return FALSE;
}

void
comic_set_title (Comic *comic, const gchar *title)
{
	g_return_if_fail (IS_COMIC (comic));

	g_object_set (G_OBJECT (comic), "title", title, NULL);
}

void
comic_set_author (Comic *comic, const gchar *author)
{
	g_return_if_fail (IS_COMIC (comic));

	g_object_set (G_OBJECT (comic), "author", author, NULL);
}

void
comic_set_id (Comic *comic, const gchar *id)
{
	g_return_if_fail (IS_COMIC (comic));

	g_object_set (G_OBJECT (comic), "id", id, NULL);
}

void
comic_set_pixbuf (Comic *comic, GdkPixbuf *pixbuf)
{
	g_return_if_fail (IS_COMIC (comic));

	g_object_set (G_OBJECT (comic), "pixbuf", pixbuf, NULL);
}

void
comic_set_pixbuf_from_file (Comic *comic, const gchar *filename)
{
	g_return_if_fail (IS_COMIC (comic));

	comic_set_pixbuf (comic, gdk_pixbuf_new_from_file (filename, NULL));
}

gchar *
comic_get_title (Comic *comic)
{
	gchar *title;
	   
	g_return_val_if_fail (IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "title", &title, NULL);

	return title;
}

gchar *
comic_get_author (Comic *comic)
{
	gchar *author;
	   
	g_return_val_if_fail (IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "author", &author, NULL);

	return author;
}

gchar *
comic_get_id (Comic *comic)
{
	gchar *id;
	   
	g_return_val_if_fail (IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "id", &id, NULL);

	return id;
}

GdkPixbuf *
comic_get_pixbuf (Comic *comic)
{
	GdkPixbuf *pixbuf;

	g_return_val_if_fail (IS_COMIC (comic), NULL);

	g_object_get (G_OBJECT (comic), "pixbuf", &pixbuf, NULL);

	return pixbuf;
}

/* Private methods */
static void
comic_set_property (GObject      *object,
		    guint         prop_id,
		    const GValue *value,
		    GParamSpec   *pspec)
{
	Comic *comic = COMIC (object);
	ComicPrivate *private;
	   
	g_return_if_fail (IS_COMIC (comic));

	private = COMIC_GET_PRIVATE (comic);
	   
	switch (prop_id) {
	case PROP_TITLE:
		g_free (private->title);
		private->title = g_value_dup_string (value);
		break;
	case PROP_AUTHOR:
		g_free (private->author);
		private->author = g_value_dup_string (value);
		break;
	case PROP_ID:
		g_free (private->author);
		private->id = g_value_dup_string (value);
		break;		
	case PROP_PIXBUF:
		if (private->pixbuf)
			g_object_unref (private->pixbuf);
		private->pixbuf = g_value_get_object (value);
		g_object_ref (g_value_get_object (value));
		break;
	default:
		break;
	}
}


static void
comic_get_property (GObject      *object,
		    guint         prop_id,
		    GValue       *value,
		    GParamSpec   *pspec)
{
	Comic *comic = COMIC (object);
	ComicPrivate *private;
	   
	g_return_if_fail (IS_COMIC (comic));

	private = COMIC_GET_PRIVATE (comic);

	switch (prop_id) {
	case PROP_TITLE:
		g_value_set_string (value, private->title);
		break;
	case PROP_AUTHOR:
		g_value_set_string (value, private->author);
		break;
	case PROP_ID:
		g_value_set_string (value, private->id);
		break;
	case PROP_PIXBUF:
		g_value_set_object (value, private->pixbuf);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}
