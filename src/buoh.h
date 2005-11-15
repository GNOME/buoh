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

#ifndef BUOH_H
#define BUOH_H

#include <glib-object.h>
#include <gtk/gtktreemodel.h>

G_BEGIN_DECLS

enum {
	COMIC_LIST_VISIBLE,
	COMIC_LIST_TITLE,
	COMIC_LIST_AUTHOR,
	COMIC_LIST_LANGUAGE,
	COMIC_LIST_COMIC_MANAGER,
	N_COLUMNS
};

typedef struct _Buoh        Buoh;
typedef struct _BuohClass   BuohClass;
typedef struct _BuohPrivate BuohPrivate;

#define BUOH_TYPE_BUOH                  (buoh_get_type())
#define BUOH_BUOH(object)               (G_TYPE_CHECK_INSTANCE_CAST((object), BUOH_TYPE_BUOH, Buoh))
#define BUOH_CLASS(klass)          	(G_TYPE_CHACK_CLASS_CAST((klass), BUOH_TYPE_BUOH, BuohClass))
#define BUOH_IS_BUOH(object)            (G_TYPE_CHECK_INSTANCE_TYPE((object), BUOH_TYPE_BUOH))
#define BUOH_IS_BUOH_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), BUOH_TYPE_BUOH))
#define BUOH_BUOH_GET_CLASS(object)     (G_TYPE_INSTANCE_GET_CLASS((object), BUOH_TYPE_BUOH, BuohClass))

#define BUOH                            (buoh_get_instance())

struct _Buoh {
	GObject      parent;
	BuohPrivate *priv;
};

struct _BuohClass {
	GObjectClass  parent_class;
};

GType         buoh_get_type           (void);
Buoh         *buoh_get_instance       (void);
Buoh         *buoh_new                (void);

void          buoh_exit_app           (Buoh        *buoh);

void          buoh_create_main_window (Buoh        *buoh);
GtkTreeModel *buoh_get_comics_model   (Buoh        *buoh);
const gchar  *buoh_get_datadir        (Buoh        *buoh);

void          buoh_debug              (const gchar *format,
				       ...);


G_END_DECLS

#endif /* !BUOH_H */
