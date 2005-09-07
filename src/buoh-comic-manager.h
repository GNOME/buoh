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

typedef struct _BuohComicManager        BuohComicManager;
typedef struct _BuohComicManagerClass   BuohComicManagerClass;
typedef struct _BuohComicManagerPrivate BuohComicManagerPrivate;

#define BUOH_COMIC_MANAGER_TYPE	        (buoh_comic_manager_get_type ())
#define BUOH_COMIC_MANAGER(o)	        (G_TYPE_CHECK_INSTANCE_CAST ((o), BUOH_COMIC_MANAGER_TYPE, BuohComicManager))
#define BUOH_COMIC_MANAGER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), BUOH_COMIC_MANAGER_TYPE, BuohComicManagerClass))
#define BUOH_IS_COMIC_MANAGER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), BUOH_COMIC_MANAGER_TYPE))
#define BUOH_IS_COMIC_MANAGER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), BUOH_COMIC_MANAGER_TYPE))
#define BUOH_COMIC_MANAGER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), BUOH_COMIC_MANAGER_TYPE, BuohComicManagerClass))


struct _BuohComicManager {
	GObject parent;
	
	BuohComicManagerPrivate *priv;
};

struct _BuohComicManagerClass {
	GObjectClass      parent_class;

	/* Point to functions of the abstract class */
	gchar     *(* get_page)       (BuohComicManager *comic_manager);
	gchar     *(* get_uri)        (BuohComicManager *comic_manager);
	BuohComic *(* get_next)       (BuohComicManager *comic_manager);
	BuohComic *(* get_previous)   (BuohComicManager *comic_manager);
	BuohComic *(* get_n_next)     (BuohComicManager *comic_manager, int n);
	BuohComic *(* get_n_previous) (BuohComicManager *comic_manager, int n);
	gboolean   (* is_the_last)    (BuohComicManager *comic_manager);
	gboolean   (* is_the_first)   (BuohComicManager *comic_manager);
};

/* Public methods */
GType             buoh_comic_manager_get_type      (void); 

void     buoh_comic_manager_go_next            (BuohComicManager   *comic);
void     buoh_comic_manager_go_previous        (BuohComicManager   *comic);
BuohComic *buoh_comic_manager_get_n_next       (BuohComicManager *comic_manager,
						int n);
BuohComic *buoh_comic_manager_get_n_previous   (BuohComicManager *comic_manager,
						int n);
BuohComic *buoh_comic_manager_get_next         (BuohComicManager *comic_manager);
BuohComic *buoh_comic_manager_get_previous     (BuohComicManager *comic_manager);

gboolean buoh_comic_manager_is_the_last        (BuohComicManager *comic_manager);
gboolean buoh_comic_manager_is_the_first       (BuohComicManager *comic_manager);

void     buoh_comic_manager_set_title          (BuohComicManager   *comic_manager,
						const gchar *title);
void     buoh_comic_manager_set_author         (BuohComicManager   *comic_manager,
						const gchar *author);
void     buoh_comic_manager_set_language       (BuohComicManager   *comic_manager,
						const gchar *language);
void     buoh_comic_manager_set_id             (BuohComicManager   *comic_manager,
						const gchar *id);
gchar   *buoh_comic_manager_get_uri            (BuohComicManager   *comic_manager);
gchar   *buoh_comic_manager_get_title          (BuohComicManager   *comic_manager);
gchar   *buoh_comic_manager_get_author         (BuohComicManager   *comic_manager);
gchar   *buoh_comic_manager_get_language       (BuohComicManager   *comic_manager);
gchar   *buoh_comic_manager_get_id             (BuohComicManager   *comic_manager);
gchar   *buoh_comic_manager_get_page           (BuohComicManager   *comic_manager);
gchar   *buoh_comic_manager_get_uri            (BuohComicManager   *comic_manager);

G_END_DECLS

#endif /* !BUOH_COMIC_MANAGER_H */
