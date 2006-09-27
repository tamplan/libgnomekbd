#include <config.h>

#include <gdk/gdkx.h>

#include <gkb-config-registry.h>
#include <gkb-config-registry-server.h>
#include <gkb-keyboard-config.h>

static GObjectClass *parent_class = NULL;

gboolean
    gkb_config_registry_get_current_descriptions_as_utf8
    (GkbConfigRegistry * registry,
     gchar *** short_layout_descriptions,
     gchar *** long_layout_descriptions,
     gchar *** short_variant_descriptions,
     gchar *** long_variant_descriptions, GError ** error) {
	XklConfigRec *xkl_config;
	char **pl, **pv;
	guint total_layouts;
	gchar **sld, **lld, **svd, **lvd;

	if (!registry->registry) {
		registry->registry =
		    xkl_config_registry_get_instance (registry->engine);

		xkl_config_registry_load (registry->registry);
	}

	if (!
	    (xkl_engine_get_features (registry->engine) &
	     XKLF_MULTIPLE_LAYOUTS_SUPPORTED))
		return FALSE;

	xkl_config = xkl_config_rec_new ();

	if (!xkl_config_rec_get_from_server (xkl_config, registry->engine))
		return FALSE;

	pl = xkl_config->layouts;
	pv = xkl_config->variants;
	total_layouts = g_strv_length (xkl_config->layouts);
	sld = *short_layout_descriptions =
	    g_new0 (char *, total_layouts + 1);
	lld = *long_layout_descriptions =
	    g_new0 (char *, total_layouts + 1);
	svd = *short_variant_descriptions =
	    g_new0 (char *, total_layouts + 1);
	lvd = *long_variant_descriptions =
	    g_new0 (char *, total_layouts + 1);

	while (pl != NULL && *pl != NULL) {
		XklConfigItem item;

		g_snprintf (item.name, sizeof item.name, "%s", *pl);
		if (xkl_config_registry_find_layout
		    (registry->registry, &item)) {
			*sld++ = g_strdup (item.short_description);
			*lld++ = g_strdup (item.description);
		} else {
			*sld++ = g_strdup ("");
			*lld++ = g_strdup ("");
		}

		if (*pv != NULL) {
			g_snprintf (item.name, sizeof item.name, "%s",
				    *pv);
			if (xkl_config_registry_find_variant
			    (registry->registry, *pl, &item)) {
				*svd = g_strdup (item.short_description);
				*lvd = g_strdup (item.description);
			} else {
				*svd++ = g_strdup ("");
				*lvd++ = g_strdup ("");
			}
		} else {
			*svd++ = g_strdup ("");
			*lvd++ = g_strdup ("");
		}

		pl++;
		pv++;
	}
	g_object_unref (G_OBJECT (xkl_config));

	return TRUE;
}

G_DEFINE_TYPE (GkbConfigRegistry, gkb_config_registry, G_TYPE_OBJECT)
static void
finalize (GObject * object)
{
	GkbConfigRegistry *registry;

	registry = GKB_CONFIG_REGISTRY (object);
	if (registry->registry == NULL) {
		return;
	}

	g_object_unref (registry->registry);
	registry->registry = NULL;

	g_object_unref (registry->engine);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gkb_config_registry_class_init (GkbConfigRegistryClass * klass)
{
	GError *error = NULL;
	GObjectClass *object_class;

	/* Init the DBus connection, per-klass */
	klass->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (klass->connection == NULL) {
		g_warning ("Unable to connect to dbus: %s",
			   error->message);
		g_error_free (error);
		return;
	}

	dbus_g_object_type_install_info (GKB_CONFIG_TYPE_REGISTRY,
					 &dbus_glib_gkb_config_registry_object_info);

	object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = finalize;

	parent_class = g_type_class_peek_parent (klass);
}

static void
gkb_config_registry_init (GkbConfigRegistry * registry)
{
	GError *error = NULL;
	DBusGProxy *driver_proxy;
	GkbConfigRegistryClass *klass =
	    GKB_CONFIG_REGISTRY_GET_CLASS (registry);
	unsigned request_ret;

	/* Register DBUS path */
	dbus_g_connection_register_g_object (klass->connection,
					     "/org/gnome/GkbConfigRegistry",
					     G_OBJECT (registry));

	/* Register the service name, the constant here are defined in dbus-glib-bindings.h */
	driver_proxy = dbus_g_proxy_new_for_name (klass->connection,
						  DBUS_SERVICE_DBUS,
						  DBUS_PATH_DBUS,
						  DBUS_INTERFACE_DBUS);

	if (!org_freedesktop_DBus_request_name
	    (driver_proxy, "org.gnome.GkbConfigRegistry", 0,
	     &request_ret, &error)) {
		g_warning ("Unable to register service: %s",
			   error->message);
		g_error_free (error);
	}
	g_object_unref (driver_proxy);

	/* Init libxklavier stuff */
	registry->engine = xkl_engine_get_instance (GDK_DISPLAY ());
	/* Lazy initialization */
	registry->registry = NULL;
}
