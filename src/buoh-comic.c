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
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "buoh-application.h"
#include "buoh-comic.h"
#include "buoh-comic-cache.h"

enum {
        PROP_0,
        PROP_ID,
        PROP_URI,
        PROP_PIXBUF,
        PROP_IMAGE,
        PROP_DATE
};

struct _BuohComic {
        GObject         parent;

        gchar          *id;
        gchar          *uri;
        GDate          *date;

        BuohComicCache *cache;
};

static void buoh_comic_init         (BuohComic      *buoh_comic);
static void buoh_comic_class_init   (BuohComicClass *klass);
static void buoh_comic_finalize     (GObject        *object);
static void buoh_comic_get_property (GObject        *object,
                                     guint           prop_id,
                                     GValue         *value,
                                     GParamSpec     *pspec);
static void buoh_comic_set_property (GObject        *object,
                                     guint           prop_id,
                                     const GValue   *value,
                                     GParamSpec     *pspec);

G_DEFINE_TYPE (BuohComic, buoh_comic, G_TYPE_OBJECT)

static void
buoh_comic_init (BuohComic *buoh_comic)
{
        buoh_comic->cache = buoh_comic_cache_new ();
}

static void
buoh_comic_class_init (BuohComicClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->set_property = buoh_comic_set_property;
        object_class->get_property = buoh_comic_get_property;

        g_object_class_install_property (object_class,
                                         PROP_ID,
                                         g_param_spec_string ("id",
                                                              "Id",
                                                              "Identificator of the comic",
                                                              NULL,
                                                              G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_URI,
                                         g_param_spec_string ("uri",
                                                              "URI",
                                                              "URI of the comic",
                                                              NULL,
                                                              G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_PIXBUF,
                                         g_param_spec_pointer ("pixbuf",
                                                               "Pixbuf",
                                                               "Pixbuf of the comic",
                                                               G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_IMAGE,
                                         g_param_spec_pointer ("image",
                                                               "Image",
                                                               "Compressed image of the comic",
                                                               G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_DATE,
                                         g_param_spec_pointer ("date",
                                                               "Date",
                                                               "Date release of the comic",
                                                               G_PARAM_READWRITE));

        object_class->finalize = buoh_comic_finalize;
}

static void
buoh_comic_finalize (GObject *object)
{
        BuohComic *comic = BUOH_COMIC (object);

        buoh_debug ("buoh-comic-finalize");

        g_clear_pointer (&comic->id, g_free);
        g_clear_pointer (&comic->uri, g_free);
        g_clear_pointer (&comic->date, g_date_free);
        g_clear_object (&comic->cache);

        if (G_OBJECT_CLASS (buoh_comic_parent_class)->finalize) {
                (* G_OBJECT_CLASS (buoh_comic_parent_class)->finalize) (object);
        }
}

BuohComic *
buoh_comic_new (void)
{
        BuohComic *comic;

        comic = BUOH_COMIC (g_object_new (BUOH_TYPE_COMIC, NULL));

        return comic;
}

BuohComic *
buoh_comic_new_with_info (const gchar *id, const gchar *uri,
                          const GDate *date)
{
        BuohComic *comic;

        g_return_val_if_fail (id != NULL && uri != NULL, NULL);
        g_return_val_if_fail (date != NULL, NULL);

        comic = BUOH_COMIC (g_object_new (BUOH_TYPE_COMIC,
                                          "id", id,
                                          "uri", uri,
                                          "date", date, NULL));

        return comic;
}


static void
buoh_comic_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
        BuohComic *comic = BUOH_COMIC (object);
        GDate     *date;

        switch (prop_id) {
        case PROP_ID:
                g_free (comic->id);
                comic->id = g_value_dup_string (value);

                break;
        case PROP_URI:
                g_free (comic->uri);
                comic->uri = g_value_dup_string (value);

                break;
        case PROP_PIXBUF: {
                GdkPixbuf *pixbuf;

                pixbuf = GDK_PIXBUF (g_value_get_pointer (value));
                buoh_comic_cache_set_pixbuf (comic->cache,
                                             comic->uri, pixbuf);
        }
                break;
        case PROP_IMAGE: {
                BuohComicImage *image;

                image = (BuohComicImage *) g_value_get_pointer (value);
                buoh_comic_cache_set_image (comic->cache,
                                            comic->uri, image);
        }

                break;
        case PROP_DATE:
                if (comic->date) {
                        g_date_free (comic->date);
                }
                date = g_value_get_pointer (value);

                comic->date = g_date_new_dmy (g_date_get_day (date),
                                                    g_date_get_month (date),
                                                    g_date_get_year (date));
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static void
buoh_comic_get_property (GObject      *object,
                         guint         prop_id,
                         GValue       *value,
                         GParamSpec   *pspec)
{
        BuohComic *comic = BUOH_COMIC (object);

        switch (prop_id) {
        case PROP_ID:
                g_value_set_string (value, comic->id);

                break;
        case PROP_URI:
                g_value_set_string (value, comic->uri);

                break;
        case PROP_PIXBUF: {
                GdkPixbuf *pixbuf;

                pixbuf = buoh_comic_cache_get_pixbuf (comic->cache,
                                                      comic->uri);
                g_value_set_pointer (value, pixbuf);
        }
                break;
        case PROP_IMAGE: {
                BuohComicImage *image;

                image = buoh_comic_cache_get_image (comic->cache,
                                                    comic->uri);
                g_value_set_pointer (value, image);
        }
                break;
        case PROP_DATE:
                g_value_set_pointer (value, comic->date);

                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

void
buoh_comic_set_id (BuohComic *comic, const gchar *id)
{
        g_return_if_fail (BUOH_IS_COMIC (comic));
        g_return_if_fail (id != NULL);

        g_object_set (G_OBJECT (comic), "id", id, NULL);
}

void
buoh_comic_set_pixbuf (BuohComic *comic, GdkPixbuf *pixbuf)
{
        g_return_if_fail (BUOH_IS_COMIC (comic));
        g_return_if_fail (GDK_IS_PIXBUF (pixbuf));

        g_object_set (G_OBJECT (comic), "pixbuf", pixbuf, NULL);
}

void
buoh_comic_set_image (BuohComic *comic, BuohComicImage *image)
{
        g_return_if_fail (BUOH_IS_COMIC (comic));
        g_return_if_fail (image != NULL);

        g_object_set (G_OBJECT (comic), "image", image, NULL);
}

void
buoh_comic_set_date (BuohComic *comic, GDate *date)
{
        g_return_if_fail (BUOH_IS_COMIC (comic));
        g_return_if_fail (date != NULL);

        g_object_set (G_OBJECT (comic), "date", date, NULL);
}

void
buoh_comic_set_pixbuf_from_file (BuohComic *comic, const gchar *filename)
{
        GdkPixbuf *pixbuf = NULL;

        g_return_if_fail (BUOH_IS_COMIC (comic));
        g_return_if_fail (filename != NULL);

        pixbuf = gdk_pixbuf_new_from_file (filename, NULL);

        buoh_comic_set_pixbuf (comic, pixbuf);
}

const gchar *
buoh_comic_get_id (BuohComic *comic)
{
        g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

        return comic->id;
}

const gchar *
buoh_comic_get_uri (BuohComic *comic)
{
        g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

        return comic->uri;
}

GdkPixbuf *
buoh_comic_get_pixbuf (BuohComic *comic)
{
        GdkPixbuf *pixbuf = NULL;

        g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

        g_object_get (G_OBJECT (comic), "pixbuf", &pixbuf, NULL);

        return pixbuf;
}

BuohComicImage *
buoh_comic_get_image (BuohComic *comic)
{
        BuohComicImage *image = NULL;

        g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

        g_object_get (G_OBJECT (comic), "image", &image, NULL);

        return image;
}

GDate *
buoh_comic_get_date (BuohComic *comic)
{
        GDate *date = NULL;

        g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

        g_object_get (G_OBJECT (comic), "date", &date, NULL);

        return date;
}

GdkPixbuf *
buoh_comic_get_thumbnail (BuohComic *comic)
{
        GdkPixbuf *pixbuf = NULL;
        GdkPixbuf *thumbnail = NULL;
        gint       c_width, c_height;
        gint       d_width, d_height;

        g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

        g_object_get (G_OBJECT (comic), "pixbuf", &pixbuf, NULL);

        if (pixbuf) {
                c_width = gdk_pixbuf_get_width (pixbuf);
                c_height = gdk_pixbuf_get_height (pixbuf);

                if (c_width > c_height) {
                        d_width = 96;
                        d_height = c_height - (((c_width - d_width) * c_height) / c_width);
                } else {
                        d_height = 96;
                        d_width = c_width - (((c_height - d_height) * c_width) / c_height);
                }

                thumbnail = gdk_pixbuf_scale_simple (pixbuf, d_width, d_height, GDK_INTERP_BILINEAR);

                return thumbnail;
        }

        return NULL;
}

gchar *
buoh_comic_get_filename (BuohComic *comic)
{
        gchar *filename;

        g_return_val_if_fail (BUOH_IS_COMIC (comic), NULL);

        filename = g_path_get_basename (comic->uri);

        return filename;
}

gboolean
buoh_comic_image_save (BuohComicImage *image,
                       const gchar    *path,
                       GError        **error)
{
        g_return_val_if_fail (image != NULL && image->data != NULL, FALSE);
        g_return_val_if_fail (path != NULL, FALSE);

        gint fd;

        if ((fd = open (path, O_CREAT | O_WRONLY, 0644)) < 0) {
                g_set_error (error, G_FILE_ERROR,
                             g_file_error_from_errno (errno),
                             _("Cannot create file “%s”: %s"),
                             path, g_strerror (errno));
                return FALSE;
        }

        if (write (fd, image->data, image->size) < 0) {
                g_set_error (error, G_FILE_ERROR,
                             g_file_error_from_errno (errno),
                             _("Error writing to file “%s”: %s"),
                             path, g_strerror (errno));
                close (fd);
                return FALSE;
        }

        if (close (fd) < 0) {
                g_set_error (error, G_FILE_ERROR,
                             g_file_error_from_errno (errno),
                             _("Error writing to file “%s”: %s"),
                             path, g_strerror (errno));
                return FALSE;
        }

        return TRUE;
}

void
buoh_comic_image_free (BuohComicImage *image)
{
        if (image) {
                g_free (image->data);
                g_free (image);
        }
}
