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
 
/* Authors: Pablo Arroyo Loma (zioma) <zioma@linups.org>
 *          Esteban Sanchez Munoz (steve-o) <esteban@steve-o.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgnomevfs/gnome-vfs.h>
#include <gnome.h>
#include <glade/glade.h>

#include "buoh.h"

gint
main (gint argc, gchar **argv)
{
	Buoh      *buoh;
	   
	gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
			    argc, argv,
			    GNOME_PARAM_NONE);
	   
	g_type_init ();
	   
	buoh = BUOH (buoh_new ());
	buoh_gui_setup (buoh);
	buoh_gui_show (buoh);	   
		   
	gtk_main();
	   
	g_free (buoh);

	return 0;
}
