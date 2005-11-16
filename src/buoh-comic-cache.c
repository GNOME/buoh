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

#include <glib.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "buoh.h"
#include "buoh-comic-cache.h"

struct _BuohComicCachePrivate {
	gchar      *cache_dir;
	
	GHashTable *pixbuf_hash;
	GList      *pixbuf_list;
	GList      *pixbuf_disk;
	gulong      size;
};

#define BUOH_COMIC_CACHE_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_COMIC_CACHE, BuohComicCachePrivate))

#define CACHE_SIZE 5242880 /* 5MB */

static GObjectClass *parent_class = NULL;

static void buoh_comic_cache_init       (BuohComicCache *buoh_comic_cache);
static void buoh_comic_cache_class_init (BuohComicCacheClass *klass);
static void buoh_comic_cache_finalize   (GObject *object);

GType
buoh_comic_cache_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (BuohComicCacheClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) buoh_comic_cache_class_init,
			NULL,
			NULL,
			sizeof (BuohComicCache),
			0,
			(GInstanceInitFunc) buoh_comic_cache_init
		};

		type = g_type_register_static (G_TYPE_OBJECT, "BuohComicCache",
					       &info, 0);
	}

	return type;
}

static void
buoh_comic_cache_init (BuohComicCache *buoh_comic_cache)
{
	buoh_comic_cache->priv = BUOH_COMIC_CACHE_GET_PRIVATE (buoh_comic_cache);

	buoh_comic_cache->priv->cache_dir =
		g_build_filename (buoh_get_datadir (BUOH), "cache", NULL);
	
	buoh_comic_cache->priv->pixbuf_list = NULL;
	buoh_comic_cache->priv->pixbuf_hash =
		g_hash_table_new_full (g_str_hash,
				       g_str_equal,
				       g_free,
				       g_object_unref);
	buoh_comic_cache->priv->pixbuf_disk = NULL;
	
	buoh_comic_cache->priv->size = 0;
}

static void
buoh_comic_cache_class_init (BuohComicCacheClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (BuohComicCachePrivate));

	object_class->finalize = buoh_comic_cache_finalize;
}

static void
buoh_comic_cache_free_disk (gchar *uri, gpointer gdata)
{
	if (g_unlink (uri) < 0)
		buoh_debug ("Error deleting %s", uri);
	g_free (uri);
}

static void
buoh_comic_cache_finalize (GObject *object)
{
	BuohComicCache *comic_cache = BUOH_COMIC_CACHE (object);
	
	buoh_debug ("comic-cache finalize");

	if (comic_cache->priv->cache_dir) {
		g_free (comic_cache->priv->cache_dir);
		comic_cache->priv->cache_dir = NULL;
	}
	
	if (comic_cache->priv->pixbuf_list) {
		g_list_free (comic_cache->priv->pixbuf_list);
		comic_cache->priv->pixbuf_list = NULL;
	}

	if (comic_cache->priv->pixbuf_hash) {
		g_hash_table_destroy (comic_cache->priv->pixbuf_hash);
		comic_cache->priv->pixbuf_hash = NULL;
	}

	if (comic_cache->priv->pixbuf_disk) {
		g_list_foreach (comic_cache->priv->pixbuf_disk,
				(GFunc) buoh_comic_cache_free_disk,
				NULL);
		g_list_free (comic_cache->priv->pixbuf_disk);
		comic_cache->priv->pixbuf_disk = NULL;
	}

	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
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
	path = g_build_filename (cache->priv->cache_dir,
				 filename, NULL);
	g_free (filename);

	return path;
}

static void
buoh_comic_cache_to_disk (BuohComicCache *cache,
			  const gchar    *uri,
			  GdkPixbuf      *pixbuf)
{
	gchar  *path;
	GError *error = NULL;

	g_assert (uri != NULL);
	g_assert (GDK_IS_PIXBUF (pixbuf));

	path = buoh_comic_cache_uri_to_filename (cache, uri);
	buoh_debug ("CACHE: caching (disk) %s", path);

	if (g_file_test (path, G_FILE_TEST_EXISTS)) {
		g_free (path);
		return;
	}
	
	gdk_pixbuf_save (pixbuf, path, "png", &error, NULL);
	
	if (error) {
		g_warning (error->message);
		g_error_free (error);
		g_free (path);

		return;
	}

	if (!g_list_find_custom (cache->priv->pixbuf_disk,
				 (gconstpointer) path,
				 (GCompareFunc) g_ascii_strcasecmp)) {
		cache->priv->pixbuf_disk = g_list_prepend (cache->priv->pixbuf_disk,
							   g_strdup (path));
	}
	
	g_free (path);
}

static gulong
pixbuf_get_size (const GdkPixbuf *pixbuf)
{
	g_assert (GDK_IS_PIXBUF (pixbuf));

	gint height = gdk_pixbuf_get_height (pixbuf);
	gint width = gdk_pixbuf_get_width (pixbuf);
	gint rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	gint n_channels = gdk_pixbuf_get_n_channels (pixbuf);

	return ((height - 1) * rowstride + width * n_channels);
}

void
buoh_comic_cache_set_pixbuf (BuohComicCache *cache,
			     const gchar    *uri,
			     GdkPixbuf      *pixbuf)
{
	gulong  size;
	gchar  *key_uri;

	g_return_if_fail (BUOH_IS_COMIC_CACHE (cache));
	g_return_if_fail (uri != NULL);
	g_return_if_fail (GDK_IS_PIXBUF (pixbuf));

	buoh_debug ("CACHE: uri %s", uri);
	
	if (g_hash_table_lookup (cache->priv->pixbuf_hash, uri))
		return;

	size = pixbuf_get_size (pixbuf);
	buoh_debug ("CACHE: pixbuf size %d", size);

	if (size > CACHE_SIZE) {
		buoh_comic_cache_to_disk (cache, uri, pixbuf);
		return;
	}
	
	while (CACHE_SIZE - cache->priv->size < size) {
		GList     *item;
		gchar     *item_uri;
		gulong     item_size;
		GdkPixbuf *pix;

		if (!cache->priv->pixbuf_list)
			break;
		
		item = g_list_last (cache->priv->pixbuf_list);
		item_uri = (gchar *) item->data;
		
		pix = GDK_PIXBUF (g_hash_table_lookup (cache->priv->pixbuf_hash,
						       item_uri));
		item_size = pixbuf_get_size (pix);

		buoh_comic_cache_to_disk (cache, item_uri, pix);

		buoh_debug ("CACHE: removing %s", item_uri);
		cache->priv->pixbuf_list = g_list_delete_link (cache->priv->pixbuf_list,
							       item);
		g_hash_table_remove (cache->priv->pixbuf_hash, item_uri);

		cache->priv->size -= item_size;
		buoh_debug ("CACHE: cache size %d\n", cache->priv->size);
	}

	key_uri = g_strdup (uri);

	buoh_debug ("CACHE: caching (memory) %s", key_uri);
	cache->priv->pixbuf_list = g_list_prepend (cache->priv->pixbuf_list, key_uri);
	g_hash_table_insert (cache->priv->pixbuf_hash, key_uri, g_object_ref (pixbuf));
	
	cache->priv->size += size;
	buoh_debug ("CACHE: cache size %d\n", cache->priv->size);
}

GdkPixbuf *
buoh_comic_cache_get_pixbuf (BuohComicCache *cache,
			     const gchar    *uri)
{
	GdkPixbuf *pixbuf = NULL;
	gchar     *path;
	GList     *item;
	
	g_return_val_if_fail (BUOH_IS_COMIC_CACHE (cache), NULL);
	g_return_val_if_fail (uri != NULL, NULL);

	pixbuf = GDK_PIXBUF (g_hash_table_lookup (cache->priv->pixbuf_hash, uri));
	if (pixbuf) {
		/* keep items ordered by access time */
		item = g_list_find_custom (cache->priv->pixbuf_list,
					   (gconstpointer) uri,
					   (GCompareFunc) g_ascii_strcasecmp);
		cache->priv->pixbuf_list = g_list_remove_link (cache->priv->pixbuf_list,
							       item);
		cache->priv->pixbuf_list = g_list_prepend (cache->priv->pixbuf_list, item->data);
		g_list_free (item);
		
		buoh_debug ("CACHE: return pixbuf from memory");
		return pixbuf;
	}

	path = buoh_comic_cache_uri_to_filename (cache, uri);
	pixbuf = gdk_pixbuf_new_from_file (path, NULL);

	if (pixbuf) {
		buoh_comic_cache_set_pixbuf (cache, uri, pixbuf);
		g_object_unref (pixbuf);
		buoh_debug ("CACHE: return pixbuf from disk");
		return pixbuf;
	}

	buoh_debug ("CACHE: pixbuf is not cached");
	
	return NULL;
}
