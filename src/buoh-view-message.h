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

#ifndef BUOH_VIEW_MESSAGE_H
#define BUOH_VIEW_MESSAGE_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BuohViewMessage        BuohViewMessage;
typedef struct _BuohViewMessageClass   BuohViewMessageClass;
typedef struct _BuohViewMessagePrivate BuohViewMessagePrivate;

#define BUOH_TYPE_VIEW_MESSAGE                  (buoh_view_message_get_type())
#define BUOH_VIEW_MESSAGE(object)               (G_TYPE_CHECK_INSTANCE_CAST((object), BUOH_TYPE_VIEW_MESSAGE, BuohViewMessage))
#define BUOH_VIEW_MESSAGE_CLASS(klass)          (G_TYPE_CHACK_CLASS_CAST((klass), BUOH_TYPE_VIEW_MESSAGE, BuohViewMessageClass))
#define BUOH_IS_VIEW_MESSAGE(object)            (G_TYPE_CHECK_INSTANCE_TYPE((object), BUOH_TYPE_VIEW_MESSAGE))
#define BUOH_IS_VIEW_MESSAGE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), BUOH_TYPE_VIEW_MESSAGE))
#define BUOH_VIEW_MESSAGE_GET_CLASS(object)     (G_TYPE_INSTANCE_GET_CLASS((object), BUOH_TYPE_VIEW_MESSAGE, BuohViewMessageClass))

struct _BuohViewMessage {
	GtkViewport                 parent;
	BuohViewMessagePrivate *priv;
};

struct _BuohViewMessageClass {
	GtkViewportClass   parent_class;
};

GType      buoh_view_message_get_type  (void) G_GNUC_CONST;
GtkWidget *buoh_view_message_new       (void);

void       buoh_view_message_set_title (BuohViewMessage *m_view,
					const gchar     *title);
void       buoh_view_message_set_text  (BuohViewMessage *m_view,
					const gchar     *text);
void       buoh_view_message_set_icon  (BuohViewMessage *m_view,
					const gchar     *icon);

G_END_DECLS

#endif /* !BUOH_VIEW_MESSAGE_H */
