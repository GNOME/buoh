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

#include "comic-simple.h"

#define PARENT_TYPE TYPE_COMIC

#define COMIC_SIMPLE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), TYPE_COMIC_SIMPLE, ComicSimplePrivate))

static void comic_simple_init         (ComicSimple *);
static void comic_simple_class_init   (ComicSimpleClass *);
static void comic_simple_finalize     (GObject *);

static void comic_simple_set_property (GObject      *object,
				       guint         prop_id,
				       const GValue *value,
				       GParamSpec   *pspec);

static void comic_simple_get_property (GObject      *object,
				       guint         prop_id,
				       GValue       *value,
				       GParamSpec   *pspec);
gchar * comic_simple_get_uri (Comic *comic);

gchar * comic_simple_get_last_uri (Comic *comic);

gchar * comic_simple_get_uri_from_date (ComicSimple *comic);

static GObjectClass *parent_class = NULL;

typedef struct _ComicSimplePrivate ComicSimplePrivate;

enum {
	PROP_0,
	PROP_GENERIC_URI
};

enum {
	PROP_,
	PROP_TITLE,
	PROP_AUTHOR
};


struct _ComicSimplePrivate {
	gchar *generic_uri;
	GDate *date;
};

GType
comic_simple_get_type ()
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (ComicSimpleClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) comic_simple_class_init,
			NULL,
			NULL,
			sizeof (ComicSimple),
			0,
			(GInstanceInitFunc) comic_simple_init
		};

		type = g_type_register_static (PARENT_TYPE, "ComicSimple",
					       &info, 0);
	}

	return type;
}

static void
comic_simple_init (ComicSimple *comic)
{
	ComicSimplePrivate *private;
	   
	g_return_if_fail (IS_COMIC_SIMPLE (comic));

	private = COMIC_SIMPLE_GET_PRIVATE (comic);

	private->generic_uri = NULL;
	private->date        = NULL;
}

static void
comic_simple_class_init (ComicSimpleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	ComicClass *comic_class = (ComicClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	comic_class->get_uri = comic_simple_get_uri;
	comic_class->go_previous = comic_simple_go_previous;
	comic_class->go_next = comic_simple_go_next;
	comic_class->is_the_last = comic_simple_is_the_last;

	g_type_class_add_private (klass, sizeof (ComicSimplePrivate));

	object_class->set_property = comic_simple_set_property;
	object_class->get_property = comic_simple_get_property;
	   
	g_object_class_install_property (object_class,
					 PROP_GENERIC_URI,
					 g_param_spec_string ("generic_uri",
							      "Generic URI",
							      "URI with date format",
							      NULL,
							      G_PARAM_READWRITE));
	   
	object_class->finalize = comic_simple_finalize;
}

static void
comic_simple_finalize (GObject *object)
{
	ComicSimplePrivate *private;
	   
	g_return_if_fail (IS_COMIC_SIMPLE (object));

	private = COMIC_SIMPLE_GET_PRIVATE (object);

	if (private->generic_uri != NULL) {
		g_free (private->generic_uri);
	}

	if (private->date != NULL) {
		g_date_free (private->date);
	}
	   
	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}


ComicSimple *
comic_simple_new ()
{
	ComicSimple *comic;

	comic = g_object_new (TYPE_COMIC_SIMPLE, NULL);

	return COMIC_SIMPLE (comic);
	   
}


ComicSimple *
comic_simple_new_with_info (const gchar *id,
			    const gchar *title,
			    const gchar *author,
			    const gchar *generic_uri)
{
	ComicSimple        *comic;

	comic = comic_simple_new ();

	g_object_set (G_OBJECT (comic), "id", id,
		      "title", title, "author", author,
		      "generic_uri", generic_uri, NULL);
	   
	return COMIC_SIMPLE (comic);
}

void
comic_simple_set_generic_uri (Comic *comic,
			      const gchar *generic_uri)
{
	g_return_if_fail (IS_COMIC_SIMPLE (comic));

	g_object_set (G_OBJECT (comic),
		      "generic_uri", generic_uri, NULL);
}

static void
comic_simple_set_property (GObject      *object,
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
	Comic *comic = COMIC (object);
	ComicSimplePrivate *private;
	   
	g_return_if_fail (IS_COMIC_SIMPLE (comic));

	private = COMIC_SIMPLE_GET_PRIVATE (comic);
	   
	switch (prop_id)
	{
	case PROP_GENERIC_URI:
		g_free (private->generic_uri);
		private->generic_uri = g_value_dup_string (value);
		break;
	default:
		break;
	}
}

static void
comic_simple_get_property (GObject      *object,
			   guint         prop_id,
			   GValue       *value,
			   GParamSpec   *pspec)
{
	ComicSimple *comic = COMIC_SIMPLE (object);
	ComicSimplePrivate *private;
	   
	g_return_if_fail (IS_COMIC_SIMPLE (comic));

	private = COMIC_SIMPLE_GET_PRIVATE (comic);


	switch (prop_id)
	{
	case PROP_GENERIC_URI:
		g_value_set_string (value, private->generic_uri);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

gchar *
comic_simple_get_uri_from_date (ComicSimple *comic)
{
	gchar               uri[100];
	ComicSimplePrivate *private;

	g_return_val_if_fail (IS_COMIC_SIMPLE (comic), NULL);

	private = COMIC_SIMPLE_GET_PRIVATE (comic);

	g_date_strftime (uri, 100,
			 private->generic_uri,
			 private->date);
	   
	g_debug ("uri: %s\n", uri);

	return g_strdup (uri);
}

gboolean
comic_simple_is_the_last (Comic *comic)
{
	GDate              *date;
	struct tm          *gmt;
	time_t              now;
	ComicSimplePrivate *private;
	   
	private = COMIC_SIMPLE_GET_PRIVATE (comic);
	   
	now = time (NULL);
	gmt = gmtime (&now);
	   
	date = g_date_new ();
	g_date_set_time (date, mktime (gmt));

	return (gboolean) g_date_compare (private->date, date);
}

gchar *
comic_simple_get_last_uri (Comic *comic)
{
	GDate              *date;
	struct tm          *gmt;
	time_t              now;
	ComicSimplePrivate *private;
	   
	private = COMIC_SIMPLE_GET_PRIVATE (comic);
	   
	if (private->date != NULL)
		g_date_free (private->date);

	now = time (NULL);
	gmt = gmtime (&now);
	   
	date = g_date_new ();
	g_date_set_time (date, mktime (gmt));
	private->date = date;
	   
	return comic_simple_get_uri_from_date (COMIC_SIMPLE (comic));
}

gchar *
comic_simple_get_uri (Comic *comic)
{
	ComicSimplePrivate *private;
	   
	private = COMIC_SIMPLE_GET_PRIVATE (comic);

	/* If the object has a date */
	if (private->date == NULL)
		return comic_simple_get_last_uri (comic);
	else
		return comic_simple_get_uri_from_date (COMIC_SIMPLE (comic));
	   
}

void
comic_simple_go_next (Comic *comic)
{
	ComicSimplePrivate *private;
	   
	private = COMIC_SIMPLE_GET_PRIVATE (comic);

	/* TODO: Check if a date is valid for a comic */
	g_date_add_days (private->date, 1);

	return;
}

void
comic_simple_go_previous (Comic *comic)
{
	ComicSimplePrivate *private;
	   
	private = COMIC_SIMPLE_GET_PRIVATE (comic);

	/* TODO: Check if a date is valid for a comic */
	g_date_subtract_days (private->date, 1);

	return;
}
