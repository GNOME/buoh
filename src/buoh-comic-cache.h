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
 *  Authors: Carlos García Campos <carlosgc@gnome.org>
 */

#ifndef BUOH_COMIC_CACHE_H
#define BUOH_COMIC_CACHE_H

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "buoh-comic.h"

G_BEGIN_DECLS

#define BUOH_TYPE_COMIC_CACHE buoh_comic_cache_get_type ()
G_DECLARE_FINAL_TYPE (BuohComicCache, buoh_comic_cache, BUOH, COMIC_CACHE, GObject)

BuohComicCache *buoh_comic_cache_new        (void);

void            buoh_comic_cache_set_image  (BuohComicCache *cache,
                                             const gchar    *uri,
                                             BuohComicImage *image);
BuohComicImage *buoh_comic_cache_get_image  (BuohComicCache *cache,
                                             const gchar    *uri);
GdkPixbuf      *buoh_comic_cache_get_pixbuf (BuohComicCache *cache,
                                             const gchar    *uri);

G_END_DECLS

#endif /* !BUOH_COMIC_CACHE_H */
