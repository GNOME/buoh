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

#ifndef BUOH_VIEW_H
#define BUOH_VIEW_H

#include <glib-object.h>
#include <gtk/gtknotebook.h>

#include "buoh-comic.h"

G_BEGIN_DECLS

typedef struct _BuohView        BuohView;
typedef struct _BuohViewClass   BuohViewClass;
typedef struct _BuohViewPrivate BuohViewPrivate;

#define BUOH_TYPE_VIEW                  (buoh_view_get_type())
#define BUOH_VIEW(object)               (G_TYPE_CHECK_INSTANCE_CAST((object), BUOH_TYPE_VIEW, BuohView))
#define BUOH_VIEW_CLASS(klass)          (G_TYPE_CHACK_CLASS_CAST((klass), BUOH_TYPE_VIEW, BuohViewClass))
#define BUOH_IS_VIEW(object)            (G_TYPE_CHECK_INSTANCE_TYPE((object), BUOH_TYPE_VIEW))
#define BUOH_IS_VIEW_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), BUOH_TYPE_VIEW))
#define BUOH_VIEW_GET_CLASS(object)     (G_TYPE_INSTANCE_GET_CLASS((object), BUOH_TYPE_VIEW, BuohViewClass))

#define BUOH_TYPE_VIEW_STATUS           (buoh_view_status_get_type ())

typedef enum {
	STATE_MESSAGE_WELCOME,
	STATE_MESSAGE_ERROR,
	STATE_COMIC_LOADING,
	STATE_COMIC_LOADED,
	STATE_EMPTY
} BuohViewStatus;

struct _BuohView {
	GtkNotebook      parent;
	BuohViewPrivate *priv;
};

struct _BuohViewClass {
	GtkNotebookClass   parent_class;
	
	void (* scale_changed) (BuohView *view);
};

GType          buoh_view_get_type          (void);
GType          buoh_view_status_get_type   (void);
GtkWidget     *buoh_view_new               (void);

gboolean       buoh_view_is_min_zoom       (BuohView    *view);
gboolean       buoh_view_is_max_zoom       (BuohView    *view);
gboolean       buoh_view_is_normal_size    (BuohView    *view);
void           buoh_view_zoom_in           (BuohView    *view);
void           buoh_view_zoom_out          (BuohView    *view);
void           buoh_view_normal_size       (BuohView    *view);

BuohViewStatus buoh_view_get_status        (BuohView    *view);

void           buoh_view_set_comic         (BuohView    *view,
					    BuohComic   *comic);
BuohComic     *buoh_view_get_comic         (BuohView    *view);

void           buoh_view_set_message_title (BuohView    *view,
					    const gchar *title);
void           buoh_view_set_message_text  (BuohView    *view,
					    const gchar *text);
void           buoh_view_set_message_icon  (BuohView    *view,
					    const gchar *icon);

void           buoh_view_clear           (BuohView *view);

G_END_DECLS

#endif /* !BUOH_VIEW_H */
