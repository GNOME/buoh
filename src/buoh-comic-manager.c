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

#include "buoh-comic-manager.h"

#define BUOH_COMIC_MANAGER_GET_PRIVATE(object) \
     	  (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_COMIC_MANAGER_TYPE, BuohComicManagerPrivate))

enum {
	PROP_0,
	PROP_TITLE,
	PROP_AUTHOR,
	PROP_LANGUAGE,
	PROP_ID,
	PROP_GENERIC_URI
};

struct _BuohComicManagerPrivate {
	gchar *author;
	gchar *title;
	gchar *language;
	gchar *id;
	gchar *generic_uri;
	/* Comic list must be in the derivated class */
};

static GObjectClass *parent_class = NULL;

static void buoh_comic_manager_init         (BuohComicManager      *comic_manager);
static void buoh_comic_manager_class_init   (BuohComicManagerClass *klass);
static void buoh_comic_manager_finalize     (GObject        *object);
static void buoh_comic_manager_get_property (GObject        *object,
					     guint           prop_id,
					     GValue         *value,
					     GParamSpec     *pspec);
static void buoh_comic_manager_set_property (GObject        *object,
					     guint           prop_id,
					     const GValue   *value,
					     GParamSpec     *pspec);

GType
buoh_comic_manager_get_type ()
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (BuohComicManagerClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) buoh_comic_manager_class_init,
			NULL,
			NULL,
			sizeof (BuohComicManager),
			0,
			(GInstanceInitFunc) buoh_comic_manager_init
		};

		type = g_type_register_static (G_TYPE_OBJECT, "Comic Manager",
					       &info, G_TYPE_FLAG_ABSTRACT);
	}
	
	return type;
}

static void
buoh_comic_manager_init (BuohComicManager *comic_manager)
{
	comic_manager->priv = BUOH_COMIC_MANAGER_GET_PRIVATE (comic_manager);
	
	comic_manager->priv->author      = NULL;
	comic_manager->priv->title       = NULL;
	comic_manager->priv->language    = NULL;
	comic_manager->priv->id          = NULL;
	comic_manager->priv->generic_uri = NULL;
}

static void
buoh_comic_manager_class_init (BuohComicManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (BuohComicManagerPrivate));

	object_class->set_property = buoh_comic_manager_set_property;
	object_class->get_property = buoh_comic_manager_get_property;
	   
	g_object_class_install_property (object_class,
					 PROP_TITLE,
					 g_param_spec_string ("title",
							      "Title",
							      "Title of the comic",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_AUTHOR,
					 g_param_spec_string ("author",
							      "Author",
							      "Name of the author",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_LANGUAGE,
					 g_param_spec_string ("language",
							      "Language",
							      "Language of the comic",
							      NULL,
							      G_PARAM_READWRITE));
	
	g_object_class_install_property (object_class,
					 PROP_ID,
					 g_param_spec_string ("id",
							      "Id",
							      "Identificator of the comic",
							      NULL,
							      G_PARAM_READWRITE));
	
	g_object_class_install_property (object_class,
					 PROP_GENERIC_URI,
					 g_param_spec_string ("generic_uri",
							      "Generic URI",
							      "URI with date format",
							      NULL,
							      G_PARAM_READWRITE));
	
	object_class->finalize = buoh_comic_manager_finalize;
}

static void
buoh_comic_manager_finalize (GObject *object)
{
	BuohComicManager *comic_manager = BUOH_COMIC_MANAGER (object);
	
	g_return_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager));

	if (comic_manager->priv->author) {
		g_free (comic_manager->priv->author);
		comic_manager->priv->author = NULL;
	}

	if (comic_manager->priv->title) {
		g_free (comic_manager->priv->title);
		comic_manager->priv->title = NULL;
	}

	if (comic_manager->priv->language) {
		g_free (comic_manager->priv->language);
		comic_manager->priv->language = NULL;
	}

	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
buoh_comic_manager_set_property (GObject      *object,
			         guint         prop_id,
			         const GValue *value,
			         GParamSpec   *pspec)
{
	BuohComicManager *comic_manager = BUOH_COMIC_MANAGER (object);

	switch (prop_id) {
	case PROP_TITLE:
		g_free (comic_manager->priv->title);
		comic_manager->priv->title = g_value_dup_string (value);
		
		break;
	case PROP_AUTHOR:
		g_free (comic_manager->priv->author);
		comic_manager->priv->author = g_value_dup_string (value);
		
		break;
	case PROP_LANGUAGE:
		g_free (comic_manager->priv->language);
		comic_manager->priv->language = g_value_dup_string (value);

		break;
	case PROP_ID:
		g_free (comic_manager->priv->id);
		comic_manager->priv->id = g_value_dup_string (value);
		
		break;
	case PROP_GENERIC_URI:
		g_free (comic_manager->priv->generic_uri);
		comic_manager->priv->generic_uri = g_value_dup_string (value);
		
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

	switch (prop_id) {
	case PROP_TITLE:
		g_value_set_string (value, comic_manager->priv->title);
		
		break;
	case PROP_AUTHOR:
		g_value_set_string (value, comic_manager->priv->author);
		
		break;
	case PROP_LANGUAGE:
		g_value_set_string (value, comic_manager->priv->language);

		break;
	case PROP_ID:
		g_value_set_string (value, comic_manager->priv->id);
		
		break;
	case PROP_GENERIC_URI:
		g_value_set_string (value, comic_manager->priv->generic_uri);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

gchar *
buoh_comic_manager_get_uri (BuohComicManager *comic_manager)
{
	if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_uri)
		return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_uri) (comic_manager);
	else
		return NULL;
}

gchar *
buoh_comic_manager_get_page (BuohComicManager *comic_manager)
{
	if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_uri)
		return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_page) (comic_manager);
	else
		return NULL;
}

BuohComic *
buoh_comic_manager_get_next (BuohComicManager *comic_manager)
{
	if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_next)
		return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_next) (comic_manager);
	else
		return NULL;
}

BuohComic *
buoh_comic_manager_get_previous (BuohComicManager *comic_manager)
{
	if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_previous)
		return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_previous) (comic_manager);
	else
		return NULL;
}

BuohComic *
buoh_comic_manager_get_n_next (BuohComicManager *comic_manager,
			       int n)
{
	if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_next)
		return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_n_next) (comic_manager,
									      n);
	else
		return NULL;
}

BuohComic *
buoh_comic_manager_get_n_previous (BuohComicManager *comic_manager,
			     	   int n)
{
	if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_previous)
		return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->get_n_previous) (comic_manager,
										 n);
	else
		return NULL;
}

gboolean
buoh_comic_manager_is_the_last (BuohComicManager *comic_manager)
{
	if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->is_the_last)
		return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->is_the_last) (comic_manager);
	else
		return FALSE;
}

gboolean
buoh_comic_manager_is_the_first (BuohComicManager *comic_manager)
{
	if (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->is_the_last)
		return (BUOH_COMIC_MANAGER_GET_CLASS (comic_manager)->is_the_first)(comic_manager);
	else
		return FALSE;
}

void
buoh_comic_manager_set_title (BuohComicManager *comic_manager, const gchar *title)
{
	g_return_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager));
	
	g_object_set (G_OBJECT (comic_manager), "title", title, NULL);
}

void
buoh_comic_manager_set_author (BuohComicManager *comic_manager, const gchar *author)
{
	g_return_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager));

	g_object_set (G_OBJECT (comic_manager), "author", author, NULL);
}

void
buoh_comic_manager_set_language (BuohComicManager *comic_manager, const gchar *language)
{
	g_return_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager));

	g_object_set (G_OBJECT (comic_manager), "language", language, NULL);
}

void
buoh_comic_manager_set_id (BuohComicManager *comic_manager, const gchar *id)
{
	g_return_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager));

	g_object_set (G_OBJECT (comic_manager), "id", id, NULL);
}

gchar *
buoh_comic_manager_get_title (BuohComicManager *comic_manager)
{
	gchar *title = NULL;
	   
	g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);

	g_object_get (G_OBJECT (comic_manager), "title", &title, NULL);

	return title;
}

gchar *
buoh_comic_manager_get_author (BuohComicManager *comic_manager)
{
	gchar *author = NULL;
	   
	g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);

	g_object_get (G_OBJECT (comic_manager), "author", &author, NULL);

	return author;
}

gchar *
buoh_comic_manager_get_language (BuohComicManager *comic_manager)
{
	gchar *language = NULL;

	g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);

	g_object_get (G_OBJECT (comic_manager), "language", &language, NULL);

	return language;
}

gchar *
buoh_comic_manager_get_id (BuohComicManager *comic_manager)
{
	gchar *id = NULL;
	   
	g_return_val_if_fail (BUOH_IS_COMIC_MANAGER (comic_manager), NULL);

	g_object_get (G_OBJECT (comic_manager), "id", &id, NULL);

	return id;
}


