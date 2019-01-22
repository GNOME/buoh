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

#ifndef BUOH_VIEW_H
#define BUOH_VIEW_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "buoh-comic.h"
#include "buoh-settings.h"
#include "buoh-enums.h"

G_BEGIN_DECLS

#define BUOH_TYPE_VIEW buoh_view_get_type ()
G_DECLARE_FINAL_TYPE (BuohView, buoh_view, BUOH, VIEW, GtkNotebook)

#define BUOH_TYPE_VIEW_STATUS buoh_view_status_get_type ()

typedef enum {
        STATE_MESSAGE_WELCOME,
        STATE_MESSAGE_ERROR,
        STATE_COMIC_LOADING,
        STATE_COMIC_LOADED,
        STATE_EMPTY
} BuohViewStatus;

GtkWidget       *buoh_view_new                (void);

/* Zoom */
gboolean         buoh_view_is_min_zoom        (BuohView        *view);
gboolean         buoh_view_is_max_zoom        (BuohView        *view);
gboolean         buoh_view_is_normal_size     (BuohView        *view);
void             buoh_view_zoom_in            (BuohView        *view);
void             buoh_view_zoom_out           (BuohView        *view);
void             buoh_view_zoom_normal_size   (BuohView        *view);
void             buoh_view_zoom_best_fit      (BuohView        *view);
void             buoh_view_zoom_fit_width     (BuohView        *view);
BuohViewZoomMode buoh_view_get_zoom_mode      (BuohView        *view);
void             buoh_view_set_zoom_mode      (BuohView        *view,
                                               BuohViewZoomMode mode);

/* Status */
BuohViewStatus   buoh_view_get_status         (BuohView        *view);

/* Comic */
void             buoh_view_set_comic          (BuohView        *view,
                                               const BuohComic *comic);
BuohComic       *buoh_view_get_comic          (BuohView        *view);

/* Message */
void             buoh_view_set_message_title  (BuohView        *view,
                                               const gchar     *title);
void             buoh_view_set_message_text   (BuohView        *view,
                                               const gchar     *text);
void             buoh_view_set_message_icon   (BuohView        *view,
                                               const gchar     *icon);

void             buoh_view_clear              (BuohView        *view);

G_END_DECLS

#endif /* !BUOH_VIEW_H */
