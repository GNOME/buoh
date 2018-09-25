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

#ifndef BUOH_COMIC_LIST_H
#define BUOH_COMIC_LIST_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "buoh-view.h"
#include "buoh-comic-manager.h"

G_BEGIN_DECLS

#define BUOH_TYPE_COMIC_LIST buoh_comic_list_get_type ()
G_DECLARE_FINAL_TYPE (BuohComicList, buoh_comic_list, BUOH, COMIC_LIST, GtkBin)

GtkWidget        *buoh_comic_list_new             (void);

void              buoh_comic_list_set_view        (BuohComicList *comic_list,
                                                   BuohView      *view);
void              buoh_comic_list_set_model       (BuohComicList *comic_list,
                                                   GtkTreeModel  *model);
GtkWidget        *buoh_comic_list_get_list        (BuohComicList *comic_list);
void              buoh_comic_list_clear_selection (BuohComicList *comic_list);
BuohComicManager *buoh_comic_list_get_selected    (BuohComicList *comic_list);

G_END_DECLS

#endif /* !BUOH_COMIC_LIST_H */
