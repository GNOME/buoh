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
 */
 
/* Author: Pablo Arroyo Loma (zioma) <zioma@linups.org>
 *         Esteban Sánchez (steve-o) <esteban@steve-o.org>
 */      

#include <glib-object.h>

#include "comic.h"

/* Definicion de los CAST entre objetos y clases */
#define TYPE_BUOH						(buoh_get_type ())
#define BUOH(o)					(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_BUOH, Buoh))
#define BUOH_CLASS(k) 				(G_TYPE_CHECK_CLASS_CAST((k), TYPE_BUOH, BuohClass))
#define IS_BUOH(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_BUOH))
#define IS_BUOH_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_BUOH))
#define BUOH_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_BUOH, BuohClass))

typedef struct _Buoh      Buoh;
typedef struct _BuohClass BuohClass;

/* Buoh Object */
struct _Buoh {
	GObject parent;
};

/* Buoh Class */
struct _BuohClass {
	GObjectClass parent_class;
};

/* Public methods */
GType  buoh_get_type      (void);

Buoh  *buoh_new ();

/* Get the glade file and connect the signals for the main window */
void  buoh_gui_setup (Buoh *buoh);

/* Add a simple comic in the comic list*/
void  buoh_load_comic_simple (Buoh *buoh, gpointer *comic);

/* Get a widget from the Interface */
GtkWidget  *buoh_get_widget (Buoh *buoh, const gchar *widget);

/* Show the main window */
void  buoh_gui_show (Buoh *buoh);

/* Get / Set the current comic */
void   buoh_set_current_comic (Buoh *buoh, Comic *comic);
Comic *buoh_get_current_comic (Buoh *buoh);

void buoh_comic_zoom_in     (Buoh *buoh);
void buoh_comic_zoom_out    (Buoh *buoh);
void buoh_comic_normal_size (Buoh *buoh);
