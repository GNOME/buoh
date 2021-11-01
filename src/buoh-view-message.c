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

#include <glib.h>
#include <gtk/gtk.h>

#include "buoh-view-message.h"

struct _BuohViewMessage {
        GtkViewport parent;

        GtkWidget *title;
        GtkWidget *text;
        GtkWidget *icon;
};

static void buoh_view_message_init          (BuohViewMessage *m_view);
static void buoh_view_message_class_init    (BuohViewMessageClass *klass);

G_DEFINE_FINAL_TYPE (BuohViewMessage, buoh_view_message, GTK_TYPE_VIEWPORT)

static void
buoh_view_message_init (BuohViewMessage *m_view)
{
        gtk_widget_init_template (GTK_WIDGET (m_view));
}

static void
buoh_view_message_class_init (BuohViewMessageClass *klass)
{
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/buoh/ui/view-message.ui");

        gtk_widget_class_bind_template_child (widget_class, BuohViewMessage, title);
        gtk_widget_class_bind_template_child (widget_class, BuohViewMessage, text);
        gtk_widget_class_bind_template_child (widget_class, BuohViewMessage, icon);
}

GtkWidget *
buoh_view_message_new (void)
{
        GtkWidget *m_view;

        m_view = GTK_WIDGET (g_object_new (BUOH_TYPE_VIEW_MESSAGE, NULL));
        return m_view;
}

void
buoh_view_message_set_title (BuohViewMessage *m_view, const gchar *title)
{
        g_return_if_fail (BUOH_IS_VIEW_MESSAGE (m_view));
        g_return_if_fail (title != NULL);

        gtk_label_set_text (GTK_LABEL (m_view->title), title);
}

void
buoh_view_message_set_text (BuohViewMessage *m_view, const gchar *text)
{
        g_return_if_fail (BUOH_IS_VIEW_MESSAGE (m_view));
        g_return_if_fail (text != NULL);

        gtk_label_set_markup (GTK_LABEL (m_view->text), text);
}

void
buoh_view_message_set_icon (BuohViewMessage *m_view, const gchar *icon)
{
        g_return_if_fail (BUOH_IS_VIEW_MESSAGE (m_view));
        g_return_if_fail (icon != NULL);

        gtk_image_set_from_icon_name (GTK_IMAGE (m_view->icon), icon, GTK_ICON_SIZE_DIALOG);
}
