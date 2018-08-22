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
#include <gdk-pixbuf/gdk-pixbuf.h>

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


typedef struct {
	guchar *data;
	gsize   size;
} BuohComicImage;

struct _BuohComic {
	GObject           parent;

	BuohComicPrivate *priv;
};

struct _BuohComicClass {
	GObjectClass      parent_class;
};

GType           buoh_comic_get_type             (void) G_GNUC_CONST; 
BuohComic      *buoh_comic_new                  (void);
BuohComic      *buoh_comic_new_with_info        (const gchar    *id,
						 const gchar    *uri,
						 const GDate    *date);

void            buoh_comic_set_id               (BuohComic      *comic,
						 const gchar    *id);
void            buoh_comic_go_next              (BuohComic      *comic);
void            buoh_comic_go_previous          (BuohComic      *comic);
void            buoh_comic_set_pixbuf           (BuohComic      *comic,
						 GdkPixbuf      *pixbuf);
void            buoh_comic_set_image            (BuohComic      *comic,
						 BuohComicImage *image);
void	        buoh_comic_set_date             (BuohComic      *comic,
						 GDate          *date);
void            buoh_comic_set_pixbuf_from_file (BuohComic      *comic,
						 const gchar    *filename);

const gchar    *buoh_comic_get_uri              (BuohComic      *comic);
const gchar    *buoh_comic_get_id               (BuohComic      *comic);
GdkPixbuf      *buoh_comic_get_pixbuf           (BuohComic      *comic);
BuohComicImage *buoh_comic_get_image            (BuohComic      *comic);
GDate          *buoh_comic_get_date             (BuohComic      *comic);
GdkPixbuf      *buoh_comic_get_thumbnail        (BuohComic      *comic);
gchar          *buoh_comic_get_filename         (BuohComic      *comic);

gboolean        buoh_comic_image_save           (BuohComicImage *image,
						 const gchar    *path,
						 GError        **error);
void            buoh_comic_image_free           (BuohComicImage *image);

G_END_DECLS

#endif /* !BUOH_COMIC_H */
