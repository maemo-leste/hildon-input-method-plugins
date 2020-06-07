#include <hildon/hildon.h>

#include <imlayouts.h>
#include <hildon-im-plugin.h>
#include <hildon-im-vkbrenderer.h>
#include <hildon-im-widget-loader.h>

#include <string.h>

#include "hildon-im-keyboard-assistant-scv.h"

#define OSSO_AF_GCONF_DIR "/system/osso/af"
#define OSSO_AF_SLIDE_OPEN OSSO_AF_GCONF_DIR "/slide-open"

#define HILDON_IM_GCONF_INT_KB_MODEL  HILDON_IM_GCONF_DIR "/int_kb_model"
#define HILDON_IM_GCONF_INT_KB_LAYOUT HILDON_IM_GCONF_DIR "/int_kb_layout"

#define VKB_HEIGHT 288

enum
{
  HILDON_IM_KEYBOARD_ASSISTANT_SCV_PROP_UI = 1
};

struct _HildonIMKeyboardAssistantSCV
{
  GtkDialog parent;
  int field_A0;
};

struct _HildonIMKeyboardAssistantSCVClass
{
  GtkDialogClass parent_class;
};

typedef struct _HildonIMKeyboardAssistantSCVClass HildonIMKeyboardAssistantSCVClass;

struct _HildonIMKeyboardAssistantSCVPrivate
{
  HildonIMUI *ui;
  gboolean shift;
  gint input_mode;
  GtkWidget *hbox;
  GtkWidget *vkb_renderer;
  gint num_layouts;
  gint numeric_sub;
  gint layout_type;
  gchar *int_kb_layout;
  gulong key_press_event_id;
  gulong key_release_event_id;
  gulong delete_event_id;
  gchar *combining_input;
};

typedef struct _HildonIMKeyboardAssistantSCVPrivate HildonIMKeyboardAssistantSCVPrivate;

static void hildon_im_keyboard_assistant_scv_iface_init(HildonIMPluginIface *iface);

#define HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv) \
  ((HildonIMKeyboardAssistantSCVPrivate *)hildon_im_keyboard_assistant_scv_get_instance_private(scv))

G_DEFINE_DYNAMIC_TYPE_EXTENDED(
  HildonIMKeyboardAssistantSCV, hildon_im_keyboard_assistant_scv, GTK_TYPE_DIALOG, 0,
  G_ADD_PRIVATE_DYNAMIC(HildonIMKeyboardAssistantSCV);
  G_IMPLEMENT_INTERFACE_DYNAMIC(HILDON_IM_TYPE_PLUGIN,
                                hildon_im_keyboard_assistant_scv_iface_init);
);

void
module_init(GTypeModule *module)
{
  hildon_im_keyboard_assistant_scv_register_type(module);
}

HildonIMPlugin*
module_create (HildonIMUI *ui)
{
  return HILDON_IM_PLUGIN(hildon_im_keyboard_assistant_scv_new(ui));
}

void
module_exit(void)
{
  /* empty */
}

static void close_scv(HildonIMKeyboardAssistantSCV *scv);

static gchar *
hildon_im_keyboard_assistant_scv_get_layout_value(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantSCV *scv;
  HildonIMKeyboardAssistantSCVPrivate *priv;
  gchar *layout;
  GError *err = NULL;

  g_return_val_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(plugin), NULL);

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(plugin);
  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);

  if (!priv->ui->client)
    return NULL;

  layout = gconf_client_get_string(priv->ui->client,
                                   HILDON_IM_GCONF_INT_KB_LAYOUT, &err);

  if (err)
  {
    g_warning("Error occured when getting %s from GConf.",
              HILDON_IM_GCONF_INT_KB_LAYOUT);
    g_error_free(err);
  }
  else if (!layout)
  {
    g_warning("Can not get %s value from GConf. \n",
              HILDON_IM_GCONF_INT_KB_LAYOUT);
  }

  if (!layout)
    layout = g_strdup("us");

  return layout;
}

static void
hildon_im_keyboard_assistant_scv_language(HildonIMPlugin *plugin)
{
  gchar *layout;

  HildonIMKeyboardAssistantSCV *scv;
  HildonIMKeyboardAssistantSCVPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(plugin));

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(plugin);
  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);


  layout = g_utf8_strdown(
        hildon_im_keyboard_assistant_scv_get_layout_value(plugin), -1);

  if (layout && g_strcmp0(layout, priv->int_kb_layout))
  {
    gchar *coll;

    g_free(priv->int_kb_layout);
    priv->int_kb_layout = g_strdup(layout);
    coll = g_strconcat("/usr/share/scv_layouts", "/", layout, ".vkb", NULL);
    g_object_set(priv->vkb_renderer,
                 "collection", coll,
                 "layout", 1,
                 "sub", 0,
                 NULL);
    gtk_widget_queue_draw(priv->vkb_renderer);
    g_free(coll);
  }

  g_free(layout);
}

static gunichar
hildon_im_keyboard_assistant_scv_get_additional_char(gunichar utf8ch)
{
  gunichar unich;

  switch(utf8ch)
  {
    case 0x7E:
      unich = 0x303;
      break;
    case 0x5E:
      unich = 0x302;
      break;
    case 0x60:
      unich = 0x300;
      break;
    case 0x22:
      unich = 0x30B;
      break;
    case 0xAF:
      unich = 0x304;
      break;
    case 0xA8:
      unich = 0x308;
      break;
    case 0xB0:
    case 0x2DA:
      unich = 0x30A;
      break;
    case 0xB4:
      unich = 0x301;
      break;
    case 0x2C8:
      unich = 0x30D;
      break;
    case 0x2BD:
      unich = 0x314;
      break;
    case 0x2C7:
      unich = 0x30C;
      break;
    case 0xB8u:
      unich = 0x327;
      break;
    case 0x2D9:
      unich = 0x307;
      break;
    case 0x2D8:
      unich = 0x306;
      break;
    default:
      unich = 0;
  }

  return unich;
}

static void
hildon_im_keyboard_assistant_scv_key_event(HildonIMPlugin *plugin,
                                           GdkEventType type, guint state,
                                           guint val, guint hardware_keycode)
{
  HildonIMKeyboardAssistantSCV *scv;
  HildonIMKeyboardAssistantSCVPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(plugin));

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(plugin);
  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);


  if (!priv->vkb_renderer)
    g_warning("NULL VKB renderer for keyboard assistant SCV \n");

  if ((val == GDK_KEY_Shift_L || val == GDK_KEY_Shift_R) &&
      type == GDK_KEY_PRESS)
  {
    hildon_vkb_renderer_set_variance_layout(
          HILDON_VKB_RENDERER(priv->vkb_renderer));
    priv->shift = priv->shift == 0;
  }

  if (val == GDK_KEY_Multi_key && type == GDK_KEY_PRESS)
    goto out;

  if (type != GDK_KEY_RELEASE)
    return;

  switch (val)
  {
    case GDK_KEY_Tab:
    {
      hildon_im_ui_send_communication_message(priv->ui,
                                              HILDON_IM_CONTEXT_HANDLE_TAB);
      break;
    }
    case GDK_KEY_Return:
    case GDK_KEY_KP_Enter:
    {
      hildon_im_ui_send_communication_message(priv->ui,
                                              HILDON_IM_CONTEXT_HANDLE_ENTER);
      break;
    }
    case GDK_KEY_BackSpace:
    {
      hildon_im_ui_send_communication_message(
            priv->ui, HILDON_IM_CONTEXT_HANDLE_BACKSPACE);
      break;
    }
    case ' ':
    {
      if (priv->combining_input)
      {
        hildon_im_ui_send_utf8(priv->ui, priv->combining_input);
        hildon_im_ui_append_plugin_buffer(priv->ui, priv->combining_input);
        g_free(priv->combining_input);
        priv->combining_input = NULL;
      }
      else
      {
        hildon_im_ui_send_communication_message(priv->ui,
                                                HILDON_IM_CONTEXT_HANDLE_SPACE);
      }
      break;
    }
    default:
    {
      gunichar uc = gdk_keyval_to_unicode(val);
      gchar *utf8;
      char utf8buf[7];

      if (!g_unichar_isprint(uc))
        return;

      if (priv->shift)
        uc = g_unichar_toupper(uc);

      utf8buf[g_unichar_to_utf8(uc, utf8buf)] = '\0';

      utf8 = g_strdup(utf8buf);

      if (priv->combining_input)
      {
        gunichar unitext[2];
        gchar *utf8text;
        gchar *norm;
        gunichar utf8ch;

        unitext[0] = g_utf8_get_char(utf8buf);
        unitext[1] = hildon_im_keyboard_assistant_scv_get_additional_char(
              g_utf8_get_char(priv->combining_input));

        utf8text = g_ucs4_to_utf8(unitext, 2, NULL, NULL, NULL);
        norm = g_utf8_normalize(utf8text, -1, G_NORMALIZE_DEFAULT_COMPOSE);
        utf8ch = g_utf8_get_char(norm);

        if (strlen(utf8text) > strlen(norm) && strcmp(utf8text, norm))
        {
          cairo_t *cr = gdk_cairo_create(GTK_WIDGET(scv)->window);
          char tmp[7];
          gint len = g_unichar_to_utf8(utf8ch, tmp);
          PangoLayout *pango_layout = pango_cairo_create_layout(cr);
          int unk_glyphs;

          pango_layout_set_text(pango_layout, tmp, len);
          unk_glyphs = pango_layout_get_unknown_glyphs_count(pango_layout);
          g_object_unref(pango_layout);
          cairo_destroy(cr);

          if (!unk_glyphs)
          {
            g_free(utf8);
            utf8 = g_strdup(norm);
          }
        }

        g_free(utf8text);
        g_free(norm);
      }

      hildon_im_ui_send_utf8(priv->ui, utf8);
      hildon_im_ui_append_plugin_buffer(priv->ui, utf8);
      g_free(utf8);
    }
  }

out:
  close_scv(scv);
}

static void
hildon_im_keyboard_assistant_scv_input_mode_changed(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantSCV *scv;
  HildonIMKeyboardAssistantSCVPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(plugin));

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(plugin);
  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);

  priv->input_mode = hildon_im_ui_get_current_input_mode(priv->ui);

  if (priv->vkb_renderer)
  {
    g_object_set(priv->vkb_renderer, "mode",
                 KEY_TYPE_ALPHA | KEY_TYPE_NUMERIC | KEY_TYPE_HEXA |
                 KEY_TYPE_TELE | KEY_TYPE_SPECIAL | KEY_TYPE_DEAD |
                 KEY_TYPE_WHITESPACE | KEY_TYPE_TAB,
                 NULL);
  }
}

static gboolean
on_key_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
  hildon_im_keyboard_assistant_scv_key_event(HILDON_IM_PLUGIN(widget),
                                             event->type,
                                             event->state,
                                             event->keyval,
                                             event->hardware_keycode);

  return FALSE;
}

static gboolean
on_delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  g_return_val_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(widget), FALSE);

  close_scv(HILDON_IM_KEYBOARD_ASSISTANT_SCV(widget));

  return TRUE;
}

static void
hildon_im_keyboard_assistant_scv_enable(HildonIMPlugin *plugin, gboolean init)
{
  gboolean old_shift;
  gboolean shift = FALSE;
  gint numeric_sub;
  HildonVKBRendererLayoutInfo *layout_info = NULL;

  HildonIMKeyboardAssistantSCV *scv;
  HildonIMKeyboardAssistantSCVPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(plugin));

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(plugin);
  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);

  hildon_im_ui_parse_rc_file(priv->ui, "/usr/share/hildon-input-method/himrc");
  hildon_im_ui_send_communication_message(
        priv->ui, HILDON_IM_CONTEXT_CONFIRM_SENTENCE_START);

  priv->combining_input = NULL;
  priv->input_mode = hildon_im_ui_get_current_input_mode(priv->ui);

  hildon_im_keyboard_assistant_scv_language(plugin);
  hildon_im_keyboard_assistant_scv_input_mode_changed(plugin);

  g_object_get(priv->vkb_renderer,
               "subs", &layout_info,
               "sub", &numeric_sub,
               NULL);

  if (layout_info)
  {
    gint height = VKB_HEIGHT;

    priv->numeric_sub = numeric_sub;
    priv->num_layouts = layout_info->num_layouts;
    priv->layout_type = layout_info->type[numeric_sub];

    if (layout_info->num_rows > 0)
      height = 72 * layout_info->num_rows;

    gtk_widget_set_size_request(
          GTK_WIDGET(priv->vkb_renderer), -1, height);

    layout_info_free(layout_info);
  }

  old_shift = priv->shift;

  if (hildon_im_ui_get_shift_sticky(priv->ui) ||
      hildon_im_ui_get_shift_locked(priv->ui))
  {
    shift = TRUE;
  }

  priv->shift = shift;

  if (shift != old_shift)
  {
    g_object_get(priv->vkb_renderer, "sub", &numeric_sub, NULL);

    if (numeric_sub != hildon_vkb_renderer_get_numeric_sub(
          HILDON_VKB_RENDERER(priv->vkb_renderer)))
    {
      g_object_set(priv->vkb_renderer,
                   "sub", hildon_vkb_renderer_get_numeric_sub(
                     HILDON_VKB_RENDERER(priv->vkb_renderer)), NULL);
      gtk_widget_queue_draw(priv->vkb_renderer);
    }

    if (priv->shift )
    {
      hildon_vkb_renderer_set_variance_layout(
            HILDON_VKB_RENDERER(priv->vkb_renderer));
    }
    else
      g_object_set(priv->vkb_renderer, "sub", 0, NULL);
  }

  priv->key_press_event_id =
      g_signal_connect(G_OBJECT(plugin), "key-press-event",
                       G_CALLBACK(on_key_event_cb), NULL);
  priv->key_release_event_id =
      g_signal_connect(G_OBJECT(plugin), "key-release-event",
                       G_CALLBACK(on_key_event_cb), NULL);
  priv->delete_event_id =
      g_signal_connect(G_OBJECT(plugin), "delete-event",
                       G_CALLBACK(on_delete_event_cb), NULL);

  gtk_widget_show_all(GTK_WIDGET(scv));
}

static void
hildon_im_keyboard_assistant_scv_disable(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantSCV *scv;
  HildonIMKeyboardAssistantSCVPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(plugin));

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(plugin);
  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);

  g_free(priv->combining_input);
  priv->combining_input = NULL;

  priv->shift = FALSE;

  if (g_signal_handler_is_connected(G_OBJECT(plugin), priv->key_press_event_id))
    g_signal_handler_disconnect(G_OBJECT(plugin), priv->key_press_event_id);

  if (g_signal_handler_is_connected(G_OBJECT(plugin),
                                    priv->key_release_event_id))
  {
    g_signal_handler_disconnect(G_OBJECT(plugin), priv->key_release_event_id);
  }

  if (g_signal_handler_is_connected(G_OBJECT(plugin), priv->delete_event_id))
    g_signal_handler_disconnect(G_OBJECT(plugin), priv->delete_event_id);

  gtk_widget_hide(GTK_WIDGET(scv));
}

static void
hildon_im_keyboard_assistant_scv_clear(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantSCV *scv;
  HildonIMKeyboardAssistantSCVPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(plugin));

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(plugin);
  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);

  hildon_im_ui_send_communication_message(
        priv->ui, HILDON_IM_CONTEXT_CONFIRM_SENTENCE_START);
  hildon_im_ui_clear_plugin_buffer(priv->ui);
}

static void
close_scv(HildonIMKeyboardAssistantSCV *scv)
{
  g_return_if_fail(scv != NULL);

  gtk_widget_hide(GTK_WIDGET(scv));
  hildon_im_ui_restore_previous_mode(
        HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv)->ui);
}

static void
hildon_im_keyboard_assistant_scv_client_widget_changed(HildonIMPlugin *plugin)
{
  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(plugin));

  close_scv(HILDON_IM_KEYBOARD_ASSISTANT_SCV(plugin));
}

static void
hildon_im_keyboard_assistant_scv_save_data(HildonIMPlugin *plugin)
{
}

static void
hildon_im_keyboard_assistant_scv_language_settings_changed(
    HildonIMPlugin *plugin, gint index)
{
  hildon_im_keyboard_assistant_scv_language(plugin);
}

static void
hildon_im_keyboard_assistant_scv_fullscreen(HildonIMPlugin *plugin,
                                            gboolean fullscreen)
{
}

static void
hildon_im_keyboard_assistant_scv_settings_changed(HildonIMPlugin *plugin,
                                                  const gchar *key,
                                                  const GConfValue *value)
{
  HildonIMKeyboardAssistantSCV *scv;
  HildonIMKeyboardAssistantSCVPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(plugin));
  g_return_if_fail(key != NULL);

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(plugin);
  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);

  if (!strcmp(key, OSSO_AF_SLIDE_OPEN))
  {
    if (value->type == GCONF_VALUE_BOOL && !gconf_value_get_bool(value))
      close_scv(scv);
  }
  else if (!strcmp(key, HILDON_IM_GCONF_INT_KB_MODEL))
  {
    hildon_im_ui_send_communication_message(
          priv->ui, HILDON_IM_CONTEXT_CONFIRM_SENTENCE_START);
  }
}

static void
hildon_im_keyboard_assistant_scv_iface_init(HildonIMPluginIface *iface)
{
  iface->key_event = hildon_im_keyboard_assistant_scv_key_event;
  iface->enable = hildon_im_keyboard_assistant_scv_enable;
  iface->disable = hildon_im_keyboard_assistant_scv_disable;
  iface->clear = hildon_im_keyboard_assistant_scv_clear;
  iface->input_mode_changed =
      hildon_im_keyboard_assistant_scv_input_mode_changed;
  iface->client_widget_changed =
      hildon_im_keyboard_assistant_scv_client_widget_changed;
  iface->save_data = hildon_im_keyboard_assistant_scv_save_data;
  iface->language = hildon_im_keyboard_assistant_scv_language;
  iface->language_settings_changed =
      hildon_im_keyboard_assistant_scv_language_settings_changed;
  iface->fullscreen = hildon_im_keyboard_assistant_scv_fullscreen;
  iface->settings_changed = hildon_im_keyboard_assistant_scv_settings_changed;
}

static void
hildon_im_keyboard_assistant_scv_finalize(GObject *object)
{
  HildonIMKeyboardAssistantSCV *scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(object);
  HildonIMKeyboardAssistantSCVPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);
  GObjectClass *object_class;

  g_free(priv->int_kb_layout);
  priv->int_kb_layout = NULL;

  g_free(priv->combining_input);
  priv->combining_input = NULL;

  object_class = G_OBJECT_CLASS(hildon_im_keyboard_assistant_scv_parent_class);

  if (object_class->finalize)
      object_class->finalize(object);
}

static void
hildon_im_keyboard_assistant_scv_set_property(GObject *object, guint prop_id,
                                              const GValue *value,
                                              GParamSpec *pspec)
{
  HildonIMKeyboardAssistantSCV *scv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(object));

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(object);

  switch (prop_id)
  {
    case HILDON_IM_KEYBOARD_ASSISTANT_SCV_PROP_UI:
      HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv)->ui =
          (HildonIMUI *)g_value_get_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void
hildon_im_keyboard_assistant_scv_get_property(GObject *object, guint prop_id,
                                              GValue *value, GParamSpec *pspec)
{
  HildonIMKeyboardAssistantSCV *scv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(object));

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(object);

  switch (prop_id)
  {
    case HILDON_IM_KEYBOARD_ASSISTANT_SCV_PROP_UI:
      g_value_set_object(
            value, HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv)->ui);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void
hildon_im_keyboard_assistant_scv_class_init(
    HildonIMKeyboardAssistantSCVClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = hildon_im_keyboard_assistant_scv_set_property;
  object_class->get_property = hildon_im_keyboard_assistant_scv_get_property;
  object_class->finalize = hildon_im_keyboard_assistant_scv_finalize;

  g_object_class_install_property(
        object_class, HILDON_IM_KEYBOARD_ASSISTANT_SCV_PROP_UI,
        g_param_spec_object(
          "UI",
          "UI",
          "Keyboard that uses plugin",
          HILDON_IM_TYPE_UI,
          G_PARAM_CONSTRUCT_ONLY|G_PARAM_WRITABLE|G_PARAM_READABLE));
}

static void
hildon_im_keyboard_assistant_scv_class_finalize(
    HildonIMKeyboardAssistantSCVClass *klass)
{
}

static void
renderer_character_send(HildonVKBRenderer *vkb, gchar *input, gboolean unk,
                        gpointer data)
{
  HildonIMKeyboardAssistantSCV *scv;
  HildonIMKeyboardAssistantSCVPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(data));

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(data);
  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);

  if (*input)
    hildon_im_ui_send_utf8(priv->ui, input);
  else
    hildon_im_ui_send_utf8(priv->ui, "\t");

  hildon_im_ui_append_plugin_buffer(priv->ui, input);
  hildon_im_ui_set_shift_sticky(priv->ui, FALSE);
  hildon_im_ui_send_communication_message(priv->ui,
                                          HILDON_IM_CONTEXT_SHIFT_UNSTICKY);
  hildon_im_ui_set_level_sticky(priv->ui, FALSE);
  hildon_im_ui_send_communication_message(priv->ui,
                                          HILDON_IM_CONTEXT_LEVEL_UNSTICKY);
  close_scv(scv);
}

static void
illegal_input(HildonVKBRenderer *vkb, gchar *input, gpointer data)
{
  HildonIMKeyboardAssistantSCV *scv;
  HildonIMKeyboardAssistantSCVPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(data));

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(data);
  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);

  hildon_im_ui_play_sound(priv->ui, HILDON_IM_ILLEGAL_INPUT_SOUND);
}

static void
combining_input(HildonVKBRenderer *vkb, gchar *input, gpointer data)
{
  HildonIMKeyboardAssistantSCV *scv;
  HildonIMKeyboardAssistantSCVPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(data));

  scv = HILDON_IM_KEYBOARD_ASSISTANT_SCV(data);
  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);

  if (input && *input)
  {
    g_free(priv->combining_input);
    priv->combining_input = g_strdup(input);
  }
  else
  {
    g_free(priv->combining_input);
    priv->combining_input = NULL;
  }
}

static void
hildon_im_keyboard_assistant_scv_init(HildonIMKeyboardAssistantSCV *scv)
{
  HildonIMKeyboardAssistantSCVPrivate *priv;
  GtkRequisition dimension;
  HildonVKBRendererLayoutInfo *layout_info = NULL;
  gint height = VKB_HEIGHT;

  dimension.height = VKB_HEIGHT;
  dimension.width = gdk_screen_get_width(gtk_widget_get_screen(GTK_WIDGET(scv)));

  g_return_if_fail(HILDON_IM_IS_KEYBOARD_ASSISTANT_SCV(scv));

  priv = HILDON_IM_KEYBOARD_ASSISTANT_SCV_GET_PRIVATE(scv);

  priv->hbox = gtk_hbox_new(FALSE, 0);
  priv->vkb_renderer = hildon_im_widget_load("vkbrenderer",
                                             "vkb_renderer",
                                             "dimension",
                                             &dimension,
                                             "repeat_interval",
                                             0,
                                             NULL);
  gtk_window_set_title(GTK_WINDOW(scv), "");
  gtk_window_set_modal(GTK_WINDOW(scv), TRUE);

  g_return_if_fail(priv->vkb_renderer != NULL);

  gtk_widget_set_name(priv->vkb_renderer, "osso-im-scv-renderer");
  g_object_set(priv->vkb_renderer, "style_normal", "hildon-scv-button", NULL);
  g_object_set(priv->vkb_renderer, "style_special", "hildon-scv-button", NULL);
  g_object_set(priv->vkb_renderer, "style_tab", "hildon-scv-tab-button", NULL);

  g_signal_connect(priv->vkb_renderer, "input",
                   G_CALLBACK(renderer_character_send), scv);
  g_signal_connect(priv->vkb_renderer, "illegal_input",
                   G_CALLBACK(illegal_input), scv);
  g_signal_connect(priv->vkb_renderer, "combining_input",
                   G_CALLBACK(combining_input), scv);

  gtk_box_pack_start(GTK_BOX(priv->hbox), priv->vkb_renderer, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(scv)->vbox), priv->hbox, TRUE, TRUE, 0);

  g_object_get(priv->vkb_renderer, "subs", &layout_info, NULL);

  if (layout_info && layout_info->num_rows)
  {
    height = 72 * layout_info->num_rows;
    layout_info_free(layout_info);
  }

  gtk_widget_set_size_request(
        GTK_WIDGET(priv->vkb_renderer), -1, height);

  priv->numeric_sub = 0;
  priv->combining_input = NULL;
  priv->key_press_event_id = 0;
  priv->key_release_event_id = 0;
  priv->int_kb_layout = g_strdup("");

  if (priv->ui)
    priv->input_mode = hildon_im_ui_get_current_input_mode(priv->ui);
}

HildonIMKeyboardAssistantSCV *
hildon_im_keyboard_assistant_scv_new(HildonIMUI *ui)
{
  return g_object_new(HILDON_IM_TYPE_KEYBOARD_ASSISTANT_SCV, "type", 0,
                      "UI", ui, NULL);
}

gchar **
hildon_im_plugin_get_available_languages(gboolean *free)
{
  *free = FALSE;

  return NULL;
}

HildonIMPluginInfo *
hildon_im_plugin_get_info()
{
  static HildonIMPluginInfo info =
  {
    "(c) 2007 Nokia Corporation. All rights reserved", /* description */
    "hildon_keyboard_assistant_scv",                   /* name */
    "On Screen SCV",                                   /* menu title */
    NULL,                                              /* gettext domain */
    FALSE,                                             /* visible in menu */
    FALSE,                                             /* cached */
    HILDON_IM_TYPE_SPECIAL_STANDALONE,                 /* UI type */
    HILDON_IM_GROUP_LATIN,                             /* group */
    -1,                                                /* priority */
    NULL,                                              /* special character plugin */
    NULL,                                              /* help page */
    TRUE,                                              /* disable common UI buttons */
    0,                                                 /* plugin height */
    HILDON_IM_TRIGGER_NONE                             /* trigger */
  };

  return &info;
}
