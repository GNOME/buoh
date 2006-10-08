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

#include <libsoup/soup.h>

#include "buoh.h"
#include "buoh-comic-loader.h"

/* BuohComicLoaderJob */
#define BUOH_TYPE_COMIC_LOADER_JOB          (buoh_comic_loader_job_get_type())
#define BUOH_COMIC_LOADER_JOB(object)       (G_TYPE_CHECK_INSTANCE_CAST((object), BUOH_TYPE_COMIC_LOADER_JOB, BuohComicLoaderJob))
#define BUOH_COMIC_LOADER_JOB_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST((klass), BUOH_TYPE_COMIC_LOADER_JOB, BuohComicLoaderJobClass))
#define BUOH_COMIC_LOADER_IS_JOB(object)    (G_TYPE_CHECK_INSTANCE_TYPE((object), BUOH_TYPE_COMIC_LOADER_JOB))

typedef struct _BuohComicLoaderJob      BuohComicLoaderJob;
typedef struct _BuohComicLoaderJobClass BuohComicLoaderJobClass;

struct _BuohComicLoaderJob {
	GObject parent;

	gboolean         canceled;
	
	gchar           *uri;
	BuohComicLoaderLoadFunc callback;
	gpointer         callback_data;
	
	SoupSession     *session;

	BuohComicLoader *loader;

	GError          *error;
};

struct _BuohComicLoaderJobClass {
	GObjectClass parent_class;
};

static GType buoh_comic_loader_job_get_type     (void);
static void  buoh_comic_loader_job_init         (BuohComicLoaderJob      *job);
static void  buoh_comic_loader_job_class_init   (BuohComicLoaderJobClass *klass);
static void  buoh_comic_loader_job_finalize     (GObject                 *object);

G_DEFINE_TYPE (BuohComicLoaderJob, buoh_comic_loader_job, G_TYPE_OBJECT)

struct _BuohComicLoaderPrivate {
	BuohComicLoaderJob *job;

	GError             *error;
};

#define BUOH_COMIC_LOADER_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_COMIC_LOADER, BuohComicLoaderPrivate))


enum {
	FINISHED,
	LAST_SIGNAL
};

static guint buoh_comic_loader_signals[LAST_SIGNAL] = { 0 };

static void     buoh_comic_loader_init          (BuohComicLoader      *loader);
static void     buoh_comic_loader_class_init    (BuohComicLoaderClass *klass);
static void     buoh_comic_loader_finalize      (GObject              *object);

static void     buoh_comic_loader_clear         (BuohComicLoader      *loader);

G_DEFINE_TYPE (BuohComicLoader, buoh_comic_loader, G_TYPE_OBJECT)

GQuark
buoh_comic_loader_error_quark (void)
{
	static GQuark quark = 0;
	
	if (G_UNLIKELY (quark == 0))
		quark = g_quark_from_static_string ("buoh-comic-loader-error-quark");

	return quark;
}

static void
buoh_comic_loader_init (BuohComicLoader *loader)
{
	loader->priv = BUOH_COMIC_LOADER_GET_PRIVATE (loader);
}

static void
buoh_comic_loader_class_init (BuohComicLoaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (BuohComicLoaderPrivate));

	buoh_comic_loader_signals [FINISHED] =
		g_signal_new ("finished",
			      BUOH_TYPE_COMIC_LOADER,
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BuohComicLoaderClass, finished),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	object_class->finalize = buoh_comic_loader_finalize;
}

static void
buoh_comic_loader_finalize (GObject *object)
{
	BuohComicLoader *loader = BUOH_COMIC_LOADER (object);

	buoh_debug ("comic-loader finalize");

	buoh_comic_loader_clear (loader);

	if (loader->priv->error) {
		g_error_free (loader->priv->error);
		loader->priv->error = NULL;
	}

	if (G_OBJECT_CLASS (buoh_comic_loader_parent_class)->finalize)
		(* G_OBJECT_CLASS (buoh_comic_loader_parent_class)->finalize) (object);
}

BuohComicLoader *
buoh_comic_loader_new (void)
{
	BuohComicLoader *loader;

	loader = BUOH_COMIC_LOADER (g_object_new (BUOH_TYPE_COMIC_LOADER, NULL));

	return loader;
}

/* BuohComicLoaderJob */
static void
buoh_comic_loader_job_init (BuohComicLoaderJob *job)
{
}

static void
buoh_comic_loader_job_class_init (BuohComicLoaderJobClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize = buoh_comic_loader_job_finalize;
}

static void
buoh_comic_loader_job_finalize (GObject *object)
{
	BuohComicLoaderJob *job = BUOH_COMIC_LOADER_JOB (object);

	buoh_debug ("comic-loader-job finalize");

	if (job->uri) {
		g_free (job->uri);
		job->uri = NULL;
	}

	if (job->session) {
		g_object_unref (job->session);
		job->session = NULL;
	}

	if (job->error) {
		g_error_free (job->error);
		job->error = NULL;
	}

	if (G_OBJECT_CLASS (buoh_comic_loader_job_parent_class)->finalize)
		(* G_OBJECT_CLASS (buoh_comic_loader_job_parent_class)->finalize) (object);
}

static BuohComicLoaderJob *
buoh_comic_loader_job_new (const gchar             *uri,
			   BuohComicLoaderLoadFunc  callback,
			   gpointer                 gdata)
{
	const gchar        *proxy_uri;
	BuohComicLoaderJob *job;

	job = g_object_new (BUOH_TYPE_COMIC_LOADER_JOB, NULL);

	job->uri = g_strdup (uri);
	job->callback = callback;
	job->callback_data = gdata;

	proxy_uri = buoh_get_http_proxy_uri (BUOH);
	if (proxy_uri) {
		SoupUri *soup_uri = soup_uri_new (proxy_uri);

		job->session = soup_session_sync_new_with_options (SOUP_SESSION_PROXY_URI, soup_uri, NULL);
		
		soup_uri_free (soup_uri);
	} else {
		job->session = soup_session_sync_new ();
	}

	return job;
}

static gpointer
buoh_comic_loader_job_finished (BuohComicLoaderJob *job)
{
	if (!job->canceled) {
		if (job->loader->priv->error)
			g_error_free (job->loader->priv->error);
		job->loader->priv->error = NULL;
		
		if (job->error)
			job->loader->priv->error = g_error_copy (job->error);
		
		g_signal_emit (job->loader, buoh_comic_loader_signals[FINISHED], 0);
	}
	
	g_object_unref (job);

	return FALSE;
}

static void
buoh_comic_loader_job_read_next (SoupMessage        *msg,
				 BuohComicLoaderJob *job)
{
	gboolean success;

	buoh_debug ("read chunk");
	
	success = SOUP_STATUS_IS_SUCCESSFUL (msg->status_code);
	
	if (job->canceled || !success) {
		if (!success) {
			if (job->error)
				g_error_free (job->error);
			job->error = g_error_new (BUOH_COMIC_LOADER_ERROR,
						  (gint) msg->status_code,
						  msg->reason_phrase);
		}

		soup_message_set_status (msg, SOUP_STATUS_CANCELLED);
		soup_session_cancel_message (job->session, msg);

		return;
	}

	if (job->callback) {
		job->callback (msg->response.body,
			       msg->response.length,
			       job->callback_data);
	}
}

static gpointer
buoh_comic_loader_job_run (BuohComicLoaderJob *job)
{
	SoupMessage *msg;
	
	buoh_debug ("comic_load_job");

	msg = soup_message_new (SOUP_METHOD_GET, job->uri);
	
	soup_message_set_flags (msg, SOUP_MESSAGE_OVERWRITE_CHUNKS);
	soup_message_add_handler (msg, SOUP_HANDLER_BODY_CHUNK,
				  (SoupMessageCallbackFn)buoh_comic_loader_job_read_next,
				  (gpointer) job);

	buoh_debug ("resolving . . .");

	soup_session_send_message (job->session, msg);
	g_object_unref (msg);

	if (job->canceled) {
		buoh_debug ("canceled . . .");
		g_object_unref (job);
	} else {
		buoh_debug ("finished . . .");
		
		g_idle_add ((GSourceFunc)buoh_comic_loader_job_finished,
			    (gpointer) job);
	}

	buoh_debug ("thread died");

	return NULL;
}

static void
buoh_comic_loader_clear (BuohComicLoader *loader)
{
	g_return_if_fail (BUOH_IS_COMIC_LOADER (loader));

	if (!loader->priv->job)
		return;

	loader->priv->job->canceled = TRUE;
	/* job unref is done by current thread */
	loader->priv->job = NULL;
}

void
buoh_comic_loader_load_comic (BuohComicLoader *loader,
			      BuohComic       *comic,
			      BuohComicLoaderLoadFunc callback,
			      gpointer         gdata)
{
	g_return_if_fail (BUOH_IS_COMIC_LOADER (loader));
	g_return_if_fail (BUOH_IS_COMIC (comic));
	
	buoh_comic_loader_clear (loader);
	
	loader->priv->job = buoh_comic_loader_job_new (buoh_comic_get_uri (comic),
						       callback, gdata);
	loader->priv->job->loader = loader;

	g_thread_create ((GThreadFunc) buoh_comic_loader_job_run,
			 loader->priv->job, FALSE, NULL);
}

void
buoh_comic_loader_get_error (BuohComicLoader *loader,
			     GError         **error)
{
	if (loader->priv->error)
		*error = g_error_copy (loader->priv->error);
}

