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

typedef struct _BuohViewComic        BuohViewComic;
typedef struct _BuohViewComicClass   BuohViewComicClass;
typedef struct _BuohViewComicPrivate BuohViewComicPrivate;

#define BUOH_TYPE_VIEW_COMIC                  (buoh_view_comic_get_type())
#define BUOH_VIEW_COMIC(object)               (G_TYPE_CHECK_INSTANCE_CAST((object), BUOH_TYPE_VIEW_COMIC, BuohViewComic))
#define BUOH_VIEW_COMIC_CLASS(klass)          (G_TYPE_CHACK_CLASS_CAST((klass), BUOH_TYPE_VIEW_COMIC, BuohViewComicClass))
#define BUOH_IS_VIEW_COMIC(object)            (G_TYPE_CHECK_INSTANCE_TYPE((object), BUOH_TYPE_VIEW_COMIC))
#define BUOH_IS_VIEW_COMIC_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), BUOH_TYPE_VIEW_COMIC))
#define BUOH_VIEW_COMIC_GET_CLASS(object)     (G_TYPE_INSTANCE_GET_CLASS((object), BUOH_TYPE_VIEW_COMIC, BuohViewComicClass))

struct _BuohViewComic {
        GtkViewport           parent;
        BuohViewComicPrivate *priv;
};

struct _BuohViewComicClass {
        GtkViewportClass   parent_class;
};

GType            buoh_view_comic_get_type       (void) G_GNUC_CONST;
GtkWidget       *buoh_view_comic_new            (BuohView        *view);

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
