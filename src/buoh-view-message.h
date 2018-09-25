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

#ifndef BUOH_VIEW_MESSAGE_H
#define BUOH_VIEW_MESSAGE_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BUOH_TYPE_VIEW_MESSAGE buoh_view_message_get_type ()
G_DECLARE_FINAL_TYPE (BuohViewMessage, buoh_view_message, BUOH, VIEW_MESSAGE, GtkViewport)

GtkWidget *buoh_view_message_new       (void);

void       buoh_view_message_set_title (BuohViewMessage *m_view,
                                        const gchar     *title);
void       buoh_view_message_set_text  (BuohViewMessage *m_view,
                                        const gchar     *text);
void       buoh_view_message_set_icon  (BuohViewMessage *m_view,
                                        const gchar     *icon);

G_END_DECLS

#endif /* !BUOH_VIEW_MESSAGE_H */
