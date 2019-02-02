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
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "buoh-application.h"
#include "buoh-comic-cache.h"

struct _BuohComicCache {
        GObject     parent;

        gchar      *cache_dir;

        GHashTable *image_hash;
        GList      *image_list;
        GList      *image_disk;
        gulong      size;

        GdkPixbuf  *current_pixbuf;
        gchar      *current_uri;
};

#define CACHE_SIZE 1048576 /* 1MB */

static void buoh_comic_cache_init       (BuohComicCache *buoh_comic_cache);
static void buoh_comic_cache_class_init (BuohComicCacheClass *klass);
static void buoh_comic_cache_finalize   (GObject *object);

G_DEFINE_TYPE (BuohComicCache, buoh_comic_cache, G_TYPE_OBJECT)

static void
buoh_comic_cache_init (BuohComicCache *buoh_comic_cache)
{
        buoh_comic_cache->cache_dir =
                g_build_filename (g_get_user_cache_dir (),
                                  "buoh",
                                  NULL);

        buoh_comic_cache->image_hash =
                g_hash_table_new_full (g_str_hash,
                                       g_str_equal,
                                       g_free,
                                       (GDestroyNotify)buoh_comic_image_free);
}

static void
buoh_comic_cache_class_init (BuohComicCacheClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = buoh_comic_cache_finalize;
}

static void
buoh_comic_cache_free_disk (gchar *uri, gpointer gdata)
{
        if (g_unlink (uri) < 0) {
                buoh_debug ("Error deleting %s", uri);
        }
        g_free (uri);
}

static void
buoh_comic_cache_finalize (GObject *object)
{
        BuohComicCache *comic_cache = BUOH_COMIC_CACHE (object);

        buoh_debug ("comic-cache finalize");

        g_clear_pointer (&comic_cache->cache_dir, g_free);

        g_clear_pointer (&comic_cache->image_list, g_list_free);

        g_clear_pointer (&comic_cache->image_hash, g_hash_table_destroy);

        if (comic_cache->image_disk) {
                g_list_foreach (comic_cache->image_disk,
                                (GFunc) buoh_comic_cache_free_disk,
                                NULL);
                g_clear_pointer (&comic_cache->image_disk, g_list_free);
        }

        g_clear_object (&comic_cache->current_pixbuf);

        g_clear_pointer (&comic_cache->current_uri, g_free);

        if (G_OBJECT_CLASS (buoh_comic_cache_parent_class)->finalize) {
                (* G_OBJECT_CLASS (buoh_comic_cache_parent_class)->finalize) (object);
        }
}

BuohComicCache *
buoh_comic_cache_new (void)
{
        static BuohComicCache *cache = NULL;

        if (!cache) {
                cache = BUOH_COMIC_CACHE (g_object_new (BUOH_TYPE_COMIC_CACHE,
                                                        NULL));
                return cache;
        }

        return BUOH_COMIC_CACHE (g_object_ref (cache));
}

static gchar *
buoh_comic_cache_uri_to_filename (BuohComicCache *cache,
                                  const gchar    *uri)
{
        gchar *filename;
        gchar *path;

        g_assert (uri != NULL);

        filename = g_strdup (uri);
        g_strdelimit (filename, "/", '_');
        path = g_build_filename (cache->cache_dir,
                                 filename, NULL);
        g_free (filename);

        return path;
}

static void
buoh_comic_cache_to_disk (BuohComicCache *cache,
                          const gchar    *uri,
                          BuohComicImage *image)
{
        gchar  *path;
        GError *error = NULL;

        g_assert (uri != NULL);
        g_assert (image != NULL);

        path = buoh_comic_cache_uri_to_filename (cache, uri);
        buoh_debug ("CACHE: caching (disk) %s", path);

        if (g_list_find_custom (cache->image_disk,
                                (gconstpointer) path,
                                (GCompareFunc) g_ascii_strcasecmp)) {
                /* Already on disk */
                g_free (path);
                return;
        }

        if (!buoh_comic_image_save (image, path, &error)) {
                g_warning ("%s", error->message);
                g_error_free (error);
                g_free (path);
                return;
        }

        cache->image_disk = g_list_prepend (cache->image_disk,
                                                  g_strdup (path));
        g_free (path);
}

static void
buoh_comic_cache_set_current (BuohComicCache *cache,
                              const gchar    *uri,
                              BuohComicImage *image)
{
        GdkPixbufLoader *loader;
        GError          *error = NULL;

        if (cache->current_uri &&
            (g_ascii_strcasecmp (uri, cache->current_uri) == 0) &&
            GDK_IS_PIXBUF (cache->current_pixbuf)) {
                return;
        }

        if (cache->current_pixbuf) {
                g_object_unref (cache->current_pixbuf);
        }
        if (cache->current_uri) {
                g_free (cache->current_uri);
        }

        loader = gdk_pixbuf_loader_new ();
        gdk_pixbuf_loader_write (loader, image->data,
                                 image->size, &error);
        if (error) {
                g_warning ("%s", error->message);
                g_clear_error (&error);

                cache->current_pixbuf = NULL;
                cache->current_uri = NULL;
                gdk_pixbuf_loader_close (loader, NULL);
                g_object_unref (loader);

                return;
        }

        cache->current_pixbuf =
                gdk_pixbuf_loader_get_pixbuf (loader);
        g_object_ref (cache->current_pixbuf);
        gdk_pixbuf_loader_close (loader, &error);
        g_object_unref (loader);

        if (error) {
                g_warning ("%s", error->message);
                g_clear_error (&error);

                cache->current_pixbuf = NULL;
                cache->current_uri = NULL;

                return;
        }

        cache->current_uri = g_strdup (uri);
}

void
buoh_comic_cache_set_image (BuohComicCache *cache,
                            const gchar    *uri,
                            BuohComicImage *image)
{
        gchar          *key_uri;
        BuohComicImage *img;

        g_return_if_fail (BUOH_IS_COMIC_CACHE (cache));
        g_return_if_fail (uri != NULL);
        g_return_if_fail (image != NULL);

        buoh_debug ("CACHE: uri %s", uri);

        if ((img = g_hash_table_lookup (cache->image_hash, uri))) {
                buoh_comic_cache_set_current (cache, uri, img);
                return;
        }

        buoh_debug ("CACHE: image size %d", image->size);

        if (image->size > CACHE_SIZE) {
                buoh_comic_cache_to_disk (cache, uri, image);
                buoh_comic_cache_set_current (cache, uri, image);
                return;
        }

        while (CACHE_SIZE - cache->size < image->size) {
                GList *item;
                gchar *item_uri;

                if (!cache->image_list) {
                        break;
                }

                item = g_list_last (cache->image_list);
                item_uri = (gchar *) item->data;

                img = (BuohComicImage *) g_hash_table_lookup (cache->image_hash,
                                                              item_uri);
                buoh_comic_cache_to_disk (cache, item_uri, img);

                buoh_debug ("CACHE: removing %s", item_uri);
                cache->image_list = g_list_delete_link (cache->image_list,
                                                              item);
                g_hash_table_remove (cache->image_hash, item_uri);

                cache->size -= img->size;
                buoh_debug ("CACHE: cache size %d\n", cache->size);
        }

        key_uri = g_strdup (uri);

        buoh_debug ("CACHE: caching (memory) %s", key_uri);
        cache->image_list = g_list_prepend (cache->image_list, key_uri);
        g_hash_table_insert (cache->image_hash, key_uri, image);
        buoh_comic_cache_set_current (cache, uri, image);

        cache->size += image->size;
        buoh_debug ("CACHE: cache size %d\n", cache->size);
}

BuohComicImage *
buoh_comic_cache_get_image (BuohComicCache *cache,
                            const gchar    *uri)
{
        BuohComicImage *image;
        gchar          *path;
        GList          *item;

        g_return_val_if_fail (BUOH_IS_COMIC_CACHE (cache), NULL);
        g_return_val_if_fail (uri != NULL, NULL);

        image = (BuohComicImage *) g_hash_table_lookup (cache->image_hash, uri);
        if (image) {
                /* keep items ordered by access time */
                item = g_list_find_custom (cache->image_list,
                                           (gconstpointer) uri,
                                           (GCompareFunc) g_ascii_strcasecmp);
                if (item != cache->image_list) {
                        cache->image_list = g_list_remove_link (cache->image_list,
                                                                      item);
                        cache->image_list = g_list_prepend (cache->image_list,
                                                                  item->data);
                        g_list_free (item);
                }

                buoh_debug ("CACHE: return image from memory");
                return image;
        }

        path = buoh_comic_cache_uri_to_filename (cache, uri);
        image = g_new0 (BuohComicImage, 1);
        if (g_file_get_contents (path, (gchar **)&image->data, &image->size, NULL)) {
                buoh_comic_cache_set_image (cache, uri, image);
                buoh_debug ("CACHE: return image from disk");
                g_free (path);

                return image;
        }
        g_free (path);
        g_free (image);

        buoh_debug ("CACHE: image is not cached");

        return NULL;
}

void
buoh_comic_cache_set_pixbuf (BuohComicCache *cache,
                             const gchar    *uri,
                             GdkPixbuf      *pixbuf)
{
        g_return_if_fail (BUOH_IS_COMIC_CACHE (cache));
        g_return_if_fail (uri != NULL);
        g_return_if_fail (GDK_IS_PIXBUF (pixbuf));

        if (cache->current_uri) {
                g_free (cache->current_uri);
        }
        if (cache->current_pixbuf) {
                g_object_unref (cache->current_pixbuf);
        }

        cache->current_uri = g_strdup (uri);
        cache->current_pixbuf = g_object_ref (pixbuf);
}

GdkPixbuf *
buoh_comic_cache_get_pixbuf (BuohComicCache *cache,
                             const gchar    *uri)
{
        BuohComicImage *image;

        g_return_val_if_fail (BUOH_IS_COMIC_CACHE (cache), NULL);
        g_return_val_if_fail (uri != NULL, NULL);

        if (cache->current_uri &&
            g_ascii_strcasecmp (uri, cache->current_uri) == 0) {
                buoh_debug ("is the current pixbuf");
                return cache->current_pixbuf;
        }

        image = buoh_comic_cache_get_image (cache, uri);

        if (image) {
                buoh_comic_cache_set_current (cache, uri, image);
                return cache->current_pixbuf;
        }

        return NULL;
}
