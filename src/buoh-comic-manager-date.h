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
 *  Author: Esteban Sánchez (steve-o) <esteban@steve-o.org>
 */

#ifndef BUOH_COMIC_MANAGER_DATE_H
#define BUOH_COMIC_MANAGER_DATE_H

#include <glib-object.h>

#include "buoh-comic-manager.h"
#include "buoh-comic.h"

G_BEGIN_DECLS

typedef struct _BuohComicManagerDate        BuohComicManagerDate;
typedef struct _BuohComicManagerDateClass   BuohComicManagerDateClass;
typedef struct _BuohComicManagerDatePrivate BuohComicManagerDatePrivate;

#define BUOH_COMIC_MANAGER_DATE_TYPE	     (buoh_comic_manager_date_get_type ())
#define BUOH_COMIC_MANAGER_DATE(o)	     (G_TYPE_CHECK_INSTANCE_CAST ((o), BUOH_COMIC_MANAGER_DATE_TYPE, BuohComicManagerDate))
#define BUOH_COMIC_MANAGER_DATE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), BUOH_COMIC_MANAGER_DATE_TYPE, BuohComicManagerDateClass))
#define BUOH_IS_COMIC_MANAGER_DATE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), BUOH_COMIC_MANAGER_DATE_TYPE))
#define BUOH_IS_COMIC_MANAGER_DATE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), BUOH_COMIC_MANAGER_DATE_TYPE))
#define BUOH_COMIC_MANAGER_DATE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), BUOH_COMIC_MANAGER_DATE_TYPE, BuohComicManagerDateClass))

struct _BuohComicManagerDate {
	BuohComicManager parent;

	BuohComicManagerDatePrivate *priv;
};

struct _BuohComicManagerDateClass {
	BuohComicManagerClass parent_class;
};

GType             buoh_comic_manager_date_get_type        (void);

BuohComicManager *buoh_comic_manager_date_new             (const gchar *id,
							   const gchar *title,
							   const gchar *author,
							   const gchar *language,
							   const gchar *generic_uri);

void              buoh_comic_manager_date_set_restriction (BuohComicManagerDate *comic_manager,
							   GDateWeekday day);
void              buoh_comic_manager_date_set_first       (BuohComicManagerDate *comic_manager,
							   gchar *first);

G_END_DECLS;

#endif
