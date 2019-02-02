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
 *  Authors: Carlos García Campos <carlosgc@gnome.org>
 *           Pablo Castellano <pablog@src.gnome.org>
 */

#include <libsoup/soup.h>

#include "buoh-application.h"
#include "buoh-comic-loader.h"

/* BuohComicLoaderJob */
#define BUOH_TYPE_COMIC_LOADER_JOB          (buoh_comic_loader_job_get_type())
G_DECLARE_FINAL_TYPE (BuohComicLoaderJob, buoh_comic_loader_job, BUOH, COMIC_LOADER_JOB, GObject)

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

static void  buoh_comic_loader_job_init         (BuohComicLoaderJob      *job);
static void  buoh_comic_loader_job_class_init   (BuohComicLoaderJobClass *klass);
static void  buoh_comic_loader_job_finalize     (GObject                 *object);

G_DEFINE_TYPE (BuohComicLoaderJob, buoh_comic_loader_job, G_TYPE_OBJECT)

typedef struct {
        BuohComicLoaderJob *job;

        GError             *error;
} BuohComicLoaderPrivate;


enum {
        FINISHED,
        LAST_SIGNAL
};

static guint buoh_comic_loader_signals[LAST_SIGNAL] = { 0 };

static void     buoh_comic_loader_init          (BuohComicLoader      *loader);
static void     buoh_comic_loader_class_init    (BuohComicLoaderClass *klass);
static void     buoh_comic_loader_finalize      (GObject              *object);

static void     buoh_comic_loader_clear         (BuohComicLoader      *loader);

G_DEFINE_TYPE_WITH_PRIVATE (BuohComicLoader, buoh_comic_loader, G_TYPE_OBJECT)

GQuark
buoh_comic_loader_error_quark (void)
{
        static GQuark quark = 0;

        if (G_UNLIKELY (quark == 0)) {
                quark = g_quark_from_static_string ("buoh-comic-loader-error-quark");
        }

        return quark;
}

static void
buoh_comic_loader_init (BuohComicLoader *loader)
{
}

static void
buoh_comic_loader_class_init (BuohComicLoaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

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
        BuohComicLoaderPrivate *priv = buoh_comic_loader_get_instance_private (loader);

        buoh_debug ("comic-loader finalize");

        buoh_comic_loader_clear (loader);

        if (priv->error) {
                g_error_free (priv->error);
                priv->error = NULL;
        }

        if (G_OBJECT_CLASS (buoh_comic_loader_parent_class)->finalize) {
                (* G_OBJECT_CLASS (buoh_comic_loader_parent_class)->finalize) (object);
        }
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

        if (G_OBJECT_CLASS (buoh_comic_loader_job_parent_class)->finalize) {
                (* G_OBJECT_CLASS (buoh_comic_loader_job_parent_class)->finalize) (object);
        }
}

static BuohComicLoaderJob *
buoh_comic_loader_job_new (const gchar             *uri,
                           BuohComicLoaderLoadFunc  callback,
                           gpointer                 gdata)
{
        BuohComicLoaderJob *job;

        job = g_object_new (BUOH_TYPE_COMIC_LOADER_JOB, NULL);

        job->uri = g_strdup (uri);
        job->callback = callback;
        job->callback_data = gdata;

        job->session = soup_session_new ();

        return job;
}

static gpointer
buoh_comic_loader_job_finished (BuohComicLoaderJob *job)
{
        if (!job->canceled) {
                BuohComicLoaderPrivate *priv = buoh_comic_loader_get_instance_private (job->loader);
                if (priv->error) {
                        g_error_free (priv->error);
                }
                priv->error = NULL;

                if (job->error) {
                        priv->error = g_error_copy (job->error);
                }

                g_signal_emit (job->loader, buoh_comic_loader_signals[FINISHED], 0);
        }

        g_object_unref (job);

        return FALSE;
}

static void
buoh_comic_loader_job_read_next (SoupMessage        *msg,
                                 SoupBuffer         *chunk,
                                 BuohComicLoaderJob *job)
{
        gboolean success;

        buoh_debug ("read chunk");

        success = SOUP_STATUS_IS_SUCCESSFUL (msg->status_code);

        if (job->canceled || !success) {
                if (!success) {
                        if (job->error) {
                                g_error_free (job->error);
                        }
                        job->error = g_error_new (BUOH_COMIC_LOADER_ERROR,
                                                  (gint) msg->status_code,
                                                  "%s", msg->reason_phrase);
                }

                soup_message_set_status (msg, SOUP_STATUS_CANCELLED);
                soup_session_cancel_message (job->session, msg, SOUP_STATUS_CANCELLED);

                return;
        }

        if (job->callback) {
                job->callback (chunk->data,
                               chunk->length,
                               job->callback_data);
        }
}

static gpointer
buoh_comic_loader_job_run (BuohComicLoaderJob *job)
{
        SoupMessage *msg;

        buoh_debug ("comic_load_job");

        msg = soup_message_new (SOUP_METHOD_GET, job->uri);

        soup_message_body_set_accumulate (msg->response_body, FALSE);
        g_signal_connect (msg, "got-chunk",
                          G_CALLBACK (buoh_comic_loader_job_read_next),
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
        BuohComicLoaderPrivate *priv = buoh_comic_loader_get_instance_private (loader);

        if (!priv->job) {
                return;
        }

        priv->job->canceled = TRUE;
        /* job unref is done by current thread */
        priv->job = NULL;
}

void
buoh_comic_loader_load_comic (BuohComicLoader *loader,
                              BuohComic       *comic,
                              BuohComicLoaderLoadFunc callback,
                              gpointer         gdata)
{
        g_return_if_fail (BUOH_IS_COMIC_LOADER (loader));
        g_return_if_fail (BUOH_IS_COMIC (comic));

        BuohComicLoaderPrivate *priv = buoh_comic_loader_get_instance_private (loader);

        buoh_comic_loader_clear (loader);

        priv->job = buoh_comic_loader_job_new (buoh_comic_get_uri (comic),
                                                       callback, gdata);
        priv->job->loader = loader;

        g_thread_new ("comic_loader",
                      (GThreadFunc) buoh_comic_loader_job_run,
                      priv->job);
}

void
buoh_comic_loader_get_error (BuohComicLoader *loader,
                             GError         **error)
{
        g_return_if_fail (BUOH_IS_COMIC_LOADER (loader));
        BuohComicLoaderPrivate *priv = buoh_comic_loader_get_instance_private (loader);

        if (priv->error) {
                *error = g_error_copy (priv->error);
        }
}

void
buoh_comic_loader_cancel (BuohComicLoader *loader)
{
        g_return_if_fail (BUOH_IS_COMIC_LOADER (loader));

        buoh_comic_loader_clear (loader);
}
