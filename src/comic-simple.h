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
 */
 
/* Author: Pablo Arroyo Loma (zioma) <zioma@linups.org>
 *         Esteban Sánchez (steve-o) <steve-o@linups.org>
 */      

#include <glib-object.h>

#ifndef TYPE_COMIC
#include "comic.h"
#endif

/* Definicion de los CAST entre objetos y clases */
#define TYPE_COMIC_SIMPLE	    (comic_simple_get_type ())
#define COMIC_SIMPLE(o)		    (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_COMIC_SIMPLE, ComicSimple))
#define COMIC_SIMPLE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TYPE_ComicSimple, ComicSimpleClass))
#define IS_COMIC_SIMPLE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_COMIC_SIMPLE))
#define IS_COMIC_SIMPLE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_COMIC_SIMPLE))
#define COMIC_SIMPLE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_COMIC_SIMPLE, ComicSimpleClass))

typedef struct _ComicSimple      ComicSimple;
typedef struct _ComicSimpleClass ComicSimpleClass;

/* ComicSimple Object */
struct _ComicSimple {
	Comic parent;
};

/* ComicSimple Class */
struct _ComicSimpleClass {
	ComicClass parent_class;

	/* Point to the real comic_get_uri function */
//	   gchar * (* get_uri) ();
};

/* Public methods */
GType  comic_simple_get_type      (void);

ComicSimple  *comic_simple_new ();


ComicSimple  *comic_simple_new_with_info (const gchar *id,
					  const gchar *title,
					  const gchar *author,
					  const gchar *main_uri);

void   comic_simple_set_title (Comic *comic, const gchar *title);

void   comic_simple_set_author (Comic *comic, const gchar *author);

void   comic_simple_set_generic_uri (Comic *comic, const gchar *generic_uri);
void   comic_simple_set_restriction (ComicSimple *comic, GDateWeekday day);
void   comic_simple_go_next (Comic *comic);
void   comic_simple_go_previous (Comic *comic);

gboolean comic_simple_is_the_last (Comic *comic);
