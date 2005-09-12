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
 *  Author: Esteban Sánchez (steve-o) <esteban@steve-o.org>
 */

#include <gnome.h>
#include <glib.h>

#include "buoh.h"
#include "buoh-comic-manager-date.h"

#define BUOH_COMIC_MANAGER_DATE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), BUOH_COMIC_MANAGER_DATE_TYPE, BuohComicManagerDatePrivate))

#define URI_BUFFER 256
#define ID_BUFFER 100

static void buoh_comic_manager_date_init         (BuohComicManagerDate *);
static void buoh_comic_manager_date_class_init   (BuohComicManagerDateClass *);
static void buoh_comic_manager_date_finalize     (GObject *);

static BuohComic *buoh_comic_manager_date_get_next     (BuohComicManager *manager);
static BuohComic *buoh_comic_manager_date_get_previous (BuohComicManager *manager);
static BuohComic *buoh_comic_manager_date_get_first    (BuohComicManager *manager);
static BuohComic *buoh_comic_manager_date_get_last     (BuohComicManager *manager);
static gboolean   buoh_comic_manager_date_is_the_first (BuohComicManager *manager);

static BuohComicManagerClass *parent_class = NULL;

struct _BuohComicManagerDatePrivate {
	GDate     *date;
	gboolean   restrictions[8]; /* Days of week */
};

GType
buoh_comic_manager_date_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (BuohComicManagerDateClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) buoh_comic_manager_date_class_init,
			NULL,
			NULL,
			sizeof (BuohComicManagerDate),
			0,
			(GInstanceInitFunc) buoh_comic_manager_date_init
		};

		type = g_type_register_static (BUOH_COMIC_MANAGER_TYPE,
					       "ComicManagerDate",
					       &info, 0);
	}

	return type;
}

static void
buoh_comic_manager_date_init (BuohComicManagerDate *comic_manager)
{
	gint i;
	
	comic_manager->priv = BUOH_COMIC_MANAGER_DATE_GET_PRIVATE (comic_manager);
	
	comic_manager->priv->date = NULL;

 	for (i = 0; i < 7; i++)
		comic_manager->priv->restrictions[i] = FALSE;
}

static void
buoh_comic_manager_date_class_init (BuohComicManagerDateClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	BuohComicManagerClass *manager_class = (BuohComicManagerClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);

	manager_class->get_next     = buoh_comic_manager_date_get_next;
	manager_class->get_previous = buoh_comic_manager_date_get_previous;
	manager_class->get_last     = buoh_comic_manager_date_get_last;
	manager_class->get_first    = buoh_comic_manager_date_get_first;
	manager_class->is_the_first = buoh_comic_manager_date_is_the_first;
	
	g_type_class_add_private (klass, sizeof (BuohComicManagerDatePrivate));
	
	object_class->finalize = buoh_comic_manager_date_finalize;
}

static void
buoh_comic_manager_date_finalize (GObject *object)
{
	BuohComicManagerDate *comic_manager;
	
	g_return_if_fail (BUOH_IS_COMIC_MANAGER_DATE (object));
	
	comic_manager = BUOH_COMIC_MANAGER_DATE (object);
	
	if (comic_manager->priv->date != NULL) {
		g_date_free (comic_manager->priv->date);
	}
	
	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

BuohComicManagerDate *
buoh_comic_manager_date_new (const gchar *id,
			     const gchar *title,
			     const gchar *author,
			     const gchar *language,
			     const gchar *generic_uri)
{
	BuohComicManagerDate *comic_manager;
	
	comic_manager = BUOH_COMIC_MANAGER_DATE (g_object_new (BUOH_COMIC_MANAGER_DATE_TYPE,
							       NULL));
	g_object_set (G_OBJECT (BUOH_COMIC_MANAGER (comic_manager)),
		      "id", id,
		      "title", title,
		      "author", author,
		      "language", language,
		      "generic_uri", generic_uri,
		      NULL);
	
	return BUOH_COMIC_MANAGER_DATE (comic_manager);
}

void
buoh_comic_manager_date_set_restriction (BuohComicManagerDate *comic_manager,
					 GDateWeekday day)
{
	g_return_if_fail (BUOH_IS_COMIC_MANAGER_DATE (comic_manager));

	comic_manager->priv->restrictions[day] = TRUE;
}

static BuohComic *
buoh_comic_manager_date_new_comic (BuohComicManagerDate *comic_manager)
{
	BuohComic *comic;
	gchar      id[ID_BUFFER];
	gchar      uri[URI_BUFFER];
	gchar     *uri_aux;

	g_object_get (G_OBJECT (comic_manager),
		      "generic_uri", &uri_aux, NULL);
	
	g_date_strftime (id, ID_BUFFER,
			 "%x", /* Date in locale preferred format */
			 comic_manager->priv->date);
	g_date_strftime (uri, URI_BUFFER,
			 uri_aux,
			 comic_manager->priv->date);

	g_debug ("uri: %s\n", uri);
	
	comic = buoh_comic_new_with_info (id, uri, comic_manager->priv->date);
	
	return comic;	
}

static BuohComic *
buoh_comic_manager_date_get_next (BuohComicManager *comic_manager)
{
	GDateWeekday                 weekday;
	BuohComic                   *comic;
	BuohComicManagerDatePrivate *priv;
	GList                       *comic_list;
	
	priv = BUOH_COMIC_MANAGER_DATE_GET_PRIVATE (comic_manager);

	g_date_add_days (priv->date, 1);
	
	/* Check the restrictions */
	weekday = g_date_get_weekday (priv->date);
	while (priv->restrictions[weekday] == TRUE) {
		g_date_add_days (priv->date, 1);
		weekday = g_date_get_weekday (priv->date);
	}
	
	g_object_get (G_OBJECT (comic_manager), 
		      "comic_list", &comic_list,
		      NULL);
	comic = buoh_comic_manager_date_new_comic (BUOH_COMIC_MANAGER_DATE (comic_manager));
	comic_list = g_list_append (comic_list,
				    comic);

	g_object_set (G_OBJECT (comic_manager), "current",
		      comic_list, NULL);
	
	return comic;
}

static BuohComic *
buoh_comic_manager_date_get_previous (BuohComicManager *comic_manager)
{
	GDateWeekday                 weekday;
	BuohComic                   *comic;
	BuohComicManagerDatePrivate *priv;
	GList                       *comic_list;
	
	priv = BUOH_COMIC_MANAGER_DATE_GET_PRIVATE (comic_manager);

	g_date_subtract_days (priv->date, 1);
	
	/* Check the restrictions */
	weekday = g_date_get_weekday (priv->date);
	while (priv->restrictions[weekday] == TRUE) {
		g_date_subtract_days (priv->date, 1);
		weekday = g_date_get_weekday (priv->date);
	}
	
	g_object_get (G_OBJECT (comic_manager), 
		      "comic_list", &comic_list,
		      NULL);
	comic = buoh_comic_manager_date_new_comic (BUOH_COMIC_MANAGER_DATE (comic_manager));
	comic_list = g_list_prepend (comic_list,
				     comic);
	
	g_object_set (G_OBJECT (comic_manager), "current",
		      comic_list, NULL);
	
	return comic;
}

static BuohComic *
buoh_comic_manager_date_get_last (BuohComicManager *comic_manager)
{
	GDate                       *date;
	struct tm                   *gmt;
	time_t                       now;
	GDateWeekday                 weekday;
	BuohComic                   *comic;
	BuohComicManagerDatePrivate *priv;
	GList                       *comic_list;

	priv = BUOH_COMIC_MANAGER_DATE_GET_PRIVATE (comic_manager);

	now = time (NULL);
	gmt = gmtime (&now);
	
	date = g_date_new ();
	g_date_set_time (date, mktime (gmt));

	/* Check the restrictions */
	weekday = g_date_get_weekday (date);
	while (priv->restrictions[weekday] == TRUE) {
		g_date_subtract_days (date, 1);
		weekday = g_date_get_weekday (date);
	}
	
	if (priv->date != NULL)
		g_date_free (priv->date);
	priv->date = date;
	
	g_object_get (G_OBJECT (comic_manager), 
		      "comic_list", &comic_list,
		      NULL);
	comic = buoh_comic_manager_date_new_comic (BUOH_COMIC_MANAGER_DATE (comic_manager));
	
	comic_list = g_list_append (comic_list,
				    comic);
	g_object_set (G_OBJECT (comic_manager), "current",
		      comic_list, NULL);
	
	return comic;
}

static BuohComic *
buoh_comic_manager_date_get_first (BuohComicManager *comic_manager)
{
	/* TODO: Need work from a scholarship holder :) */
	return NULL;
}

static gboolean 
buoh_comic_manager_date_is_the_first (BuohComicManager *comic_manager)
{
	/* TODO: Need work from a scholarship holder :) */
	return FALSE;
}
