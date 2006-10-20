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
 *          Carlos García Campos <carlosgc@gnome.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>

#ifdef HAVE_LIBGNOMEUI
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#else
#include <gtk/gtkmain.h>
#include <gtk/gtkwindow.h>
#endif /* HAVE_LIBGNOMEUI */

#include "buoh.h"

gint
main (gint argc, gchar **argv)
{
	Buoh *buoh;

#ifdef ENABLE_NLS
        /* Initialize the i18n stuff */
        bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);
#endif

#ifdef HAVE_LIBGNOMEUI
	gnome_program_init (PACKAGE, VERSION,
			    LIBGNOMEUI_MODULE,
			    argc, argv,
			    GNOME_PARAM_NONE);
#else
	gtk_init (&argc, &argv);
#endif /* HAVE_LIBGNOMEUI */

	g_set_application_name (_("Buoh online comics browser"));
	gtk_window_set_default_icon_name ("buoh");

	/* Init threads */
	if (!g_thread_supported ()) {
		g_thread_init (NULL);
	}

	buoh = buoh_new ();
	buoh_create_main_window (buoh);

	gtk_main ();

	return 0;
}
