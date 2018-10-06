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

#ifndef BUOH_APPLICATION_H
#define BUOH_APPLICATION_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

enum {
        COMIC_LIST_VISIBLE,
        COMIC_LIST_TITLE,
        COMIC_LIST_AUTHOR,
        COMIC_LIST_LANGUAGE,
        COMIC_LIST_COMIC_MANAGER,
        N_COLUMNS
};

typedef struct _BuohApplication        BuohApplication;
typedef struct _BuohApplicationClass   BuohApplicationClass;
typedef struct _BuohApplicationPrivate BuohApplicationPrivate;

#define BUOH_TYPE_APPLICATION                  (buoh_application_get_type())
#define BUOH_APPLICATION(object)               (G_TYPE_CHECK_INSTANCE_CAST((object), BUOH_TYPE_APPLICATION, BuohApplication))
#define BUOH_APPLICATION_CLASS(klass)          (G_TYPE_CHACK_CLASS_CAST((klass), BUOH_TYPE_APPLICATION, BuohApplicationClass))
#define BUOH_IS_APPLICATION(object)            (G_TYPE_CHECK_INSTANCE_TYPE((object), BUOH_TYPE_APPLICATION))
#define BUOH_IS_APPLICATION_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), BUOH_TYPE_APPLICATION))
#define BUOH_APPLICATION_GET_CLASS(object)     (G_TYPE_INSTANCE_GET_CLASS((object), BUOH_TYPE_APPLICATION, BuohApplicationClass))

struct _BuohApplication {
        GObject                 parent;
        BuohApplicationPrivate *priv;
};

struct _BuohApplicationClass {
        GObjectClass  parent_class;
};

GType            buoh_application_get_type           (void) G_GNUC_CONST;
BuohApplication *buoh_application_get_instance       (void);
BuohApplication *buoh_application_new                (void);

void             buoh_application_exit               (BuohApplication *buoh);

void             buoh_application_create_main_window (BuohApplication *buoh);
GtkTreeModel    *buoh_application_get_comics_model   (BuohApplication *buoh);
const gchar     *buoh_application_get_datadir        (BuohApplication *buoh);

void             buoh_debug                          (const gchar     *format,
                                                      ...);


G_END_DECLS

#endif /* !BUOH_APPLICATION_H */
