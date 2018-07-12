#include <hildon-im-settings-plugin.h>

#include <assert.h>

#define CVIM_SETTINGS_TYPE (cvim_settings_get_type())
#define CVIM_SETTINGS(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), CVIM_SETTINGS_TYPE, \
                                    HildonIMWesternCommonVIMSettings))

struct _HildonIMWesternCommonVIMSettings
{
  GObject parent_instance;
  HildonIMSettingsPluginManager *manager;
};
typedef struct _HildonIMWesternCommonVIMSettings HildonIMWesternCommonVIMSettings;

struct _HildonIMWesternCommonVIMSettingsClass
{
  GObjectClass parent_class;
};
typedef struct _HildonIMWesternCommonVIMSettingsClass HildonIMWesternCommonVIMSettingsClass;

static void cvim_settings_interface_init(HildonIMSettingsPluginIface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(HildonIMWesternCommonVIMSettings,
                               cvim_settings, G_TYPE_OBJECT, 0,
                               G_IMPLEMENT_INTERFACE_DYNAMIC(HILDON_IM_TYPE_SETTINGS_PLUGIN,
                                                             cvim_settings_interface_init)
)

static void
cvim_settings_init(HildonIMWesternCommonVIMSettings *self)
{
}

static void
cvim_settings_finalize(GObject *object)
{
  if (G_OBJECT_CLASS(cvim_settings_parent_class)->finalize)
    G_OBJECT_CLASS(cvim_settings_parent_class)->finalize(object);
}

static void
cvim_settings_class_init(HildonIMWesternCommonVIMSettingsClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = cvim_settings_finalize;
}

static void
cvim_settings_class_finalize(HildonIMWesternCommonVIMSettingsClass *klass)
{
}

static void
cvim_settings_set_manager(HildonIMSettingsPlugin *plugin,
                          HildonIMSettingsPluginManager *manager)
{
  CVIM_SETTINGS(plugin)->manager = manager;
}

static GtkWidget *
cvim_settings_create_widget(HildonIMSettingsPlugin *plugin,
                            HildonIMSettingsCategory category,
                            GtkSizeGroup *size_group, gint *width)
{
  assert(0);
  return NULL;
}

static void
cvim_settings_value_changed(HildonIMSettingsPlugin *plugin, const gchar *key,
                            GType type, gpointer value)
{
  assert(0);
}

static void
cvim_settings_save_data(HildonIMSettingsPlugin *plugin,
                        HildonIMSettingsCategory where)
{
  assert(0);
}

static void
cvim_settings_interface_init(HildonIMSettingsPluginIface *iface)
{
  iface->set_manager = cvim_settings_set_manager;
  iface->create_widget = cvim_settings_create_widget;
  iface->value_changed = cvim_settings_value_changed;
  iface->save_data = cvim_settings_save_data;
}

void
settings_plugin_init(GTypeModule *type_module)
{
  cvim_settings_register_type(type_module);
}

gpointer
settings_plugin_new()
{
  return g_object_new(CVIM_SETTINGS_TYPE, NULL);
}

void
settings_plugin_exit()
{
}
