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
 *  Authors: Esteban Sánchez (steve-o) <esteban@steve-o.org>
 */

#ifndef BUOH_COMIC_MANAGER_H
#define BUOH_COMIC_MANAGER_H

#include "buoh-comic.h"
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _BuohComicManagerPrivate BuohComicManagerPrivate;

#define BUOH_TYPE_COMIC_MANAGER                (buoh_comic_manager_get_type ())
G_DECLARE_DERIVABLE_TYPE (BuohComicManager, buoh_comic_manager, BUOH, COMIC_MANAGER, GObject)

struct _BuohComicManager {
        GObject parent;

        BuohComicManagerPrivate *priv;
};

// TODO: not sure what to do about this
struct _BuohComicManagerClass {
        GObjectClass      parent_class;

        /* Point to functions of the abstract class */
        BuohComic *(* get_next)       (BuohComicManager *comic_manager);
        BuohComic *(* get_previous)   (BuohComicManager *comic_manager);
        BuohComic *(* get_last)       (BuohComicManager *comic_manager);
        BuohComic *(* get_first)      (BuohComicManager *comic_manager);
        gboolean   (* is_the_first)   (BuohComicManager *comic_manager);
};

GType           buoh_comic_manager_get_type (void) G_GNUC_CONST;

BuohComicManager *buoh_comic_manager_new    (const gchar      *type,
                                             const gchar      *id,
                                             const gchar      *title,
                                             const gchar      *author,
                                             const gchar      *language,
                                             const gchar      *generic_uri);

BuohComic   *buoh_comic_manager_get_next     (BuohComicManager *comic_manager);
BuohComic   *buoh_comic_manager_get_previous (BuohComicManager *comic_manager);
BuohComic   *buoh_comic_manager_get_current  (BuohComicManager *comic_manager);
BuohComic   *buoh_comic_manager_get_last     (BuohComicManager *comic_manager);
BuohComic   *buoh_comic_manager_get_first    (BuohComicManager *comic_manager);

gboolean     buoh_comic_manager_is_the_last  (BuohComicManager *comic_manager);
gboolean     buoh_comic_manager_is_the_first (BuohComicManager *comic_manager);

const gchar *buoh_comic_manager_get_uri      (BuohComicManager *comic_manager);
const gchar *buoh_comic_manager_get_title    (BuohComicManager *comic_manager);
const gchar *buoh_comic_manager_get_author   (BuohComicManager *comic_manager);
const gchar *buoh_comic_manager_get_language (BuohComicManager *comic_manager);
const gchar *buoh_comic_manager_get_id       (BuohComicManager *comic_manager);
const gchar *buoh_comic_manager_get_page     (BuohComicManager *comic_manager);

gint         buoh_comic_manager_compare      (gconstpointer     a,
                                              gconstpointer     b);

G_END_DECLS

#endif /* !BUOH_COMIC_MANAGER_H */
