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
 *  Authors : Carlos Garc�a Campos <carlosgc@gnome.org>
 */

#include <libgnomevfs/gnome-vfs.h>

#include "buoh-comic-loader.h"

static GObjectClass *parent_class = NULL;

static void buoh_comic_loader_init                   (BuohComicLoader      *loader);
static void buoh_comic_loader_class_init             (BuohComicLoaderClass *klass);
static void buoh_comic_loader_finalize               (GObject              *object);

static void     buoh_comic_loader_update     (GdkPixbufLoader *pixbuf_loader, gint x, gint y,
					      gint width, gint height, gpointer *gdata);
static gpointer buoh_comic_loader_run_thread (gpointer gdata);

GType
buoh_comic_loader_get_type ()
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (BuohComicLoaderClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) buoh_comic_loader_class_init,
			NULL,
			NULL,
			sizeof (BuohComicLoader),
			0,
			(GInstanceInitFunc) buoh_comic_loader_init
		};

		type = g_type_register_static (G_TYPE_OBJECT, "BuohComicLoader",
					       &info, 0);
	}

	return type;
}

static void
buoh_comic_loader_init (BuohComicLoader *loader)
{
	loader->uri = NULL;
	
	loader->thread_mutex = g_mutex_new ();
	loader->pixbuf_mutex = g_mutex_new ();
	loader->status_mutex = g_mutex_new ();
	
	loader->thread = NULL;
	loader->pixbuf = NULL;
	loader->status = LOADER_STATE_READY;
}

static void
buoh_comic_loader_class_init (BuohComicLoaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = buoh_comic_loader_finalize;
}

static void
buoh_comic_loader_finalize (GObject *object)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (object);

	g_debug ("comic-loader finalize\n");

	if (loader->uri) {
		g_free (loader->uri);
		loader->uri = NULL;
	}

	switch (loader->status) {
	case LOADER_STATE_RUNNING:
		buoh_comic_loader_stop (loader);
		/* Do not break! */
	case LOADER_STATE_STOPPING:
		g_thread_join (loader->thread);
		break;
	default:
		break;
	}
	
	if (loader->thread_mutex) {
		g_mutex_free (loader->thread_mutex);
		loader->thread_mutex = NULL;
	}
	
	if (loader->pixbuf_mutex) {
		g_mutex_free (loader->pixbuf_mutex);
		loader->pixbuf_mutex = NULL;
	}

	if (loader->status_mutex) {
		g_mutex_free (loader->status_mutex);
		loader->status_mutex = NULL;
	}

	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

BuohComicLoader *
buoh_comic_loader_new (void)
{
	BuohComicLoader *loader;

	loader = BUOH_COMIC_LOADER (g_object_new (BUOH_TYPE_COMIC_LOADER, NULL));

	return loader;
}

static void
buoh_comic_loader_update (GdkPixbufLoader *pixbuf_loader, gint x, gint y,
			  gint width, gint height, gpointer *gdata)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (gdata);

	g_mutex_lock (loader->pixbuf_mutex);
	loader->pixbuf = gdk_pixbuf_loader_get_pixbuf (pixbuf_loader);
	g_mutex_unlock (loader->pixbuf_mutex);
}

static gpointer
buoh_comic_loader_run_thread (gpointer gdata)
{
	BuohComicLoader  *loader = BUOH_COMIC_LOADER (gdata);
	GnomeVFSHandle   *read_handle;
	GnomeVFSResult    result;
	GnomeVFSFileSize  bytes_read;
	GdkPixbufLoader  *pixbuf_loader;
	guint             buffer[2048];

	g_debug ("comic_load_thread");

/*	g_mutex_lock (loader->status_mutex);
	loader->status = LOADER_STATE_RUNNING;
	g_mutex_unlock (loader->status_mutex);*/

	result = gnome_vfs_open (&read_handle, loader->uri, GNOME_VFS_OPEN_READ);
	g_free (loader->uri);
	loader->uri = NULL;

	if (result != GNOME_VFS_OK) {
		g_print ("Error %d - %s\n", result,
			 gnome_vfs_result_to_string (result));

		g_mutex_lock (loader->status_mutex);
		loader->status = LOADER_STATE_FAILED;
		g_mutex_unlock (loader->status_mutex);

		g_mutex_unlock (loader->thread_mutex);

		return NULL;
	}

	pixbuf_loader = gdk_pixbuf_loader_new ();
	g_signal_connect (G_OBJECT (pixbuf_loader), "area-updated",
			  G_CALLBACK (buoh_comic_loader_update),
			  (gpointer) loader);

	result = gnome_vfs_read (read_handle, buffer,
				 2048, &bytes_read);

	while ((result != GNOME_VFS_ERROR_EOF) &&
	       (result == GNOME_VFS_OK)) {

/*		g_usleep (1000000);*/

		if (loader->status == LOADER_STATE_STOPPING) {
			g_debug ("Stopping loader");
			gdk_pixbuf_loader_close (pixbuf_loader, NULL);
			g_object_unref (pixbuf_loader);
			gnome_vfs_close (read_handle);

			g_debug ("thread died");

			g_mutex_unlock (loader->thread_mutex);

			return NULL;
		}
		
		gdk_pixbuf_loader_write (pixbuf_loader,
					 (guchar *) buffer,
					 bytes_read, NULL);

		result = gnome_vfs_read (read_handle, buffer,
					 2048, &bytes_read);
	}

	if (result != GNOME_VFS_ERROR_EOF)
		g_print ("Error %d - %s\n", result,
			 gnome_vfs_result_to_string (result));

	gdk_pixbuf_loader_close (pixbuf_loader, NULL);

	g_mutex_lock (loader->pixbuf_mutex);
	loader->pixbuf = gdk_pixbuf_loader_get_pixbuf (pixbuf_loader);
	g_mutex_unlock (loader->pixbuf_mutex);

	g_object_unref (pixbuf_loader);
	gnome_vfs_close (read_handle);

	g_debug ("thread died");

	g_mutex_lock (loader->status_mutex);
	loader->status = LOADER_STATE_FINISHED;
	g_mutex_unlock (loader->status_mutex);

	g_mutex_unlock (loader->thread_mutex);
	
	return NULL;
}

void
buoh_comic_loader_run (BuohComicLoader *loader, const gchar *uri)
{
	g_return_if_fail (uri != NULL);

	g_mutex_lock (loader->status_mutex);
	loader->status = LOADER_STATE_READY;
	g_mutex_unlock (loader->status_mutex);

	g_mutex_lock (loader->pixbuf_mutex);
	loader->pixbuf = NULL;
	g_mutex_unlock (loader->pixbuf_mutex);

	loader->uri = g_strdup (uri);

	g_mutex_lock (loader->thread_mutex);

	g_mutex_lock (loader->status_mutex);
	loader->status = LOADER_STATE_RUNNING;
	g_mutex_unlock (loader->status_mutex);
	
	loader->thread = g_thread_create (buoh_comic_loader_run_thread,
					  (gpointer)loader, TRUE, NULL);
}

void
buoh_comic_loader_stop (BuohComicLoader *loader)
{
	g_debug ("buoh_comic_loader_stop");
	
	g_mutex_lock (loader->status_mutex);
	loader->status = LOADER_STATE_STOPPING;
	g_mutex_unlock (loader->status_mutex);
}