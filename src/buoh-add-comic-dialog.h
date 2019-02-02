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

#ifndef BUOH_ADD_COMIC_DIALOG_H
#define BUOH_ADD_COMIC_DIALOG_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BUOH_TYPE_ADD_COMIC_DIALOG buoh_add_comic_dialog_get_type ()
G_DECLARE_FINAL_TYPE (BuohAddComicDialog, buoh_add_comic_dialog, BUOH, ADD_COMIC_DIALOG, GtkDialog)

struct _BuohAddComicDialogClass {
        GtkDialogClass   parent_class;
};

GtkWidget *buoh_add_comic_dialog_new       (void);

G_END_DECLS

#endif /* !BUOH_ADD_COMIC_DIALOG_H */
