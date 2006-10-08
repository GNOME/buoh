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

#include <glib.h>
#include <glib/gi18n.h>
#include <time.h>

#include "buoh.h"
#include "buoh-comic-manager-date.h"

#define BUOH_COMIC_MANAGER_DATE_GET_PRIVATE(object) \
	(G_TYPE_INSTANCE_GET_PRIVATE((object), BUOH_TYPE_COMIC_MANAGER_DATE, BuohComicManagerDatePrivate))

#define URI_BUFFER 256
#define ID_BUFFER 100

static void       buoh_comic_manager_date_init              (BuohComicManagerDate *comic_manager);
static void       buoh_comic_manager_date_class_init        (BuohComicManagerDateClass *klass);
static void       buoh_comic_manager_date_finalize          (GObject *object);

static gchar     *buoh_comic_manager_date_get_id_from_date  (BuohComicManagerDate *comic_manager);
static gchar     *buoh_comic_manager_date_get_uri_from_date (BuohComicManagerDate *comic_manager);
static BuohComic *buoh_comic_manager_date_new_comic         (BuohComicManagerDate *comic_manager);

static BuohComic *buoh_comic_manager_date_get_next          (BuohComicManager     *manager);
static BuohComic *buoh_comic_manager_date_get_previous      (BuohComicManager     *manager);
static BuohComic *buoh_comic_manager_date_get_first         (BuohComicManager     *manager);
static BuohComic *buoh_comic_manager_date_get_last          (BuohComicManager     *manager);
static gboolean   buoh_comic_manager_date_is_the_first      (BuohComicManager     *manager);
static gchar     *buoh_comic_manager_date_get_dayweek       (GDateWeekday          d);

struct _BuohComicManagerDatePrivate {
	GDate     *date;
	GDate     *first;
	gboolean   publications[8]; /* Days of week */
	guint      offset;
};

static const gchar *day_names[] = {
 	NULL,
 	N_("Monday"),
 	N_("Tuesday"),
 	N_("Wednesday"),
 	N_("Thursday"),
 	N_("Friday"),
 	N_("Saturday"),
 	N_("Sunday")
};

G_DEFINE_TYPE (BuohComicManagerDate, buoh_comic_manager_date, BUOH_TYPE_COMIC_MANAGER)

static void
buoh_comic_manager_date_init (BuohComicManagerDate *comic_manager)
{
	gint i;
	
	comic_manager->priv = BUOH_COMIC_MANAGER_DATE_GET_PRIVATE (comic_manager);
	
 	for (i = G_DATE_BAD_WEEKDAY; i <= G_DATE_SUNDAY; i++)
		comic_manager->priv->publications[i] = TRUE;
}

static void
buoh_comic_manager_date_class_init (BuohComicManagerDateClass *klass)
{
	GObjectClass          *object_class = G_OBJECT_CLASS (klass);
	BuohComicManagerClass *manager_class = (BuohComicManagerClass *) klass;
	
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
	
	comic_manager = BUOH_COMIC_MANAGER_DATE (object);
	
	if (comic_manager->priv->date != NULL) {
		g_date_free (comic_manager->priv->date);
		comic_manager->priv->date = NULL;
	}
	
	if (comic_manager->priv->first != NULL) {
		g_date_free (comic_manager->priv->first);
		comic_manager->priv->first = NULL;
	}

	if (G_OBJECT_CLASS (buoh_comic_manager_date_parent_class)->finalize)
		(* G_OBJECT_CLASS (buoh_comic_manager_date_parent_class)->finalize) (object);
}

BuohComicManager *
buoh_comic_manager_date_new (const gchar *id,
			     const gchar *title,
			     const gchar *author,
			     const gchar *language,
			     const gchar *generic_uri)
{
	BuohComicManagerDate *comic_manager;

	g_return_val_if_fail (id != NULL && title != NULL &&
			      author != NULL && language != NULL &&
			      generic_uri != NULL, NULL);
	
	comic_manager = BUOH_COMIC_MANAGER_DATE (g_object_new (BUOH_TYPE_COMIC_MANAGER_DATE,
							       "id", id,
							       "title", title,
							       "author", author,
							       "language", language,
							       "generic_uri", generic_uri,
							       NULL));

	return BUOH_COMIC_MANAGER (comic_manager);
}

void
buoh_comic_manager_date_set_offset (BuohComicManagerDate *comic_manager,
				    guint                 offset)
{
	g_return_if_fail (BUOH_IS_COMIC_MANAGER_DATE (comic_manager));
	g_return_if_fail (offset > 0);

	comic_manager->priv->offset = offset;
}

void
buoh_comic_manager_date_set_restriction (BuohComicManagerDate *comic_manager,
					 GDateWeekday          day)
{
	g_return_if_fail (BUOH_IS_COMIC_MANAGER_DATE (comic_manager));

	comic_manager->priv->publications[day] = FALSE;
}

void
buoh_comic_manager_date_set_first (BuohComicManagerDate *comic_manager,
				   const gchar          *first)
{
	GDate     *date;

	g_return_if_fail (BUOH_IS_COMIC_MANAGER_DATE (comic_manager));
	g_return_if_fail (first != NULL);

	date = g_date_new ();
	g_date_set_parse (date, first);
	if (!g_date_valid (date)) {
		g_warning ("Invalid date: %s", first);
		return;
	}

	if (comic_manager->priv->first)
		g_date_free (comic_manager->priv->first);
	comic_manager->priv->first = date;
}

static gchar *
buoh_comic_manager_date_get_id_from_date (BuohComicManagerDate *comic_manager)
{
	gchar id[ID_BUFFER];

	if (g_date_strftime (id, ID_BUFFER,
			     "%x", /* Date in locale preferred format */
			     comic_manager->priv->date) == 0) {
		buoh_debug ("Id buffer too short");

		return NULL;
	} else {
		buoh_debug ("id: %s", id);
		return g_strdup (id);
	}
}

static gchar *
buoh_comic_manager_date_get_uri_from_date (BuohComicManagerDate *comic_manager)
{
	gchar  uri[URI_BUFFER];
	gchar *uri_aux = NULL;

	g_object_get (G_OBJECT (comic_manager),
		      "generic_uri", &uri_aux, NULL);
	g_assert (uri_aux != NULL);

	if (g_date_strftime (uri, URI_BUFFER,
			     uri_aux,
			     comic_manager->priv->date) == 0) {
		buoh_debug ("Uri buffer too short");
		g_free (uri_aux);

		return NULL;
	} else {
		g_free (uri_aux);
		
		buoh_debug ("uri: %s", uri);
		return g_strdup (uri);
	}
}

static BuohComic *
buoh_comic_manager_date_new_comic (BuohComicManagerDate *comic_manager)
{
	BuohComic *comic;
	gchar     *id = NULL;
	gchar     *uri = NULL;

	id = buoh_comic_manager_date_get_id_from_date (comic_manager);
	uri = buoh_comic_manager_date_get_uri_from_date (comic_manager);
	
	comic = buoh_comic_new_with_info (id, uri, comic_manager->priv->date);
	
	g_free (id);
	g_free (uri);
	
	return comic;	
}

static BuohComic *
buoh_comic_manager_date_get_next (BuohComicManager *comic_manager)
{
	GDateWeekday          weekday;
	BuohComic            *comic = NULL;
	BuohComicManagerDate *cmd;
	GList                *comic_list, *found, *current;
	
	cmd = BUOH_COMIC_MANAGER_DATE (comic_manager);

	g_date_add_days (cmd->priv->date, 1);
	
	/* Check the restrictions */
	weekday = g_date_get_weekday (cmd->priv->date);
	while (!cmd->priv->publications[weekday]) {
		g_date_add_days (cmd->priv->date, 1);
		weekday = g_date_get_weekday (cmd->priv->date);
	}
	
	g_object_get (G_OBJECT (comic_manager), 
		      "list", &comic_list,
		      NULL);
	
	comic = buoh_comic_manager_date_new_comic (BUOH_COMIC_MANAGER_DATE (comic_manager));
	found = g_list_find_custom (comic_list,
				   (gconstpointer) comic,
				   (GCompareFunc) buoh_comic_manager_compare);
	
	if (found) {
		g_object_unref (comic);
		g_object_set (G_OBJECT (comic_manager), "current",
			      found, NULL);
		return BUOH_COMIC (found->data);
	} else {
		g_object_get (G_OBJECT (comic_manager), 
			      "current", &current,
			      NULL);
		
		comic_list = g_list_insert_before (comic_list, 
						   g_list_next (current), comic);
		
		g_object_set (G_OBJECT (comic_manager), "current",
			      g_list_next (current), NULL);
		
		return comic;
	}
}

static BuohComic *
buoh_comic_manager_date_get_previous (BuohComicManager *comic_manager)
{
	GDateWeekday          weekday;
	BuohComic            *comic;
	BuohComicManagerDate *cmd;
	GList                *comic_list, *found;
	
	cmd = BUOH_COMIC_MANAGER_DATE (comic_manager);

	g_date_subtract_days (cmd->priv->date, 1);
	
	/* Check the restrictions */
	weekday = g_date_get_weekday (cmd->priv->date);
	while (!cmd->priv->publications[weekday]) {
		g_date_subtract_days (cmd->priv->date, 1);
		weekday = g_date_get_weekday (cmd->priv->date);
	}
	
	g_object_get (G_OBJECT (comic_manager), 
		      "list", &comic_list,
		      NULL);

	comic = buoh_comic_manager_date_new_comic (BUOH_COMIC_MANAGER_DATE (comic_manager));
	found = g_list_find_custom (comic_list,
				   (gconstpointer) comic,
				   (GCompareFunc) buoh_comic_manager_compare);

	if (found) {
		g_object_set (G_OBJECT (comic_manager), "current",
			      found, NULL);
		return BUOH_COMIC (found->data);
	} else {
		comic_list = g_list_prepend (comic_list,
					     comic);
		
		g_object_set (G_OBJECT (comic_manager), "current",
			      comic_list, NULL);
		g_object_set (G_OBJECT (comic_manager), "list",
			      comic_list, NULL);
		
		return comic;
	}
}

static BuohComic *
buoh_comic_manager_date_get_last (BuohComicManager *comic_manager)
{
	GDate                       *date;
	GDateWeekday                 weekday;
	BuohComic                   *comic;
	BuohComicManagerDatePrivate *priv;
	GList                       *comic_list, *found;

	priv = BUOH_COMIC_MANAGER_DATE_GET_PRIVATE (comic_manager);

	date = g_date_new ();
	
#if GLIB_CHECK_VERSION(2,10,0)
	g_date_set_time_t (date, time (NULL));
#else
	{
		struct tm *gmt;
		time_t     now;
		
		now = time (NULL);
		gmt = gmtime (&now);
		g_date_set_time (date, mktime (gmt));
	}
#endif /* GLIB_CHECK_VERSION(2,10,0) */
	
	if (priv->offset != 0) {
		g_date_subtract_days (date, priv->offset);
	}
	
	/* Check the restrictions */
	weekday = g_date_get_weekday (date);
	while (!priv->publications[weekday]) {
		g_date_subtract_days (date, 1);
		weekday = g_date_get_weekday (date);
	}
	
	if (priv->date != NULL)
		g_date_free (priv->date);
	priv->date = date;
	
	g_object_get (G_OBJECT (comic_manager), 
		      "list", &comic_list,
		      NULL);
	
	comic = buoh_comic_manager_date_new_comic (BUOH_COMIC_MANAGER_DATE (comic_manager));
	found = g_list_find_custom (comic_list,
				    (gconstpointer) comic,
				    (GCompareFunc) buoh_comic_manager_compare);

	if (found) {
		g_object_set (G_OBJECT (comic_manager), "current",
			      found, NULL);
		return BUOH_COMIC (found->data);
	} else {
		comic_list = g_list_append (comic_list,
					    comic);
		g_object_set (G_OBJECT (comic_manager), "current",
			      g_list_last (comic_list), NULL);
		g_object_set (G_OBJECT (comic_manager), "list",
			      comic_list, NULL);
		
		return comic;
	}
}

static gboolean 
buoh_comic_manager_date_is_the_first_comic (BuohComicManager *comic_manager,
					    BuohComic        *comic)
{
	GDate                       *date;
	BuohComicManagerDatePrivate *priv;

	g_assert (BUOH_IS_COMIC (comic));

	priv = BUOH_COMIC_MANAGER_DATE_GET_PRIVATE (comic_manager);
	
	date = buoh_comic_get_date (comic);

	return (g_date_compare (priv->first, date) == 0);
}

static gboolean 
buoh_comic_manager_date_is_the_first (BuohComicManager *comic_manager)
{
	GList *current;
	
	g_object_get (G_OBJECT (comic_manager), 
		      "current", &current,
		      NULL);
	
	return buoh_comic_manager_date_is_the_first_comic (comic_manager,
							   BUOH_COMIC (current->data));
}

static BuohComic *
buoh_comic_manager_date_get_first (BuohComicManager *comic_manager)
{
	GList                       *comic_list, *first;
	BuohComicManagerDatePrivate *priv;
	BuohComic                   *comic;

	priv = BUOH_COMIC_MANAGER_DATE_GET_PRIVATE (comic_manager);

	g_object_get (G_OBJECT (comic_manager), 
		      "list", &comic_list,
		      NULL);
	
	if (priv->date != NULL)
		g_date_free (priv->date);
	priv->date = g_date_new_dmy (g_date_get_day (priv->first),
				     g_date_get_month (priv->first),
				     g_date_get_year (priv->first));
	
	first = g_list_first (comic_list);
	comic = BUOH_COMIC (first->data);
	
	if (!buoh_comic_manager_date_is_the_first_comic (comic_manager,
							 comic)) {
		comic = buoh_comic_manager_date_new_comic (BUOH_COMIC_MANAGER_DATE (comic_manager));
		comic_list = g_list_prepend (comic_list, comic);
		
		g_object_set (G_OBJECT (comic_manager), "list",
			      comic_list, NULL);
	}
			
	g_object_set (G_OBJECT (comic_manager), "current",
		      comic_list, NULL);
	
	return comic;
}

static gchar *
buoh_comic_manager_date_get_dayweek (GDateWeekday d)
{
	if (d >= G_DATE_BAD_WEEKDAY && d <= G_DATE_SUNDAY) {
		return g_strdup (day_names[d]);
	} else {
		return NULL;
	}
}

gchar *
buoh_comic_manager_date_get_publication_days (BuohComicManagerDate *comic_manager)
{
	gint      i, last_printed = 0;
	gboolean  has_restrict = FALSE, prev = FALSE;
	gchar    *date;
	GString  *aux;

	g_return_val_if_fail (BUOH_IS_COMIC_MANAGER_DATE (comic_manager), NULL);
	
	aux = g_string_new ("");

 	for (i = G_DATE_MONDAY; i <= G_DATE_SUNDAY; i++) {
		if (comic_manager->priv->publications[i]) {
			if (!prev) {
				if (aux->len) {
					/* Add a separator */
					g_string_append (aux, ", ");
				}
				
				date = buoh_comic_manager_date_get_dayweek (i);
				g_string_append (aux, date);
				g_free (date);
				
				last_printed = i;
			}
			
			prev = TRUE;
		} else {
			if (prev && (last_printed != i - 1)) {
				if (aux->len) {
					/* It's a range of days */
					g_string_append (aux, _(" to "));
				}
				
				date = buoh_comic_manager_date_get_dayweek (i - 1);
				g_string_append (aux, date);
				g_free (date);
			}
			
			has_restrict = TRUE;
			prev = FALSE;
		}
	}
	
	if (!has_restrict) {
		g_string_set_size (aux, 0);
		g_string_append (aux, _("Every day"));
	}
	
	/* g_string_free () returns the "gchar *" data */
	return g_string_free (aux, FALSE);
}
