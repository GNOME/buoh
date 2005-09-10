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

#include <libgnomevfs/gnome-vfs.h>
#include <string.h>

#include "buoh.h"
#include "buoh-comic-loader.h"

static GObjectClass *parent_class = NULL;

static void     buoh_comic_loader_init           (BuohComicLoader      *loader);
static void     buoh_comic_loader_class_init     (BuohComicLoaderClass *klass);
static void     buoh_comic_loader_finalize       (GObject              *object);

static void     buoh_comic_loader_update         (GdkPixbufLoader      *pixbuf_loader,
						  gint                  x,
						  gint                  y,
						  gint                  width,
						  gint                  height,
						  gpointer              gdata);
static void     buoh_comic_loader_open_finished  (GnomeVFSAsyncHandle  *handle,
						  GnomeVFSResult        result,
						  gpointer              gdata);
static void     buoh_comic_loader_close_finished (GnomeVFSAsyncHandle  *handle,
						  GnomeVFSResult        result,
						  gpointer              gdata);
static void     buoh_comic_loader_read_next      (GnomeVFSAsyncHandle  *handle,
						  GnomeVFSResult        result,
						  gpointer              buffer,
						  GnomeVFSFileSize      bytes_requested,
						  GnomeVFSFileSize      bytes_read,
						  gpointer              gdata);
static gpointer buoh_comic_loader_run_thread     (gpointer              gdata);

#define BUFFER_SIZE 2048

GType
buoh_comic_loader_get_type (void)
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

GQuark
buoh_comic_loader_error_quark (void)
{
	static GQuark quark = 0;
	
	if (quark == 0)
		quark = g_quark_from_static_string ("buoh-comic-loader-error-quark");

	return quark;
}

static void
buoh_comic_loader_init (BuohComicLoader *loader)
{
	loader->uri = NULL;
	loader->pixbuf_loader = NULL;
	loader->buffer = NULL;
	
	loader->loop = NULL;
	loader->handle = NULL;
	
	loader->thread_mutex = g_mutex_new ();
	loader->pixbuf_mutex = g_mutex_new ();
	loader->status_mutex = g_mutex_new ();
	
	loader->thread = NULL;
	loader->pixbuf = NULL;
	loader->status = LOADER_STATE_READY;
	loader->error = NULL; 
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

	buoh_debug ("comic-loader finalize");

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

	if (loader->buffer) {
		g_free (loader->buffer);
		loader->buffer = NULL;
	}
	
	if (loader->loop) {
		g_main_loop_unref (loader->loop);
		loader->loop = NULL;
	}

	if (loader->error) {
		g_error_free (loader->error);
		loader->error = NULL;
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
			  gint width, gint height, gpointer gdata)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (gdata);

	g_mutex_lock (loader->pixbuf_mutex);
	loader->pixbuf = gdk_pixbuf_loader_get_pixbuf (pixbuf_loader);
	g_mutex_unlock (loader->pixbuf_mutex);
}

static void
buoh_comic_loader_open_finished (GnomeVFSAsyncHandle *handle,
				 GnomeVFSResult       result,
				 gpointer             gdata)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (gdata);

	if (result != GNOME_VFS_OK) {
		g_clear_error (&(loader->error));
		loader->error = g_error_new (BUOH_COMIC_LOADER_ERROR,
					     (gint) result,
					     gnome_vfs_result_to_string (result));
			
		g_mutex_lock (loader->status_mutex);
		loader->status = LOADER_STATE_FAILED;
		g_mutex_unlock (loader->status_mutex);

		gnome_vfs_async_cancel (handle);

		if (g_main_loop_is_running (loader->loop))
			g_main_loop_quit (loader->loop);

		buoh_debug ("state finished");

		return;
	}

	buoh_debug ("resolved");

	loader->pixbuf_loader = gdk_pixbuf_loader_new ();
	g_signal_connect (G_OBJECT (loader->pixbuf_loader), "area-updated",
			  G_CALLBACK (buoh_comic_loader_update),
			  (gpointer) loader);

	loader->buffer = g_malloc (BUFFER_SIZE);
	gnome_vfs_async_read (handle, loader->buffer, BUFFER_SIZE,
			      buoh_comic_loader_read_next,
			      (gpointer) loader);
}

static void
buoh_comic_loader_close_finished (GnomeVFSAsyncHandle *handle,
				  GnomeVFSResult       result,
				  gpointer             gdata)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (gdata);

	gdk_pixbuf_loader_close (loader->pixbuf_loader, NULL);

	g_mutex_lock (loader->pixbuf_mutex);
	loader->pixbuf = gdk_pixbuf_loader_get_pixbuf (loader->pixbuf_loader);
	g_mutex_unlock (loader->pixbuf_mutex);

	g_object_unref (loader->pixbuf_loader);
	loader->pixbuf_loader = NULL;

	g_mutex_lock (loader->status_mutex);
	loader->status = LOADER_STATE_FINISHED;
	g_mutex_unlock (loader->status_mutex);

	if (g_main_loop_is_running (loader->loop))
		g_main_loop_quit (loader->loop);
}

static void
buoh_comic_loader_read_next (GnomeVFSAsyncHandle *handle,
			     GnomeVFSResult       result,
			     gpointer             buffer,
			     GnomeVFSFileSize     bytes_requested,
			     GnomeVFSFileSize     bytes_read,
			     gpointer             gdata)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (gdata);

	if (loader->status == LOADER_STATE_STOPPING) {
		buoh_debug ("Stopping loader");
		
		gdk_pixbuf_loader_close (loader->pixbuf_loader, NULL);
		g_object_unref (loader->pixbuf_loader);
		loader->pixbuf_loader = NULL;

		g_free (loader->buffer);
		loader->buffer = NULL;

		gnome_vfs_async_cancel (handle);

		if (g_main_loop_is_running (loader->loop))
			g_main_loop_quit (loader->loop);

		return;
	}
	
	switch (result) {
	case GNOME_VFS_OK:
		gdk_pixbuf_loader_write (loader->pixbuf_loader,
					 (guchar *) buffer,
					 bytes_read, NULL);
		memset (loader->buffer, 0, BUFFER_SIZE);
		gnome_vfs_async_read (handle, loader->buffer, BUFFER_SIZE,
				      buoh_comic_loader_read_next,
				      (gpointer) loader);
		break;
	case GNOME_VFS_ERROR_EOF:
		gnome_vfs_async_close (handle,
				       buoh_comic_loader_close_finished,
				       (gpointer) loader);
		g_free (loader->buffer);
		loader->buffer = NULL;
		break;
	default:
		g_clear_error (&(loader->error));
		loader->error = g_error_new (BUOH_COMIC_LOADER_ERROR,
					     (gint) result,
					     gnome_vfs_result_to_string (result));
		
		g_mutex_lock (loader->status_mutex);
		loader->status = LOADER_STATE_FAILED;
		g_mutex_unlock (loader->status_mutex);

		gdk_pixbuf_loader_close (loader->pixbuf_loader, NULL);
		g_object_unref (loader->pixbuf_loader);
		loader->pixbuf_loader = NULL;

		g_free (loader->buffer);
		loader->buffer = NULL;
		
		gnome_vfs_async_cancel (handle);

		if (g_main_loop_is_running (loader->loop))
			g_main_loop_quit (loader->loop);
	}
}

static gpointer
buoh_comic_loader_run_thread (gpointer gdata)
{
	BuohComicLoader  *loader = BUOH_COMIC_LOADER (gdata);
	GMainContext     *context;

	buoh_debug ("comic_load_thread");

	context = g_main_context_new ();
	loader->loop = g_main_loop_new (context, TRUE);

	buoh_debug ("resolving . . .");
	gnome_vfs_async_open (&loader->handle, loader->uri,
			      GNOME_VFS_OPEN_READ,
			      GNOME_VFS_PRIORITY_DEFAULT,
			      buoh_comic_loader_open_finished,
			      (gpointer) loader);
	g_free (loader->uri);
	loader->uri = NULL;

	if (g_main_loop_is_running (loader->loop)) 
		g_main_loop_run (loader->loop);

	g_main_loop_unref (loader->loop);
	loader->loop = NULL;
	g_main_context_unref (context);
	
	buoh_debug ("thread died");

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

	g_clear_error (&(loader->error));
	
	loader->thread = g_thread_create (buoh_comic_loader_run_thread,
					  (gpointer)loader, TRUE, NULL);
}

void
buoh_comic_loader_stop (BuohComicLoader *loader)
{
	buoh_debug ("Stopping loader");

	g_mutex_lock (loader->status_mutex);
	loader->status = LOADER_STATE_STOPPING;
	g_mutex_unlock (loader->status_mutex);

	if (loader->pixbuf_loader) {
		gdk_pixbuf_loader_close (loader->pixbuf_loader, NULL);
		g_object_unref (loader->pixbuf_loader);
		loader->pixbuf_loader = NULL;
	}

	if (loader->buffer) {
		g_free (loader->buffer);
		loader->buffer = NULL;
	}

	if (loader->handle) {
		gnome_vfs_async_cancel (loader->handle);
	}

	if (loader->loop) {
		if (g_main_loop_is_running (loader->loop))
			g_main_loop_quit (loader->loop);
	}
}
