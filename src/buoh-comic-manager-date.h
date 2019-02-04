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

#define BUOH_TYPE_COMIC_MANAGER_DATE buoh_comic_manager_date_get_type ()
G_DECLARE_FINAL_TYPE (BuohComicManagerDate, buoh_comic_manager_date, BUOH, COMIC_MANAGER_DATE, BuohComicManager)

BuohComicManager *buoh_comic_manager_date_new                  (const gchar          *id,
                                                                const gchar          *title,
                                                                const gchar          *author,
                                                                const gchar          *language,
                                                                const gchar          *generic_uri);

void              buoh_comic_manager_date_set_offset           (BuohComicManagerDate *comic_manager,
                                                                guint                 offset);
void              buoh_comic_manager_date_set_restriction      (BuohComicManagerDate *comic_manager,
                                                                GDateWeekday          day);
void              buoh_comic_manager_date_set_first            (BuohComicManagerDate *comic_manager,
                                                                const gchar          *first);
gchar *           buoh_comic_manager_date_get_publication_days (BuohComicManagerDate *comic_manager);

G_END_DECLS

#endif /* BUOH_COMIC_MANAGER_DATE_H */
