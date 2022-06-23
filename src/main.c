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
 *  Authors: Pablo Arroyo Loma (zioma) <zioma@linups.org>
 *           Esteban Sanchez Munoz (steve-o) <esteban@steve-o.org>
 *           Carlos García Campos <carlosgc@gnome.org>
 */

#include <glib.h>
#include <glib/gi18n.h>

#ifdef ENABLE_INTROSPECTION
#  include <girepository.h>
#endif

#include <gtk/gtk.h>

#include "buoh-application.h"

int
main (int argc, char *argv[])
{
#ifdef ENABLE_INTROSPECTION
        const char *introspect_dump_prefix = "--introspect-dump=";

        if (argc == 2 && g_str_has_prefix (argv[1], introspect_dump_prefix)) {
                g_autoptr (GError) error = NULL;
                if (!g_irepository_dump (argv[1] + strlen(introspect_dump_prefix), &error)) {
                        g_critical ("Failed to dump introspection data: %s", error->message);
                        return EXIT_FAILURE;
                }

                return EXIT_SUCCESS;
        }
#endif

        g_autoptr (BuohApplication) buoh;

        bindtextdomain (GETTEXT_PACKAGE, LOCALE_DIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);

        buoh = buoh_application_new ();

        return g_application_run (G_APPLICATION (buoh), argc, argv);
}
