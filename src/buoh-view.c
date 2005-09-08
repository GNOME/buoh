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
 *  Authors : Pablo Arroyo Loma (zioma) <zioma@linups.org>
 *            Esteban Sanchez Muñoz (steve-o) <esteban@steve-o.org>
 *            Carlos García Campos <carlosgc@gnome.org>
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "buoh-view.h"
#include "buoh-view-comic.h"
#include "buoh-view-message.h"

enum {
        PROP_0,
	PROP_STATUS
};

enum {
	VIEW_PAGE_IMAGE,
	VIEW_PAGE_MESSAGE,
	VIEW_PAGE_EMPTY
};

enum {
	SCALE_CHANGED,
	N_SIGNALS
};

struct _BuohViewPrivate {
	GtkWidget       *message;
	GtkWidget       *comic;

	BuohViewStatus   status;
};

#define BUOH_VIEW_GET_PRIVATE(object) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((object), BUOH_TYPE_VIEW, BuohViewPrivate))

static GtkNotebookClass *parent_class = NULL;
static guint buoh_view_signals[N_SIGNALS];

static void buoh_view_init              (BuohView      *buoh_view);
static void buoh_view_class_init        (BuohViewClass *klass);
static void buoh_view_set_property      (GObject       *object,
					 guint          prop_id,
					 const GValue  *value,
					 GParamSpec    *pspec);
static void buoh_view_get_property      (GObject       *object,
					 guint          prop_id,
					 GValue        *value,
					 GParamSpec    *pspec);
static void buoh_view_status_changed_cb (GObject       *object,
					 GParamSpec    *arg,
					 gpointer       gdata);
static void buoh_view_scale_changed_cb  (GObject       *object,
					 GParamSpec    *arg,
					 gpointer       gdata);

GType
buoh_view_get_type ()
{
        static GType type = 0;

        if (!type) {
                static const GTypeInfo info = {
                        sizeof (BuohViewClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) buoh_view_class_init,
                        NULL,
                        NULL,
                        sizeof (BuohView),
                        0,
                        (GInstanceInitFunc) buoh_view_init
                };

                type = g_type_register_static (GTK_TYPE_NOTEBOOK, "BuohView",
                                               &info, 0);
        }

        return type;
}

GType
buoh_view_status_get_type ()
{
	static GType etype = 0;
	
	if (etype == 0) {
		static const GEnumValue values[] = {
			{ STATE_MESSAGE_WELCOME, "STATE_MESSAGE_WELCOME", "welcome" },
			{ STATE_MESSAGE_ERROR,   "STATE_MESSAGE_ERROR",   "error"   },
			{ STATE_COMIC_LOADING,   "STATE_COMIC_LOADING",   "loading" },
			{ STATE_COMIC_LOADED,    "STATE_COMIC_LOADED",    "loaded"  },
			{ STATE_EMPTY,           "STATE_EMPTY",           "empty"   },
			{ 0, NULL, NULL }
		};
		
		etype = g_enum_register_static ("BuohViewStatus", values);
	}
	
	return etype;
}

static void
buoh_view_init (BuohView *buoh_view)
{
        GtkWidget *label;
        GtkWidget *swindow;
        
        buoh_view->priv = BUOH_VIEW_GET_PRIVATE (buoh_view);

	buoh_view->priv->status = STATE_MESSAGE_WELCOME;

	/* Image view */
        swindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
	buoh_view->priv->comic = buoh_view_comic_new (buoh_view);
	gtk_container_add (GTK_CONTAINER (swindow),
			   buoh_view->priv->comic);
	gtk_widget_show (buoh_view->priv->comic);

        gtk_notebook_insert_page (GTK_NOTEBOOK (buoh_view), swindow,
                                  NULL, VIEW_PAGE_IMAGE);
        gtk_widget_show (swindow);

        /* Message view */
	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_NEVER,
					GTK_POLICY_NEVER);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swindow),
					     GTK_SHADOW_NONE);
	buoh_view->priv->message = buoh_view_message_new ();
	buoh_view_message_set_title (BUOH_VIEW_MESSAGE (buoh_view->priv->message),
				     _("Buoh Comics Reader"));
	buoh_view_message_set_text (BUOH_VIEW_MESSAGE (buoh_view->priv->message),
				    _("Welcome to <b>Buoh</b>, the comics browser for GNOME Desktop.\n"
				      "The list on the left panel contains your favourite comics, "
				      "to add or remove comics to the list click on Comic -> Add. "
				      "Just select a comic from the list, and it will be displayed "
				      "on the right side. Thanks for using Buoh."));
	buoh_view_message_set_icon (BUOH_VIEW_MESSAGE (buoh_view->priv->message),
				    "buoh64x64.png");
	gtk_container_add (GTK_CONTAINER (swindow),
			   buoh_view->priv->message);
	gtk_widget_show (buoh_view->priv->message);
	
        gtk_notebook_insert_page (GTK_NOTEBOOK (buoh_view), swindow,
                                  NULL, VIEW_PAGE_MESSAGE);
        gtk_widget_show (swindow);

        /* Empty view */
        label = gtk_label_new (NULL);
        gtk_notebook_insert_page (GTK_NOTEBOOK (buoh_view), label,
                                  NULL, VIEW_PAGE_EMPTY);
        gtk_widget_show (label);

	
        gtk_notebook_set_current_page (GTK_NOTEBOOK (buoh_view), VIEW_PAGE_MESSAGE);

        /* Callbacks */
        g_signal_connect (G_OBJECT (buoh_view), "notify::status",
                          G_CALLBACK (buoh_view_status_changed_cb),
                          NULL);
	g_signal_connect (G_OBJECT (buoh_view->priv->comic),
			  "notify::scale",
			  G_CALLBACK (buoh_view_scale_changed_cb),
			  (gpointer) buoh_view);

        gtk_widget_show (GTK_WIDGET (buoh_view));
}

static void
buoh_view_class_init (BuohViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->set_property = buoh_view_set_property;
	object_class->get_property = buoh_view_get_property;
        
        parent_class = g_type_class_peek_parent (klass);

        g_type_class_add_private (klass, sizeof (BuohViewPrivate));

        /* Properties */
        g_object_class_install_property (object_class,
                                         PROP_STATUS,
                                         g_param_spec_enum ("status",
							    "Status",
							    "The curent view status",
							    BUOH_TYPE_VIEW_STATUS,
							    STATE_MESSAGE_WELCOME,
							    G_PARAM_READWRITE));

	/* Signals */
	buoh_view_signals[SCALE_CHANGED] =
		g_signal_new ("scale-changed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (BuohViewClass, scale_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
}

static void
buoh_view_set_property (GObject       *object,
                        guint          prop_id,
                        const GValue  *value,
                        GParamSpec    *pspec)
{
        BuohView *view = BUOH_VIEW (object);
        
        switch (prop_id) {
        case PROP_STATUS:
                view->priv->status = g_value_get_enum (value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static void
buoh_view_get_property (GObject       *object,
                        guint          prop_id,
                        GValue        *value,
                        GParamSpec    *pspec)
{
        BuohView *view = BUOH_VIEW (object);

        switch (prop_id) {
        case PROP_STATUS:
                g_value_set_enum (value, view->priv->status);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

GtkWidget *
buoh_view_new (void)
{
        GtkWidget *buoh_view;

        buoh_view = GTK_WIDGET (g_object_new (BUOH_TYPE_VIEW,
                                              "show-tabs", FALSE,
                                              NULL));
        return buoh_view;
}

static void
buoh_view_status_changed_cb (GObject *object, GParamSpec *arg, gpointer gdata)
{
        BuohView *view = BUOH_VIEW (object);

	switch (view->priv->status) {
	case STATE_MESSAGE_WELCOME:
	case STATE_MESSAGE_ERROR:
		gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_MESSAGE);
		break;
	case STATE_COMIC_LOADING:
	case STATE_COMIC_LOADED:
		gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_IMAGE);
		break;
	case STATE_EMPTY:
		gtk_notebook_set_current_page (GTK_NOTEBOOK (view), VIEW_PAGE_EMPTY);
		break;
	default:
		break;
	}
}

static void
buoh_view_scale_changed_cb (GObject *object, GParamSpec *arg, gpointer gdata)
{
	BuohView *view = BUOH_VIEW (gdata);

	g_signal_emit (view, buoh_view_signals[SCALE_CHANGED], 0);
}

gboolean
buoh_view_is_min_zoom (BuohView *view)
{
	return buoh_view_comic_is_min_zoom (BUOH_VIEW_COMIC (view->priv->comic));
}

gboolean
buoh_view_is_max_zoom (BuohView *view)
{
	return buoh_view_comic_is_max_zoom (BUOH_VIEW_COMIC (view->priv->comic));
}

gboolean
buoh_view_is_normal_size (BuohView *view)
{
	return buoh_view_comic_is_normal_size (BUOH_VIEW_COMIC (view->priv->comic));
}

void
buoh_view_zoom_in (BuohView *view)
{
	buoh_view_comic_zoom_in (BUOH_VIEW_COMIC (view->priv->comic));
}

void
buoh_view_zoom_out (BuohView *view)
{
	buoh_view_comic_zoom_out (BUOH_VIEW_COMIC (view->priv->comic));
}

void
buoh_view_normal_size (BuohView *view)
{
	buoh_view_comic_normal_size (BUOH_VIEW_COMIC (view->priv->comic));
}

BuohViewStatus
buoh_view_get_status (BuohView *view)
{
	return view->priv->status;
}

void
buoh_view_set_comic (BuohView *view, BuohComic *comic)
{
        g_return_if_fail (BUOH_IS_COMIC (comic));

        g_object_set (G_OBJECT (view->priv->comic),
                      "comic", comic,
                      NULL);
}

BuohComic *
buoh_view_get_comic (BuohView *view)
{
        BuohComic *comic = NULL;

        g_object_get (G_OBJECT (view->priv->comic),
                      "comic", &comic,
                      NULL);

        return comic;
}

void
buoh_view_set_message_title (BuohView *view, const gchar *title)
{
	buoh_view_message_set_title (BUOH_VIEW_MESSAGE (view->priv->message),
				     title);
}

void
buoh_view_set_message_text (BuohView *view, const gchar *text)
{
	buoh_view_message_set_text (BUOH_VIEW_MESSAGE (view->priv->message),
				    text);
}

void
buoh_view_set_message_icon (BuohView *view, const gchar *icon)
{
	buoh_view_message_set_icon (BUOH_VIEW_MESSAGE (view->priv->message),
				    icon);
}

void
buoh_view_clear (BuohView *view)
{
	g_object_set (G_OBJECT (view),
		      "status", STATE_EMPTY,
		      NULL);
}
