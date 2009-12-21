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

#ifndef BUOH_COMIC_LIST_H
#define BUOH_COMIC_LIST_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "buoh-view.h"
#include "buoh-comic-manager.h"

G_BEGIN_DECLS

typedef struct _BuohComicList        BuohComicList;
typedef struct _BuohComicListClass   BuohComicListClass;
typedef struct _BuohComicListPrivate BuohComicListPrivate;

#define BUOH_TYPE_COMIC_LIST                  (buoh_comic_list_get_type())
#define BUOH_COMIC_LIST(object)               (G_TYPE_CHECK_INSTANCE_CAST((object), BUOH_TYPE_COMIC_LIST, BuohComicList))
#define BUOH_COMIC_LIST_CLASS(klass)          (G_TYPE_CHACK_CLASS_CAST((klass), BUOH_TYPE_COMIC_LIST, BuohComicListClass))
#define BUOH_IS_COMIC_LIST(object)            (G_TYPE_CHECK_INSTANCE_TYPE((object), BUOH_TYPE_COMIC_LIST))
#define BUOH_IS_COMIC_LIST_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), BUOH_TYPE_COMIC_LIST))
#define BUOH_COMIC_LIST_GET_CLASS(object)     (G_TYPE_INSTANCE_GET_CLASS((object), BUOH_TYPE_COMIC_LIST, BuohComicListClass))

struct _BuohComicList {
	GtkBin                parent;
	BuohComicListPrivate *priv;
};

struct _BuohComicListClass {
	GtkBinClass         parent_class;
};

GType             buoh_comic_list_get_type        (void) G_GNUC_CONST;
GtkWidget        *buoh_comic_list_new             (void);

void              buoh_comic_list_set_view        (BuohComicList *comic_list,
						   BuohView      *view);
GtkWidget        *buoh_comic_list_get_list        (BuohComicList *comic_list);
void              buoh_comic_list_clear_selection (BuohComicList *comic_list);
BuohComicManager *buoh_comic_list_get_selected    (BuohComicList *comic_list);

G_END_DECLS

#endif /* !BUOH_COMIC_LIST_H */
