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

/* Definicion de los CAST entre objetos y clases */
#define TYPE_COMIC						(comic_get_type ())
#define COMIC(o)					(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_COMIC, Comic))
#define COMIC_CLASS(k) 				(G_TYPE_CHECK_CLASS_CAST((k), TYPE_COMIC, ComicClass))
#define IS_COMIC(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_COMIC))
#define IS_COMIC_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_COMIC))
#define COMIC_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_COMIC, ComicClass))

typedef struct _Comic      Comic;
typedef struct _ComicClass ComicClass;

/* Comic Object */
struct _Comic {
	GObject parent;
};

/* Comic Class */
struct _ComicClass {
	GObjectClass parent_class;

	/* Point to comic_get_uri function which has to be redefined by the child class*/
	gchar    *(* get_uri) (Comic *comic);
	void      (* go_next) (Comic *comic);
	void      (* go_previous) (Comic *comic);
	gboolean  (* is_the_last) (Comic *comic);
};

/* Public methods */
GType comic_get_type      (void);

Comic *comic_new ();

Comic *comic_new_with_info (const gchar *id, const gchar *title, const gchar *author);

gchar *comic_get_uri (Comic *comic);

gboolean comic_is_the_last (Comic *comic);

void comic_go_next (Comic *comic);
void comic_go_previous (Comic *comic);

void comic_set_title (Comic *comic, const gchar *title);
void comic_set_author (Comic *comic, const gchar *author);
void comic_set_id (Comic *comic, const gchar *id);
void comic_set_pixbuf (Comic *comic, GdkPixbuf *pixbuf);
void comic_set_pixbuf_from_file (Comic *comic, const gchar *filename);

gchar *comic_get_title (Comic *comic);
gchar *comic_get_author (Comic *comic);
gchar *comic_get_id (Comic *comic);
GdkPixbuf *comic_get_pixbuf (Comic *comic);
