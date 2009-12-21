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

#ifndef BUOH_PROPERTIES_DIALOG_H
#define BUOH_PROPERTIES_DIALOG_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "buoh-comic.h"
#include "buoh-comic-manager.h"

G_BEGIN_DECLS

typedef struct _BuohPropertiesDialog        BuohPropertiesDialog;
typedef struct _BuohPropertiesDialogClass   BuohPropertiesDialogClass;
typedef struct _BuohPropertiesDialogPrivate BuohPropertiesDialogPrivate;

#define BUOH_TYPE_PROPERTIES_DIALOG                  (buoh_properties_dialog_get_type())
#define BUOH_PROPERTIES_DIALOG(object)               (G_TYPE_CHECK_INSTANCE_CAST((object), BUOH_TYPE_PROPERTIES_DIALOG, BuohPropertiesDialog))
#define BUOH_PROPERTIES_DIALOG_CLASS(klass)          (G_TYPE_CHACK_CLASS_CAST((klass), BUOH_TYPE_PROPERTIES_DIALOG, BuohPropertiesDialogClass))
#define BUOH_IS_PROPERTIES_DIALOG(object)            (G_TYPE_CHECK_INSTANCE_TYPE((object), BUOH_TYPE_PROPERTIES_DIALOG))
#define BUOH_IS_PROPERTIES_DIALOG_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), BUOH_TYPE_PROPERTIES_DIALOG))
#define BUOH_PROPERTIES_DIALOG_GET_CLASS(object)     (G_TYPE_INSTANCE_GET_CLASS((object), BUOH_TYPE_PROPERTIES_DIALOG, BuohPropertiesDialogClass))

struct _BuohPropertiesDialog {
	GtkDialog                   parent;
	BuohPropertiesDialogPrivate *priv;
};

struct _BuohPropertiesDialogClass {
	GtkDialogClass   parent_class;
};

GType             buoh_properties_dialog_get_type          (void) G_GNUC_CONST;
GtkWidget        *buoh_properties_dialog_new               (void);

void              buoh_properties_dialog_set_comic_manager (BuohPropertiesDialog   *dialog,
							    BuohComicManager       *comic_manager);
BuohComicManager *buoh_properties_dialog_get_comic_manager (BuohPropertiesDialog   *dialog);

G_END_DECLS

#endif /* !BUOH_PROPERTIES_DIALOG_H */
