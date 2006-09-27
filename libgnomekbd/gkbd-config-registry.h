#ifndef __GKBD_CONFIG_REGISTRY_H__
#define __GKBD_CONFIG_REGISTRY_H__

#include <dbus/dbus-glib-bindings.h>
#include <libxklavier/xklavier.h>

typedef struct GkbdConfigRegistry GkbdConfigRegistry;
typedef struct GkbdConfigRegistryClass GkbdConfigRegistryClass;

struct GkbdConfigRegistry {
	GObject parent;

	XklEngine *engine;
	XklConfigRegistry *registry;
};

struct GkbdConfigRegistryClass {
	GObjectClass parent;
	DBusGConnection *connection;
};

#define GKBD_CONFIG_TYPE_REGISTRY              (gkbd_config_registry_get_type ())
#define GKBD_CONFIG_REGISTRY(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), GKBD_CONFIG_TYPE_REGISTRY, GkbdConfigRegistry))
#define GKBD_CONFIG_REGISTRY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GKBD_CONFIG_TYPE_REGISTRY, GkbdConfigRegistryClass))
#define GKBD_IS_CONFIG_REGISTRY(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), GKBD_CONFIG_TYPE_REGISTRY))
#define GKBD_IS_CONFIG_REGISTRY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GKBD_CONFIG_TYPE_REGISTRY))
#define GKBD_CONFIG_REGISTRY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GKBD_CONFIG_TYPE_REGISTRY, GkbdConfigRegistryClass))


/**
 * DBUS server
 */

extern GType gkbd_config_registry_get_type (void);

extern gboolean
    gkbd_config_registry_get_current_descriptions_as_utf8
    (GkbdConfigRegistry * registry,
     gchar *** short_layout_descriptions,
     gchar *** long_layout_descriptions,
     gchar *** short_variant_descriptions,
     gchar *** long_variant_descriptions, GError ** error);

#endif
