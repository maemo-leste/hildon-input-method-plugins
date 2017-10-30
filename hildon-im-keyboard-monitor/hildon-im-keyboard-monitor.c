/**
   @file hildon-im-keyboard-monitor.c

   This file is part of hildon-input-method-plugins.

   This file may contain parts derived by disassembling of binaries under
   Nokia's copyright, see http://tablets-dev.nokia.com/maemo-dev-env-downloads.php

   The original licensing conditions apply to all those derived parts as well
   and you accept those by using this file.
*/

#include <string.h>
#include <libintl.h>
#include <locale.h>
#include <glib.h>
#include <assert.h>

#include <gtk/gtkbox.h>

#include <hildon/hildon.h>

#include <hildon-im-widget-loader.h>
#include <hildon-im-plugin.h>
#include <hildon-im-ui.h>
#include <hildon-im-languages.h>
#include <hildon-im-common.h>

#include <hildon-im-vkbrenderer.h>

#include "hildon-im-keyboard-monitor.h"
#include "hildon-im-western-plugin-common.h"
#include "hildon-im-xkb.h"

struct _HildonIMKeyboardMonitorPrivate
{
  HildonIMUI *ui;
  GtkWidget *banner;
  int slide_open;
  int keyboard_attached;
  int int_kb_level_shifted;
  gchar *int_kb_model;
  gchar *int_kb_layout;
  int int_kb_repeat_delay;
  int int_kb_repeat_interval;
  gchar *ext_kb_model;
  gchar *ext_kb_layout;
  int ext_kb_repeat_delay;
  int ext_kb_repeat_interval;
  int timer;
  gboolean int_setting_changed;
  gboolean ext_setting_changed;
  gboolean language_switched;
};

GType hildon_im_type_keyboard_monitor = 0;
GtkWidgetClass *hildon_im_keyboard_monitor_parent_class = NULL;
static gboolean activate_special_plugin;

static void hildon_im_keyboard_monitor_class_init(HildonIMKeyboardMonitorClass *klass);
static void hildon_im_keyboard_monitor_init(HildonIMKeyboardMonitor *monitor);
static void hildon_im_keyboard_monitor_iface_init(HildonIMPluginIface *iface);
static void hildon_im_keyboard_monitor_set_property(GObject *object,guint prop_id,const GValue *value,GParamSpec *pspec);
static void hildon_im_keyboard_monitor_get_property (GObject *object,guint prop_id,GValue *value,GParamSpec *pspec);
static void hildon_im_keyboard_monitor_finalize(GObject *obj);
static void hildon_im_keyboard_monitor_enable(HildonIMPlugin *plugin, gboolean init);
static void hildon_im_keyboard_monitor_disable(HildonIMPlugin *plugin);
static void hildon_im_keyboard_monitor_client_widget_changed(HildonIMPlugin *plugin);
static void hildon_im_keyboard_monitor_save_data(HildonIMPlugin *plugin);
static void hildon_im_keyboard_monitor_settings_changed(HildonIMPlugin *plugin, const gchar *key, const GConfValue *value);
static void hildon_im_keyboard_monitor_language(HildonIMPlugin *plugin);
static GObject *hildon_im_keyboard_monitor_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_properties);
static void hildon_im_keyboard_monitor_key_event(HildonIMPlugin *plugin, GdkEventType type, guint state, guint val, guint hardware_keycode);
static void hildon_im_keyboard_monitor_gconf_notify_cb(GConfClient *client, guint cnxn_id, GConfEntry *entry, HildonIMPlugin *user_data);
static void hildon_im_keyboard_monitor_update_keyboard_state(HildonIMKeyboardMonitor *monitor);
static void hildon_im_keyboard_monitor_update_settings(HildonIMKeyboardMonitor *monitor);
static void hildon_im_keyboard_monitor_set_lock_level(HildonIMKeyboardMonitor *monitor);
static gboolean hildon_im_keyboard_monitor_timeout_cb(HildonIMKeyboardMonitor *monitor);

GType
hildon_im_keyboard_monitor_get_type (void)
{
  return hildon_im_type_keyboard_monitor;
}

GObject *hildon_im_keyboard_monitor_new(HildonIMUI *ui)
{
  return g_object_new(HILDON_IM_TYPE_KEYBOARD_MONITOR,
                                 HILDON_IM_PROP_UI_DESCRIPTION,
                                 ui,
                                 0);
}

HildonIMPlugin*
module_create (HildonIMUI *ui)
{
  return HILDON_IM_PLUGIN (hildon_im_keyboard_monitor_new (ui));
}

void
module_exit(void)
{
  /* empty */
}

void
module_init(GTypeModule *module)
{
  static const GTypeInfo type_info = {
    sizeof(HildonIMKeyboardMonitorClass),
    NULL, /* base_init */
    NULL, /* base_finalize */
    (GClassInitFunc) hildon_im_keyboard_monitor_class_init,
    NULL, /* class_finalize */
    NULL, /* class_data */
    sizeof(HildonIMKeyboardMonitor),
    0, /* n_preallocs */
    (GInstanceInitFunc) hildon_im_keyboard_monitor_init,
    NULL
  };

  static const GInterfaceInfo plugin_info = {
    (GInterfaceInitFunc) hildon_im_keyboard_monitor_iface_init,
    NULL, /* interface_finalize */
    NULL, /* interface_data */
  };

  hildon_im_type_keyboard_monitor =
          g_type_module_register_type(module,
                                      GTK_TYPE_CONTAINER,
                                      "HildonIMKeyboardMonitor",
                                      &type_info,
                                      0);

  g_type_module_add_interface(module,
                              HILDON_IM_TYPE_KEYBOARD_MONITOR,
                              HILDON_IM_TYPE_PLUGIN,
                              &plugin_info);
}

static void 
hildon_im_keyboard_monitor_class_init(HildonIMKeyboardMonitorClass *klass)
{
  GObjectClass *object_class;

  hildon_im_keyboard_monitor_parent_class = g_type_class_peek_parent(klass);
  g_type_class_add_private(klass, sizeof(HildonIMKeyboardMonitorPrivate));

  object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = hildon_im_keyboard_monitor_set_property;
  object_class->get_property = hildon_im_keyboard_monitor_get_property;
  object_class->constructor = hildon_im_keyboard_monitor_constructor;
  object_class->finalize = hildon_im_keyboard_monitor_finalize;

  g_object_class_install_property(object_class, HILDON_IM_PROP_UI,
                                  g_param_spec_object(
                                    HILDON_IM_PROP_UI_DESCRIPTION,
                                    HILDON_IM_PROP_UI_DESCRIPTION,
                                    "UI that uses plugin",
                                    HILDON_IM_TYPE_UI,
                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
hildon_im_keyboard_monitor_iface_init(HildonIMPluginIface *iface)
{
  iface->key_event = hildon_im_keyboard_monitor_key_event;
  iface->enable = hildon_im_keyboard_monitor_enable;
  iface->disable = hildon_im_keyboard_monitor_disable;
  iface->client_widget_changed = hildon_im_keyboard_monitor_client_widget_changed;
  iface->save_data = hildon_im_keyboard_monitor_save_data;
  iface->settings_changed = hildon_im_keyboard_monitor_settings_changed;
  iface->language = hildon_im_keyboard_monitor_language;
}

static void hildon_im_keyboard_monitor_init(HildonIMKeyboardMonitor *monitor)
{
  monitor->priv = HILDON_IM_KEYBOARD_MONITOR_GET_PRIVATE (HILDON_IM_KEYBOARD_MONITOR(monitor));
  monitor->priv->banner = NULL;
}

static void
hildon_im_keyboard_monitor_enable(HildonIMPlugin *plugin, gboolean init)
{
  HildonIMKeyboardMonitor *monitor;
  HildonIMKeyboardMonitorPrivate *priv;

  monitor = HILDON_IM_KEYBOARD_MONITOR(plugin);
  priv = monitor->priv;
  gconf_client_add_dir(priv->ui->client, "/system/osso/af", 0, 0);
  gconf_client_notify_add(priv->ui->client,"/system/osso/af/keyboard-attached",(GConfClientNotifyFunc)hildon_im_keyboard_monitor_gconf_notify_cb,(gpointer)plugin,0,0);
  gconf_client_notify_add(priv->ui->client,"/system/osso/af/slide-open",(GConfClientNotifyFunc)hildon_im_keyboard_monitor_gconf_notify_cb,(gpointer)plugin,0,0);
  priv->slide_open = gconf_client_get_bool(priv->ui->client, "/system/osso/af/slide-open", 0);
  priv->keyboard_attached = gconf_client_get_bool(priv->ui->client, "/system/osso/af/keyboard-attached", 0);
  hildon_im_ui_set_context_options(priv->ui, HILDON_IM_AUTOLEVEL_NUMERIC, 1);
  priv->int_kb_model = gconf_client_get_string(priv->ui->client, "/apps/osso/inputmethod/int_kb_model", 0);
  priv->int_kb_layout = gconf_client_get_string(priv->ui->client, "/apps/osso/inputmethod/int_kb_layout", 0);
  priv->int_kb_repeat_delay = gconf_client_get_int(priv->ui->client, "/apps/osso/inputmethod/int_kb_repeat_delay", 0);
  priv->int_kb_repeat_interval = gconf_client_get_int(priv->ui->client, "/apps/osso/inputmethod/int_kb_repeat_interval", 0);
  priv->int_kb_level_shifted = gconf_client_get_bool(priv->ui->client, "/apps/osso/inputmethod/int_kb_level_shifted", 0);
  priv->ext_kb_model = gconf_client_get_string(priv->ui->client, "/apps/osso/inputmethod/ext_kb_model", 0);
  priv->ext_kb_layout = gconf_client_get_string(priv->ui->client, "/apps/osso/inputmethod/ext_kb_layout", 0);
  priv->ext_kb_repeat_delay = gconf_client_get_int(priv->ui->client, "/apps/osso/inputmethod/ext_kb_repeat_delay", 0);
  priv->ext_setting_changed = TRUE;
  priv->int_setting_changed = TRUE;
  priv->ext_kb_repeat_interval = gconf_client_get_int(priv->ui->client, "/apps/osso/inputmethod/ext_kb_repeat_interval", 0);
  hildon_im_keyboard_monitor_update_keyboard_state(monitor);
  hildon_im_keyboard_monitor_update_settings(monitor);
}

static void
hildon_im_keyboard_monitor_get_property (GObject *object,
                                    guint prop_id,
                                    GValue *value,
                                    GParamSpec *pspec)
{
  HildonIMKeyboardMonitorPrivate *priv;

  priv = HILDON_IM_KEYBOARD_MONITOR(object)->priv;

  switch (prop_id)
  {
    case HILDON_IM_PROP_UI:
      g_value_set_object(value, priv->ui);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void
hildon_im_keyboard_monitor_set_property(GObject *object,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  HildonIMKeyboardMonitorPrivate *priv;

  priv = HILDON_IM_KEYBOARD_MONITOR(object)->priv;

  switch (prop_id)
  {
    case HILDON_IM_PROP_UI:
      priv->ui = g_value_get_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void hildon_im_keyboard_monitor_finalize(GObject *object)
{
  HildonIMKeyboardMonitorPrivate *priv;
  priv = HILDON_IM_KEYBOARD_MONITOR(object)->priv;

  if (priv->banner)
  {
    gtk_widget_destroy(priv->banner);
    priv->banner = NULL;
  }
  if (G_OBJECT_CLASS(hildon_im_keyboard_monitor_parent_class)->finalize)
    G_OBJECT_CLASS(hildon_im_keyboard_monitor_parent_class)->finalize(object);
}

static void
hildon_im_keyboard_monitor_disable(HildonIMPlugin *plugin)
{
  HildonIMKeyboardMonitorPrivate *priv;

  priv = HILDON_IM_KEYBOARD_MONITOR(plugin)->priv;
  if (priv->int_kb_model)
  {
    g_free(priv->int_kb_model);
    priv->int_kb_model = 0;
  }

  if (priv->int_kb_layout)
  {
    g_free(priv->int_kb_layout);
    priv->int_kb_layout = 0;
  }
  if (priv->ext_kb_model)
  {
    g_free(priv->ext_kb_model);
    priv->ext_kb_model = 0;
  }
  if (priv->ext_kb_layout)
  {
    g_free(priv->ext_kb_layout);
    priv->ext_kb_layout = 0;
  }
  g_print("KEYBOARD MONITOR DISABLED\n");
}

static void
hildon_im_keyboard_monitor_client_widget_changed(HildonIMPlugin *plugin)
{
}

static void hildon_im_keyboard_monitor_settings_changed(HildonIMPlugin *plugin, const gchar *key, const GConfValue *value)
{
  HildonIMKeyboardMonitor *monitor;
  HildonIMKeyboardMonitorPrivate *priv;

  monitor = HILDON_IM_KEYBOARD_MONITOR(plugin);
  priv = monitor->priv;
  if (value->type != GCONF_VALUE_STRING)
  {
    if (value->type != GCONF_VALUE_INT)
    {
      if (value->type == GCONF_VALUE_BOOL && !strcmp(key, "/apps/osso/inputmethod/int_kb_level_shifted"))
      {
        priv->int_kb_level_shifted = gconf_value_get_bool(value);
        hildon_im_keyboard_monitor_set_lock_level(monitor);
      }
      goto LABEL_4;
    }
    if (!strcmp(key, "/apps/osso/inputmethod/int_kb_repeat_delay"))
    {
      priv->int_setting_changed = 1;
      priv->int_kb_repeat_delay = gconf_value_get_int(value);
      goto LABEL_5;
    }
    if (strcmp(key, "/apps/osso/inputmethod/int_kb_repeat_interval"))
    {
      if (!strcmp(key, "/apps/osso/inputmethod/ext_kb_repeat_delay"))
      {
        priv->ext_setting_changed = 1;
        priv->ext_kb_repeat_delay = gconf_value_get_int(value);
      }
      else if (!strcmp(key, "/apps/osso/inputmethod/ext_kb_repeat_interval"))
      {
        priv->ext_setting_changed = 1;
        priv->ext_kb_repeat_interval = gconf_value_get_int(value);
      }
      goto LABEL_4;
    }
    priv->int_setting_changed = 1;
    priv->int_kb_repeat_interval = gconf_value_get_int(value);
LABEL_5:
    if (priv->timer)
      return;
    goto LABEL_6;
  }
  if (!strcmp(key, "/apps/osso/inputmethod/int_kb_model"))
  {
    if (priv->int_kb_model)
      g_free(priv->int_kb_model);
    priv->int_setting_changed = 1;
    priv->int_kb_model = g_strdup(gconf_value_get_string(value));
    if (!priv->timer)
      goto LABEL_6;
    return;
  }
  if (!strcmp(key, "/apps/osso/inputmethod/int_kb_layout"))
  {
    if (priv->int_kb_layout)
      g_free(priv->int_kb_layout);
    priv->int_setting_changed = 1;
    priv->int_kb_layout = g_strdup(gconf_value_get_string(value));
    goto LABEL_5;
  }
  if (!strcmp(key, "/apps/osso/inputmethod/ext_kb_model"))
  {
    if (priv->ext_kb_model)
      g_free(priv->ext_kb_model);
    priv->ext_setting_changed = 1;
    priv->ext_kb_model = g_strdup(gconf_value_get_string(value));
  }
  else if (!strcmp(key, "/apps/osso/inputmethod/ext_kb_layout"))
  {
    if (priv->ext_kb_layout)
      g_free(priv->ext_kb_layout);
    priv->ext_setting_changed = 1;
    priv->ext_kb_layout = g_strdup(gconf_value_get_string(value));
  }
LABEL_4:
  if (priv->int_setting_changed)
    goto LABEL_5;
  if (priv->ext_setting_changed && !priv->timer)
LABEL_6:
    priv->timer = g_timeout_add(0x1F4, (GSourceFunc)hildon_im_keyboard_monitor_timeout_cb, monitor);
}

static void hildon_im_keyboard_monitor_language(HildonIMPlugin *plugin)
{
  HildonIMKeyboardMonitorPrivate *priv;
  g_return_if_fail(HILDON_IM_IS_KEYBOARD_MONITOR(plugin));

  priv = HILDON_IM_KEYBOARD_MONITOR(plugin)->priv;
  if (priv->language_switched)
  {
    priv->language_switched = FALSE;
  }
  else
  {
    hildon_im_ui_get_active_language(priv->ui);
    hildon_im_ui_get_language_setting(priv->ui, 0);
    hildon_im_ui_get_language_setting(priv->ui, 1);
  }
}

static void
hildon_im_keyboard_monitor_save_data(HildonIMPlugin *plugin)
{
  HildonIMKeyboardMonitorPrivate *priv;
  g_return_if_fail(HILDON_IM_KEYBOARD_MONITOR(plugin));
  priv = HILDON_IM_KEYBOARD_MONITOR_GET_PRIVATE(HILDON_IM_KEYBOARD_MONITOR(plugin));
}

const HildonIMPluginInfo *hildon_im_plugin_get_info(void)
{
  static const HildonIMPluginInfo info =
    {
      "(c) 2007 Nokia Corporation. All rights reserved",  /* description */
      "hildon_im_keyboard_monitor",                       /* name */
      "Keyboard Monitor Plugin",                          /* menu title */
      "hildon-input-method",                              /* gettext domain */
      FALSE,                                              /* visible in menu */
      TRUE,                                               /* cached */
      HILDON_IM_TYPE_PERSISTENT,                          /* UI type */
      HILDON_IM_GROUP_CUSTOM,                             /* group */
      HILDON_IM_DEFAULT_PLUGIN_PRIORITY,                  /* priority */
      NULL,                                               /* special character plugin */
      NULL,                                               /* help page */
      FALSE,                                              /* disable common UI buttons */
      0,                                                  /* plugin height */
      HILDON_IM_TRIGGER_NONE                              /* trigger */
    };
  return &info;
}

gchar **hildon_im_plugin_get_available_languages(gboolean *free)
{
  *free = 0;
  return NULL;
}

static GObject *hildon_im_keyboard_monitor_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_properties)
{
  GObjectClass *object_class;
  GObject *object;

  object_class = G_OBJECT_CLASS(hildon_im_keyboard_monitor_parent_class);
  object = object_class->constructor(type, n_construct_properties, construct_properties);
  return object;
}

static void hildon_im_keyboard_monitor_key_event(HildonIMPlugin *plugin, GdkEventType type, guint state, guint val, guint hardware_keycode)
{
  g_return_if_fail(HILDON_IM_IS_KEYBOARD_MONITOR(plugin));
  HildonIMKeyboardMonitorPrivate *priv = HILDON_IM_KEYBOARD_MONITOR(plugin)->priv;
  if (type != GDK_KEY_PRESS)
    goto LABEL_30;
  if (priv->banner)
  {
    gtk_widget_destroy(priv->banner);
    priv->banner = NULL;
  }
  if (state & (val == GDK_space))
    gconf_client_set_bool(priv->ui->client, "/system/osso/af/slide-open", priv->slide_open == 0, 0);
  if ((val == GDK_space) & (state >> 2))
  {
    if (priv->int_kb_layout && !strcmp("ru", priv->int_kb_layout))
    {
      gboolean level_shifted = priv->int_kb_level_shifted == 0;
      priv->int_kb_level_shifted = level_shifted;
	  const gchar *layout;
      if (level_shifted)
        layout = "Pyccknn"; //todo, cant figure out how to get the right Russian chars in here
      else
        layout = "Latin";
      const gchar *message = g_strdup_printf(dcgettext(0, "inpu_ib_quick_layout_switch", 5), layout);
      gconf_client_set_bool(
        priv->ui->client,
        "/apps/osso/inputmethod/int_kb_level_shifted",
        priv->int_kb_level_shifted,
        0);
      priv->banner = hildon_banner_show_information(GTK_WIDGET(&priv->ui->parent.bin.container.widget), NULL, message);
    }
    else
    {
      const gchar *language = hildon_im_ui_get_language_setting(priv->ui, (hildon_im_ui_get_active_language_index(priv->ui) == 0));
      if (language && *language)
      {
        gchar *language_desc = hildon_im_get_language_description(language);
        const gchar *message = g_strdup_printf(dcgettext(0, "inpu_ib_quick_layout_language", 5), language_desc);
        g_free(language_desc);
        priv->language_switched = 1;
        hildon_im_ui_set_active_language_index(priv->ui, (hildon_im_ui_get_active_language_index(priv->ui) == 0));
        priv->banner = hildon_banner_show_information(GTK_WIDGET(&priv->ui->parent.bin.container.widget), NULL, message);
      }
    }
  }
  else
  {
LABEL_30:
    if (type == GDK_KEY_RELEASE && val == GDK_Multi_key)
    {
      if (activate_special_plugin == TRUE)
      {
        hildon_im_ui_toggle_special_plugin(priv->ui);
        hildon_im_ui_set_level_sticky(priv->ui, 0);
        hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_LEVEL_UNSTICKY);
      }
      activate_special_plugin = val == GDK_Multi_key;
    }
    else
    {
      activate_special_plugin = val == GDK_Multi_key;
    }
  }
}

static void hildon_im_keyboard_monitor_gconf_notify_cb(GConfClient *client, guint cnxn_id, GConfEntry *entry, HildonIMPlugin *user_data)
{
  HildonIMKeyboardMonitor *monitor = HILDON_IM_KEYBOARD_MONITOR(user_data);
  GConfValue *value = gconf_entry_get_value(entry);
  if (value)
  {
    const char *key = gconf_entry_get_key(entry);
    if (!strcmp(key, "/system/osso/af/slide-open"))
    {
      if (value->type == GCONF_VALUE_BOOL)
      {
        monitor->priv->slide_open = gconf_value_get_bool(value);
      }
    }
    else if (!strcmp(key, "/system/osso/af/keyboard-attached") && value->type == GCONF_VALUE_BOOL)
    {
      monitor->priv->ext_setting_changed = TRUE;
      monitor->priv->keyboard_attached = gconf_value_get_bool(value);
      hildon_im_keyboard_monitor_update_settings(monitor);
    }
    hildon_im_keyboard_monitor_update_keyboard_state(monitor);
  }
}

static void hildon_im_keyboard_monitor_update_keyboard_state(HildonIMKeyboardMonitor *monitor)
{
  HildonIMKeyboardMonitorPrivate *priv = monitor->priv;
  gboolean b;
  if (priv->slide_open)
  {
    b = TRUE;
  }
  else
  {
    b = priv->keyboard_attached;
    if (priv->keyboard_attached)
      b = TRUE;
  }
  hildon_im_ui_set_keyboard_state(priv->ui, b);
}

static void hildon_im_keyboard_monitor_update_settings(HildonIMKeyboardMonitor *monitor)
{
  HildonIMKeyboardMonitorPrivate *priv;
  gchar *model;
  gchar *layout;
  GConfValue *shifted;
  GError *error;

  priv = monitor->priv;
  if (priv->int_setting_changed)
  {
    model = priv->int_kb_model;
    if (model)
    {
      layout = priv->int_kb_layout;
      if (layout)
      {
        hildon_im_xkb_set_map(model, layout, 0);
        hildon_im_xkb_set_rate(priv->int_kb_repeat_delay, priv->int_kb_repeat_interval, 0);
        shifted = gconf_client_get_default_from_schema(
                    priv->ui->client,
                    "/apps/osso/inputmethod/int_kb_level_shifted",
                    0);
        if (shifted)
        {
          if (shifted->type == GCONF_VALUE_BOOL)
            priv->int_kb_level_shifted = gconf_value_get_bool(shifted);
          else
            priv->int_kb_level_shifted = 1;
          gconf_value_free(shifted);
        }
        else
        {
          priv->int_kb_level_shifted = 1;
        }
        error = 0;
        if (gconf_client_get_bool(priv->ui->client, "/apps/osso/inputmethod/int_kb_level_shifted", &error) != priv->int_kb_level_shifted || error)
          gconf_client_set_bool(priv->ui->client, "/apps/osso/inputmethod/int_kb_level_shifted", priv->int_kb_level_shifted, 0);
        else
          hildon_im_keyboard_monitor_set_lock_level(monitor);
        if (error)
          g_error_free(error);
        priv->int_setting_changed = 0;
      }
    }
  }
  if (priv->ext_setting_changed && priv->keyboard_attached && priv->ext_kb_model)
  {
    if (priv->ext_kb_layout)
      priv->ext_setting_changed = 0;
  }
}

static void hildon_im_keyboard_monitor_set_lock_level(HildonIMKeyboardMonitor *monitor)
{
  HildonIMKeyboardMonitorPrivate *priv;
  gchar *layout;

  priv = monitor->priv;
  layout = priv->int_kb_layout;
  if (layout && !strcmp("ru", layout))
    hildon_im_ui_set_context_options(priv->ui, HILDON_IM_LOCK_LEVEL, priv->int_kb_level_shifted);
  else
    hildon_im_ui_set_context_options(priv->ui, HILDON_IM_LOCK_LEVEL, 0);
}

static gboolean hildon_im_keyboard_monitor_timeout_cb(HildonIMKeyboardMonitor *monitor)
{
  HildonIMKeyboardMonitor *m = HILDON_IM_KEYBOARD_MONITOR(monitor);
  HildonIMKeyboardMonitorPrivate *priv = m->priv;
  hildon_im_keyboard_monitor_update_settings(m);
  priv->timer = 0;
  return FALSE;
}
