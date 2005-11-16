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

#ifndef BUOH_WINDOW_H
#define BUOH_WINDOW_H

#include <glib-object.h>
#include <gtk/gtkwindow.h>

G_BEGIN_DECLS

typedef struct _BuohWindow        BuohWindow;
typedef struct _BuohWindowClass   BuohWindowClass;
typedef struct _BuohWindowPrivate BuohWindowPrivate;

#define BUOH_TYPE_WINDOW                  (buoh_window_get_type())
#define BUOH_WINDOW(object)               (G_TYPE_CHECK_INSTANCE_CAST((object), BUOH_TYPE_WINDOW, BuohWindow))
#define BUOH_WINDOW_CLASS(klass)          (G_TYPE_CHACK_CLASS_CAST((klass), BUOH_TYPE_WINDOW, BuohWindowClass))
#define BUOH_IS_WINDOW(object)            (G_TYPE_CHECK_INSTANCE_TYPE((object), BUOH_TYPE_WINDOW))
#define BUOH_IS_WINDOW_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), BUOH_TYPE_WINDOW))
#define BUOH_WINDOW_GET_CLASS(object)     (G_TYPE_INSTANCE_GET_CLASS((object), BUOH_TYPE_WINDOW, BuohWindowClass))

struct _BuohWindow {
	GtkWindow          parent;
	BuohWindowPrivate *priv;
};

struct _BuohWindowClass {
	GtkWindowClass     parent_class;
};

GType      buoh_window_get_type (void) G_GNUC_CONST;
GtkWidget *buoh_window_new      (void);

G_END_DECLS

#endif /* !BUOH_WINDOW_H */
