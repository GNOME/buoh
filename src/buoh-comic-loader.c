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

#include <gconf/gconf-client.h>

#include "buoh.h"
#include "buoh-comic-loader.h"

struct _BuohComicLoaderPrivate {
	GMainLoop       *loop;
	GConfClient     *gconf_client;
	SoupSession     *session;
	SoupMessage     *msg;

	gchar           *uri;
	GdkPixbufLoader *pixbuf_loader;
};

#define BUOH_COMIC_LOADER_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_COMIC_LOADER, BuohComicLoaderPrivate))

#define GCONF_HTTP_PROXY_DIR "/system/http_proxy"
#define GCONF_USE_HTTP_PROXY "/system/http_proxy/use_http_proxy"
#define	GCONF_HTTP_PROXY_HOST "/system/http_proxy/host"
#define GCONF_HTTP_PROXY_PORT "/system/http_proxy/port"
#define GCONF_HTTP_PROXY_USE_AUTHENTICATION "/system/http_proxy/use_authentication"
#define GCONF_HTTP_PROXY_AUTHENTICATION_USER "/system/http_proxy/authentication_user"
#define GCONF_HTTP_PROXY_AUTHENTICATION_PASSWORD "/system/http_proxy/authentication_password"

static GObjectClass *parent_class = NULL;

static void     buoh_comic_loader_init          (BuohComicLoader      *loader);
static void     buoh_comic_loader_class_init    (BuohComicLoaderClass *klass);
static void     buoh_comic_loader_finalize      (GObject              *object);

static SoupUri *buoh_comic_loader_get_proxy_uri (BuohComicLoader      *loader);
static void     buoh_comic_loader_update_proxy  (GConfClient          *gconf_client,
						 guint                 cnxn_id,
						 GConfEntry           *entry,
						 gpointer              gdata);
static void     buoh_comic_loader_update        (GdkPixbufLoader      *pixbuf_loader,
						 gint                  x,
						 gint                  y,
						 gint                  width,
						 gint                  height,
						 gpointer              gdata);
static void     buoh_comic_loader_resolved      (SoupMessage          *msg,
						 gpointer              gdata);
static void     buoh_comic_loader_read_next     (SoupMessage          *msg,
						 gpointer              gdata);
static void     buoh_comic_loader_finished      (SoupMessage          *msg,
						 gpointer              gdata);
static gpointer buoh_comic_loader_run_thread    (gpointer              gdata);

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
	SoupUri *proxy_uri = NULL;
	
	loader->priv = BUOH_COMIC_LOADER_GET_PRIVATE (loader);
	
	loader->priv->uri = NULL;
	loader->priv->pixbuf_loader = NULL;

	loader->priv->loop = NULL;

	loader->priv->gconf_client = gconf_client_get_default ();
	proxy_uri = buoh_comic_loader_get_proxy_uri (loader);

	loader->priv->session = (proxy_uri != NULL) ?
		soup_session_async_new_with_options (SOUP_SESSION_PROXY_URI, proxy_uri, NULL) :
		soup_session_async_new ();

	if (proxy_uri)
		soup_uri_free (proxy_uri);

	gconf_client_add_dir (loader->priv->gconf_client,
			      GCONF_HTTP_PROXY_DIR,
			      GCONF_CLIENT_PRELOAD_NONE,
			      NULL);
	gconf_client_notify_add (loader->priv->gconf_client,
				 GCONF_HTTP_PROXY_DIR,
				 buoh_comic_loader_update_proxy,
				 (gpointer) loader,
				 NULL, NULL);
	
	loader->priv->msg = NULL;
	
	loader->thread_mutex = g_mutex_new ();
	loader->pixbuf_mutex = g_mutex_new ();
	loader->status_mutex = g_mutex_new ();
	
	loader->thread = NULL;
	loader->status = LOADER_STATE_READY;
	loader->error = NULL; 
}

static void
buoh_comic_loader_class_init (BuohComicLoaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (BuohComicLoaderPrivate));

	object_class->finalize = buoh_comic_loader_finalize;
}

static void
buoh_comic_loader_finalize (GObject *object)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (object);

	buoh_debug ("comic-loader finalize");

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

	if (loader->error) {
		g_error_free (loader->error);
		loader->error = NULL;
	}

	if (loader->priv->gconf_client) {
		g_object_unref (loader->priv->gconf_client);
		loader->priv->gconf_client = NULL;
	}

	if (loader->priv->uri) {
		g_free (loader->priv->uri);
		loader->priv->uri = NULL;
	}

	if (loader->priv->session) {
		g_object_unref (loader->priv->session);
		loader->priv->session = NULL;
	}

	if (loader->priv->loop) {
		g_main_loop_unref (loader->priv->loop);
		loader->priv->loop = NULL;
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

static SoupUri *
buoh_comic_loader_get_proxy_uri (BuohComicLoader *loader)
{
	GConfClient *gconf_client = loader->priv->gconf_client;
	SoupUri     *uri = NULL;
	
	if (gconf_client_get_bool (gconf_client, GCONF_USE_HTTP_PROXY, NULL)) {
		uri = g_new0 (SoupUri, 1);
		
		uri->protocol = SOUP_PROTOCOL_HTTP;
		
		uri->host = gconf_client_get_string (gconf_client,
						     GCONF_HTTP_PROXY_HOST,
						     NULL);
		uri->port = gconf_client_get_int (gconf_client,
						  GCONF_HTTP_PROXY_PORT,
						  NULL);
		
		if (gconf_client_get_bool (gconf_client, GCONF_HTTP_PROXY_USE_AUTHENTICATION, NULL)) {
			uri->user = gconf_client_get_string (
				gconf_client,
				GCONF_HTTP_PROXY_AUTHENTICATION_USER,
				NULL );
			uri->passwd = gconf_client_get_string (
				gconf_client,
				GCONF_HTTP_PROXY_AUTHENTICATION_PASSWORD,
				NULL);
		}
	}

	return uri;
}

static void
buoh_comic_loader_update_proxy (GConfClient *gconf_client, guint cnxn_id,
				GConfEntry *entry, gpointer gdata)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (gdata);
	SoupUri         *uri = NULL;

	buoh_debug ("Proxy configuration changed");
	
	uri = buoh_comic_loader_get_proxy_uri (loader);
	g_object_set (G_OBJECT (loader->priv->session),
		      "proxy-uri", uri, NULL);

	if (uri)
		soup_uri_free (uri);
}

static void
buoh_comic_loader_update (GdkPixbufLoader *pixbuf_loader, gint x, gint y,
			  gint width, gint height, gpointer gdata)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (gdata);
	GdkPixbuf       *pixbuf = NULL;

	g_mutex_lock (loader->pixbuf_mutex);
	pixbuf = gdk_pixbuf_loader_get_pixbuf (pixbuf_loader);
	loader->pixbuf = pixbuf;
	g_mutex_unlock (loader->pixbuf_mutex);
}

static void
buoh_comic_loader_resolved (SoupMessage *msg, gpointer gdata)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (gdata);

	buoh_debug ("resolved");

	if (SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) {
		loader->priv->pixbuf_loader = gdk_pixbuf_loader_new ();
		g_signal_connect (G_OBJECT (loader->priv->pixbuf_loader),
				  "area-updated",
				  G_CALLBACK (buoh_comic_loader_update),
				  (gpointer) loader);
	} else {
		g_clear_error (&(loader->error));
		loader->error = g_error_new (BUOH_COMIC_LOADER_ERROR,
					     (gint) msg->status_code,
					     msg->reason_phrase);

		g_mutex_lock (loader->status_mutex);
		loader->status = LOADER_STATE_FAILED;
		g_mutex_unlock (loader->status_mutex);

		soup_message_set_status (msg, SOUP_STATUS_CANCELLED);
		soup_session_cancel_message (loader->priv->session, msg);
	}
}

static void
buoh_comic_loader_read_next (SoupMessage *msg, gpointer gdata)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (gdata);

	if (loader->status == LOADER_STATE_STOPPING) {
		buoh_debug ("Stopping loader");

		return;
	}

	if (SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) {
		gdk_pixbuf_loader_write (loader->priv->pixbuf_loader,
					 (guchar *) msg->response.body,
					 msg->response.length, NULL);
	} else {
		g_clear_error (&(loader->error));
		loader->error = g_error_new (BUOH_COMIC_LOADER_ERROR,
					     (gint) msg->status_code,
					     msg->reason_phrase);

		g_mutex_lock (loader->status_mutex);
		loader->status = LOADER_STATE_FAILED;
		g_mutex_unlock (loader->status_mutex);

		soup_message_set_status (msg, SOUP_STATUS_CANCELLED);
		soup_session_cancel_message (loader->priv->session, msg);
	}
}

static void
buoh_comic_loader_finished (SoupMessage *msg, gpointer gdata)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (gdata);
	GdkPixbuf       *pixbuf = NULL;

	buoh_debug ("loader finish");

	switch (loader->status) {
	case LOADER_STATE_RUNNING:
		if (loader->priv->pixbuf_loader) {
			gdk_pixbuf_loader_close (loader->priv->pixbuf_loader, NULL);
			
			g_mutex_lock (loader->pixbuf_mutex);
			pixbuf = gdk_pixbuf_loader_get_pixbuf (loader->priv->pixbuf_loader);
			loader->pixbuf = GDK_IS_PIXBUF (pixbuf) ? g_object_ref (pixbuf) : NULL;
			g_mutex_unlock (loader->pixbuf_mutex);

			g_object_unref (loader->priv->pixbuf_loader);
			loader->priv->pixbuf_loader = NULL;
		}

		g_mutex_lock (loader->status_mutex);
		loader->status = LOADER_STATE_FINISHED;
		g_mutex_unlock (loader->status_mutex);

		break;
	case LOADER_STATE_FAILED:
		if (loader->priv->pixbuf_loader) {
			gdk_pixbuf_loader_close (loader->priv->pixbuf_loader, NULL);

			g_mutex_lock (loader->pixbuf_mutex);
			if (GDK_IS_PIXBUF (loader->pixbuf))
				g_object_unref (loader->pixbuf);
			loader->pixbuf = NULL;
			g_mutex_unlock (loader->pixbuf_mutex);

			g_object_unref (loader->priv->pixbuf_loader);
			loader->priv->pixbuf_loader = NULL;
		}

		break;
	case LOADER_STATE_STOPPING:
		if (loader->priv->pixbuf_loader) {
			gdk_pixbuf_loader_close (loader->priv->pixbuf_loader, NULL);

			g_mutex_lock (loader->pixbuf_mutex);
			if (GDK_IS_PIXBUF (loader->pixbuf))
				g_object_unref (loader->pixbuf);
			loader->pixbuf = NULL;
			g_mutex_unlock (loader->pixbuf_mutex);
			
			g_object_unref (loader->priv->pixbuf_loader);
			loader->priv->pixbuf_loader = NULL;
		}
		
		g_mutex_lock (loader->status_mutex);
		loader->status = LOADER_STATE_FINISHED;
		g_mutex_unlock (loader->status_mutex);
		
		break;
	default:
		break;
	}
		
	if (g_main_loop_is_running (loader->priv->loop))
		g_main_loop_quit (loader->priv->loop);
}

static gpointer
buoh_comic_loader_run_thread (gpointer gdata)
{
	BuohComicLoader  *loader = BUOH_COMIC_LOADER (gdata);
	GMainContext     *context;

	buoh_debug ("comic_load_thread");

	context = g_main_context_new ();
	loader->priv->loop = g_main_loop_new (context, TRUE);

	loader->priv->msg = soup_message_new (SOUP_METHOD_GET, loader->priv->uri);
	soup_message_set_flags (loader->priv->msg,
				SOUP_MESSAGE_OVERWRITE_CHUNKS);
	soup_message_add_handler (loader->priv->msg, SOUP_HANDLER_PRE_BODY,
				  buoh_comic_loader_resolved,
				  (gpointer) loader);
	soup_message_add_handler (loader->priv->msg, SOUP_HANDLER_BODY_CHUNK,
				  buoh_comic_loader_read_next,
				  (gpointer) loader);
	
	buoh_debug ("resolving . . .");
	
	soup_session_queue_message (loader->priv->session, loader->priv->msg,
				    buoh_comic_loader_finished,
				    (gpointer) loader);
	g_free (loader->priv->uri);
	loader->priv->uri = NULL;

	if (g_main_loop_is_running (loader->priv->loop)) 
		g_main_loop_run (loader->priv->loop);

	g_main_loop_unref (loader->priv->loop);
	loader->priv->loop = NULL;
	g_main_context_unref (context);

	buoh_debug ("thread died");

	g_mutex_unlock (loader->thread_mutex);

	return NULL;
}

void
buoh_comic_loader_run (BuohComicLoader *loader, const gchar *uri)
{
	g_return_if_fail (BUOH_IS_COMIC_LOADER (loader));
	g_return_if_fail (uri != NULL);

	g_mutex_lock (loader->status_mutex);
	loader->status = LOADER_STATE_READY;
	g_mutex_unlock (loader->status_mutex);

	g_mutex_lock (loader->pixbuf_mutex);
	loader->pixbuf = NULL;
	g_mutex_unlock (loader->pixbuf_mutex);

	loader->priv->uri = g_strdup (uri);

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
	g_return_if_fail (BUOH_IS_COMIC_LOADER (loader));
	
	buoh_debug ("Stopping loader");

	g_mutex_lock (loader->status_mutex);
	loader->status = LOADER_STATE_STOPPING;
	g_mutex_unlock (loader->status_mutex);

	soup_message_set_status (loader->priv->msg, SOUP_STATUS_CANCELLED);
	soup_session_cancel_message (loader->priv->session, loader->priv->msg);
}
