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
 *           Esteban Sánchez (steve-o) <esteban@steve-o.org>
 */      

#ifndef BUOH_COMIC_H
#define BUOH_COMIC_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _BuohComic        BuohComic;
typedef struct _BuohComicClass   BuohComicClass;
typedef struct _BuohComicPrivate BuohComicPrivate;

#define BUOH_TYPE_COMIC		(buoh_comic_get_type ())
#define BUOH_COMIC(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), BUOH_TYPE_COMIC, BuohComic))
#define BUOH_COMIC_CLASS(k) 	(G_TYPE_CHECK_CLASS_CAST((k), BUOH_TYPE_COMIC, BuohComicClass))
#define BUOH_IS_COMIC(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), BUOH_TYPE_COMIC))
#define BUOH_IS_COMIC_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), BUOH_TYPE_COMIC))
#define BUOH_COMIC_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), BUOH_TYPE_COMIC, BuohComicClass))


struct _BuohComic {
	GObject           parent;

	BuohComicPrivate *priv;
};

struct _BuohComicClass {
	GObjectClass      parent_class;

	/* Point to functions that has to be redefined by the child class*/
	gchar    *(* get_uri)     (BuohComic *comic);
	void      (* go_next)     (BuohComic *comic);
	void      (* go_previous) (BuohComic *comic);
	gboolean  (* is_the_last) (BuohComic *comic);
	gchar    *(* get_page)    (BuohComic *comic);
};

/* Public methods */
GType      buoh_comic_get_type      (void); 
BuohComic *buoh_comic_new           (void);
BuohComic *buoh_comic_new_with_info (const gchar *id,
				     const gchar *title,
				     const gchar *author);

/*gboolean comic_is_the_last (Comic *comic);*/

void       buoh_comic_go_next              (BuohComic   *comic);
void       buoh_comic_go_previous          (BuohComic   *comic);

void       buoh_comic_set_title            (BuohComic   *comic,
					    const gchar *title);
void       buoh_comic_set_author           (BuohComic   *comic,
					    const gchar *author);
void       buoh_comic_set_id               (BuohComic   *comic,
					    const gchar *id);
void       buoh_comic_set_pixbuf           (BuohComic   *comic,
					    GdkPixbuf   *pixbuf);
void       buoh_comic_set_pixbuf_from_file (BuohComic   *comic,
					    const gchar *filename);
gchar     *buoh_comic_get_uri              (BuohComic   *comic);
gchar     *buoh_comic_get_title            (BuohComic   *comic);
gchar     *buoh_comic_get_author           (BuohComic   *comic);
gchar     *buoh_comic_get_id               (BuohComic   *comic);
gchar     *buoh_comic_get_page             (BuohComic   *comic);
gchar     *buoh_buoh_comic_get_uri         (BuohComic   *comic);
GdkPixbuf *buoh_comic_get_pixbuf           (BuohComic   *comic);

G_END_DECLS

#endif /* !BUOH_COMIC_H */
