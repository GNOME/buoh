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
#include <glib/gstdio.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include "buoh.h"

static gboolean create_datadir     (void);
static gboolean create_comics_file (void);

static gboolean
create_datadir ()
{
	gchar *filename;

	filename = g_build_filename (g_get_home_dir (), ".buoh", NULL);

	if (! g_file_test (filename, G_FILE_TEST_IS_DIR)) {
		if (g_mkdir (filename, 0755) != 0) {
			g_free (filename);
			return FALSE;
		}
	}

	g_free (filename);
	
	return TRUE;
}

static gboolean
create_comics_file ()
{
	xmlTextWriterPtr  writer;
	gchar            *filename;
	
	filename = g_build_filename (g_get_home_dir (), ".buoh", "comics.xml", NULL);

	if (! g_file_test (filename, G_FILE_TEST_EXISTS)) {
		writer = xmlNewTextWriterFilename (filename, 0);

		if (!writer) {
			g_free (filename);
			return FALSE;
		}

		xmlTextWriterStartDocument (writer, NULL, NULL, NULL);
		xmlTextWriterStartElement (writer, BAD_CAST "comic_list");
		xmlTextWriterEndElement (writer);
		xmlTextWriterEndDocument (writer);
		xmlFreeTextWriter (writer);
	}

	g_free (filename);

	return TRUE;
}

gint
main (gint argc, gchar **argv)
{
	Buoh *buoh;

	gnome_program_init (PACKAGE, VERSION,
			    LIBGNOMEUI_MODULE,
			    argc, argv,
			    GNOME_PARAM_NONE);

	g_set_application_name (_("Buoh Comics Reader"));

	if (! create_datadir ()) {
		g_error(_("Cannot create buoh directory\n"));

		return 1;
	}

	if (! create_comics_file ()) {
		g_error(_("Cannot create buoh comics file\n"));

		return 1;
	}

	buoh = buoh_new ();
	buoh_create_main_window (buoh);
		   
	gtk_main ();
	   
	return 0;
}
