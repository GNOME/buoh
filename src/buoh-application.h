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

#define BUOH_TYPE_APPLICATION                  (buoh_application_get_type())
G_DECLARE_FINAL_TYPE (BuohApplication, buoh_application, BUOH, APPLICATION, GtkApplication)

BuohApplication *buoh_application_get_instance       (void);
BuohApplication *buoh_application_new                (void);

void             buoh_application_activate           (GApplication    *buoh);
GtkTreeModel    *buoh_application_get_comics_model   (BuohApplication *buoh);
const gchar     *buoh_application_get_datadir        (BuohApplication *buoh);

void             buoh_debug                          (const gchar     *format,
                                                      ...);


G_END_DECLS

#endif /* !BUOH_APPLICATION_H */
