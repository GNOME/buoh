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

#ifndef BUOH_COMIC_LOADER_H
#define BUOH_COMIC_LOADER_H

#include <glib-object.h>

#include "buoh-comic.h"

G_BEGIN_DECLS


#define BUOH_TYPE_COMIC_LOADER                  (buoh_comic_loader_get_type())
G_DECLARE_DERIVABLE_TYPE (BuohComicLoader, buoh_comic_loader, BUOH, COMIC_LOADER, GObject)

#define BUOH_COMIC_LOADER_ERROR (buoh_comic_loader_error_quark())

typedef void (*BuohComicLoaderLoadFunc) (const gchar *data,
                                         guint        len,
                                         gpointer     gdata);

struct _BuohComicLoaderClass {
        GObjectClass   parent_class;

        void (* finished) (BuohComicLoader *loader);
};

GQuark           buoh_comic_loader_error_quark (void);
BuohComicLoader *buoh_comic_loader_new         (void);

void             buoh_comic_loader_load_comic  (BuohComicLoader *loader,
                                                BuohComic       *comic,
                                                BuohComicLoaderLoadFunc callback,
                                                gpointer         gdata);
void             buoh_comic_loader_get_error   (BuohComicLoader *loader,
                                                GError         **error);
void             buoh_comic_loader_cancel      (BuohComicLoader *loader);

G_END_DECLS

#endif /* !BUOH_COMIC_LOADER_H */
