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
 *  Authors: Esteban Sanchez Munoz (steve-o) <esteban@steve-o.org>
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "buoh.h"
#include "buoh-comic-manager.h"
#include "buoh-comic-manager-date.h"

enum {
        PROP_0,
        PROP_ID,
        PROP_TITLE,
        PROP_AUTHOR,
        PROP_LANGUAGE,
        PROP_GENERIC_URI,
        PROP_LIST,
        PROP_CURRENT
};

typedef struct {
        gchar *author;
        gchar *title;
        gchar *language;
        gchar *id;
        gchar *generic_uri;
        GList *comic_list;
        GList *current;
} BuohComicManagerPrivate;

static void buoh_comic_manager_init         (BuohComicManager      *comic_manager);
static void buoh_comic_manager_class_init   (BuohComicManagerClass *klass);
static void buoh_comic_manager_finalize     (GObject               *object);
static void buoh_comic_manager_get_property (GObject               *object,
                                             guint                  prop_id,
                                             GValue                *value,
                                             GParamSpec            *pspec);
static void buoh_comic_manager_set_property (GObject               *object,
                                             guint                  prop_id,
                                             const GValue          *value,
                                             GParamSpec            *pspec);

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (BuohComicManager, buoh_comic_manager, G_TYPE_OBJECT)

static void
buoh_comic_manager_init (BuohComicManager *comic_manager)
{
}

static void
buoh_comic_manager_class_init (BuohComicManagerClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->set_property = buoh_comic_manager_set_property;
        object_class->get_property = buoh_comic_manager_get_property;

        g_object_class_install_property (object_class,
                                         PROP_ID,
                                         g_param_spec_string ("id",
                                                              "Id",
                                                              "Identificator of the comic",
                                                              NULL,
                                                              G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_TITLE,
                                         g_param_spec_string ("title",
                                                              "Title",
                                                              "Title of the comic",
                                                              NULL,
                                                              G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_AUTHOR,
                                         g_param_spec_string ("author",
                                                              "Author",
                                                              "Name of the author",
                                                              NULL,
                                                              G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_LANGUAGE,
                                         g_param_spec_string ("language",
                                                              "Language",
                                                              "Language of the comic",
                                                              NULL,
                                                              G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

        g_object_class_install_property (object_class,
                                         PROP_GENERIC_URI,
                                         g_param_spec_string ("generic_uri",
                                                              "Generic URI",
                                                              "URI with date format",
                                                              NULL,
                                                              G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

        g_object_class_install_property (object_class,
                                         PROP_LIST,
                                         g_param_spec_pointer ("list",
                                                               "Comic list",
                                                               "List with the comics",
                                                               G_PARAM_READWRITE));

        g_object_class_install_property (object_class,
                                         PROP_CURRENT,
                                         g_param_spec_pointer ("current",
                                                               "Current comic",
                                                               "Current comic in the list",
                                                               G_PARAM_READWRITE));

        object_class->finalize = buoh_comic_manager_finalize;
}

static void
buoh_comic_manager_finalize (GObject *object)
{
        BuohComicManager *comic_manager = BUOH_COMIC_MANAGER (object);
        BuohComicManagerPrivate *priv = buoh_comic_manager_get_instance_private (comic_manager);

        g_clear_pointer (&priv->author, g_free);
        g_clear_pointer (&priv->title, g_free);
        g_clear_pointer (&priv->language, g_free);
        g_clear_pointer (&priv->id, g_free);
        g_clear_pointer (&priv->generic_uri, g_free);

        if (priv->comic_list) {
                g_list_foreach (priv->comic_list,
                                (GFunc) g_object_unref, NULL);
                g_clear_pointer (&priv->comic_list, g_list_free);
        }

        if (G_OBJECT_CLASS (buoh_comic_manager_parent_class)->finalize) {
                (* G_OBJECT_CLASS (buoh_comic_manager_parent_class)->finalize) (object);
        }
}

BuohComicManager *
buoh_comic_manager_new (const gchar *type,
                        const gchar *id,
                        const gchar *title,
                        const gchar *author,
                        const gchar *language,
                        const gchar *generic_uri)
{
        g_return_val_if_fail (type != NULL, NULL);

        if (g_ascii_strcasecmp (type, "date") == 0) {
                return BUOH_COMIC_MANAGER (buoh_comic_manager_date_new (id,
                                                                        title,
                                                                        author,
                                                                        language,
                                                                        generic_uri));
        } else {
                g_warning ("Invalid type %s for BuohComicManager (id: %s)\n",
                           type, id);
                return NULL;
        }
}


static void
buoh_comic_manager_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
        BuohComicManager *comic_manager = BUOH_COMIC_MANAGER (object);
        BuohComicManagerPrivate *priv = buoh_comic_manager_get_instance_private (comic_manager);

        switch (prop_id) {
        case PROP_TITLE:
                g_free (priv->title);
                priv->title = g_value_dup_string (value);

                break;
        case PROP_AUTHOR:
                g_free (priv->author);
                priv->author = g_value_dup_string (value);

                break;
        case PROP_LANGUAGE:
                g_free (priv->language);
                priv->language = g_value_dup_string (value);

                break;
        case PROP_ID:
                g_free (priv->id);
                priv->id = g_value_dup_string (value);

                break;
        case PROP_GENERIC_URI:
                g_free (priv->generic_uri);
                priv->generic_uri = g_value_dup_string (value);

                break;
        case PROP_LIST:
                priv->comic_list = g_value_get_pointer (value);

                break;
        case PROP_CURRENT:
                priv->current = g_value_get_pointer (value);

                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static void
buoh_comic_manager_get_property (GObject      *object,
                                 guint         prop_id,
                                 GValue       *value,
                                 GParamSpec   *pspec)
{
        BuohComicManager *comic_manager = BUOH_COMIC_MANAGER (object);
        BuohComicManagerPrivate *priv = buoh_comic_manager_get_instance_private (comic_manager);

        switch (prop_id) {
        case PROP_TITLE:
                g_value_set_string (value, priv->title);

                break;
        case PROP_AUTHOR:
                g_value_set_string (value, priv->author);

                break;
        case PROP_LANGUAGE:
                g_value_set_string (value, priv->language);

                break;
        case PROP_ID:
                g_value_set_string (value, priv->id);

                break;
        case PROP_GENERIC_URI:
                g_value_set_string (value, priv->generic_uri);

                break;
        case PROP_LIST:
                g_value_set_pointer (value, priv->comic_list);

                break;
        case PROP_CURRENT:
                g_value_set_pointer (value, priv->current);

                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

BuohComic *
buoh_comic_manager_get_last (BuohComicManager *comic_manager)
{
        g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);

        if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_last) {
                return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_last) (comic_manager);
        } else {
                return NULL;
        }
}

BuohComic *
buoh_comic_manager_get_first (BuohComicManager *comic_manager)
{
        g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);

        if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_first) {
                return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_first) (comic_manager);
        } else {
                return NULL;
        }
}

BuohComic *
buoh_comic_manager_get_next (BuohComicManager *comic_manager)
{
        g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);

        if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_next) {
                return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_next) (comic_manager);
        } else {
                return NULL;
        }
}

BuohComic *
buoh_comic_manager_get_previous (BuohComicManager *comic_manager)
{
        g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);

        if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_previous) {
                return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_previous) (comic_manager);
        } else {
                return NULL;
        }
}

BuohComic *
buoh_comic_manager_get_current (BuohComicManager *comic_manager)
{
        g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);
        BuohComicManagerPrivate *priv = buoh_comic_manager_get_instance_private (comic_manager);

        if (priv->current != NULL) {
                buoh_debug ("get_current");
                return priv->current->data;
        } else {
                if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_last) {
                        return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_last) (comic_manager);
                } else {
                        return NULL;
                }
        }
}

gboolean
buoh_comic_manager_is_the_last (BuohComicManager *comic_manager)
{
        GList *current;

        g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), FALSE);
        BuohComicManagerPrivate *priv = buoh_comic_manager_get_instance_private (comic_manager);

        current = priv->current;

        return (current == g_list_last (priv->comic_list));
}

gboolean
buoh_comic_manager_is_the_first (BuohComicManager *comic_manager)
{
        g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), FALSE);

        if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->is_the_first) {
                return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->is_the_first) (comic_manager);
        } else {
                return FALSE;
        }
}

const gchar *
buoh_comic_manager_get_title (BuohComicManager *comic_manager)
{
        g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);
        BuohComicManagerPrivate *priv = buoh_comic_manager_get_instance_private (comic_manager);

        return priv->title;
}

const gchar *
buoh_comic_manager_get_author (BuohComicManager *comic_manager)
{
        g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);
        BuohComicManagerPrivate *priv = buoh_comic_manager_get_instance_private (comic_manager);

        return priv->author;
}

const gchar *
buoh_comic_manager_get_language (BuohComicManager *comic_manager)
{
        g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);
        BuohComicManagerPrivate *priv = buoh_comic_manager_get_instance_private (comic_manager);

        return priv->language;
}

const gchar *
buoh_comic_manager_get_id (BuohComicManager *comic_manager)
{
        g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);
        BuohComicManagerPrivate *priv = buoh_comic_manager_get_instance_private (comic_manager);

        return priv->id;
}

gint
buoh_comic_manager_compare (gconstpointer a, gconstpointer b)
{
        GDate *date1, *date2;

        date1 = buoh_comic_get_date (BUOH_COMIC (a));
        date2 = buoh_comic_get_date (BUOH_COMIC (b));

        return g_date_compare (date1, date2);
}
