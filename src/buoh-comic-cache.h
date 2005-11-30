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
 *  Authors : Carlos García Campos <carlosgc@gnome.org>
 */

#ifndef BUOH_COMIC_CACHE_H
#define BUOH_COMIC_CACHE_H

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "buoh-comic.h"

G_BEGIN_DECLS

typedef struct _BuohComicCache        BuohComicCache;
typedef struct _BuohComicCacheClass   BuohComicCacheClass;
typedef struct _BuohComicCachePrivate BuohComicCachePrivate;

#define BUOH_TYPE_COMIC_CACHE                  (buoh_comic_cache_get_type())
#define BUOH_COMIC_CACHE(object)               (G_TYPE_CHECK_INSTANCE_CAST((object), BUOH_TYPE_COMIC_CACHE, BuohComicCache))
#define BUOH_COMIC_CACHE_CLASS(klass)          (G_TYPE_CHACK_CLASS_CAST((klass), BUOH_TYPE_COMIC_LIST, BuohComicCacheClass))
#define BUOH_IS_COMIC_CACHE(object)            (G_TYPE_CHECK_INSTANCE_TYPE((object), BUOH_TYPE_COMIC_CACHE))
#define BUOH_IS_COMIC_CACHE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), BUOH_TYPE_COMIC_CACHE))
#define BUOH_COMIC_CACHE_GET_CLASS(object)     (G_TYPE_INSTANCE_GET_CLASS((object), BUOH_TYPE_COMIC_CACHE, BuohComicCacheClass))

struct _BuohComicCache {
	GObject                parent;
	BuohComicCachePrivate *priv;
};

struct _BuohComicCacheClass {
	GObjectClass         parent_class;
};

GType           buoh_comic_cache_get_type   (void) G_GNUC_CONST;
BuohComicCache *buoh_comic_cache_new        (void);

void            buoh_comic_cache_set_image  (BuohComicCache *cache,
					     const gchar    *uri,
					     BuohComicImage *image);
BuohComicImage *buoh_comic_cache_get_image  (BuohComicCache *cache,
					     const gchar    *uri);
void            buoh_comic_cache_set_pixbuf (BuohComicCache *cache,
					     const gchar    *uri,
					     GdkPixbuf      *pixbuf);
GdkPixbuf      *buoh_comic_cache_get_pixbuf (BuohComicCache *cache,
					     const gchar    *uri);

G_END_DECLS

#endif /* !BUOH_COMIC_CACHE_H */
