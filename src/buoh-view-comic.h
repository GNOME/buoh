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

#ifndef BUOH_VIEW_COMIC_H
#define BUOH_VIEW_COMIC_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "buoh-view.h"

G_BEGIN_DECLS

#define BUOH_TYPE_VIEW_COMIC buoh_view_comic_get_type ()
G_DECLARE_FINAL_TYPE (BuohViewComic, buoh_view_comic, BUOH, VIEW_COMIC, GtkViewport)

GtkWidget       *buoh_view_comic_new            (void);
void             buoh_view_comic_setup          (BuohViewComic   *c_view,
                                                 BuohView        *view);

gboolean         buoh_view_comic_is_min_zoom    (BuohViewComic   *c_view);
gboolean         buoh_view_comic_is_max_zoom    (BuohViewComic   *c_view);
gboolean         buoh_view_comic_is_normal_size (BuohViewComic   *c_view);
void             buoh_view_comic_zoom_in        (BuohViewComic   *c_view);
void             buoh_view_comic_zoom_out       (BuohViewComic   *c_view);
void             buoh_view_comic_normal_size    (BuohViewComic   *c_view);
void             buoh_view_comic_best_fit       (BuohViewComic   *c_view);
void             buoh_view_comic_fit_width      (BuohViewComic   *c_view);
BuohViewZoomMode buoh_view_comic_get_zoom_mode  (BuohViewComic   *c_view);
void             buoh_view_comic_set_zoom_mode  (BuohViewComic   *c_view,
                                                 BuohViewZoomMode mode);

G_END_DECLS

#endif /* !BUOH_VIEW_COMIC_H */
