/**
   @file hildon-keyboard-assistant.c

   This file is part of hildon-input-method-plugins.

   This file may contain parts derived by disassembling of binaries under
   Nokia's copyright, see http://tablets-dev.nokia.com/maemo-dev-env-downloads.php

   The original licensing conditions apply to all those derived parts as well
   and you accept those by using this file.
*/

#include <string.h>
#include <assert.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkvbox.h>

#include <hildon-im-widget-loader.h>
#include <hildon-im-plugin.h>
#include <hildon-im-ui.h>
#include <hildon-im-languages.h>
#include <hildon-im-common.h>

#include "hildon-keyboard-assistant.h"
#include "hildon-im-western-plugin-common.h"
#include "hildon-im-word-completer.h"

struct _HildonIMKeyboardAssistantPrivate
{
  HildonIMUI *ui;
  const gchar *lang[2];
  int language_index;
  HildonGtkInputMode input_mode;
  int i6;
  int i7;
  int i8;
  HildonIMWordCompleter *hwc;
  int insert_space_after_word;
  gboolean word_completion;
  int i12;
  gboolean b;
  gint display_after_entering;
  gchar *word;
  gchar *predicted_suffix;
  gchar *str1;
  gchar *str2;
};

GType hildon_im_type_keyboard_assistant = 0;
GtkWidgetClass *hildon_im_keyboard_assistant_parent_class = NULL;

static void 
hildon_im_keyboard_assistant_class_init(HildonIMKeyboardAssistantClass *klass);
static void hildon_im_keyboard_assistant_init(HildonIMKeyboardAssistant *assistant);
static void hildon_im_keyboard_assistant_iface_init(HildonIMPluginIface *iface);
static void hildon_im_keyboard_assistant_set_property(GObject *object,guint prop_id,const GValue *value,GParamSpec *pspec);
static void hildon_im_keyboard_assistant_get_property (GObject *object,guint prop_id,GValue *value,GParamSpec *pspec);
static void hildon_im_keyboard_assistant_finalize(GObject *obj);
static void hildon_im_keyboard_assistant_enable(HildonIMPlugin *plugin, gboolean init);
static void hildon_im_keyboard_assistant_disable(HildonIMPlugin *plugin);
static void hildon_im_keyboard_assistant_client_widget_changed(HildonIMPlugin *plugin);
static void hildon_im_keyboard_assistant_save_data(HildonIMPlugin *plugin);
static void hildon_im_keyboard_assistant_language(HildonIMPlugin *plugin);
static GObject *hildon_im_keyboard_assistant_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_properties);
static void hildon_im_keyboard_assistant_key_event(HildonIMPlugin *plugin, GdkEventType type, guint state, guint val, guint hardware_keycode);
static void hildon_im_keyboard_assistant_notify_cb(GConfClient *client, guint cnxn_id, GConfEntry *entry, HildonIMPlugin *user_data);
static void hildon_im_keyboard_assistant_read_settings(HildonIMKeyboardAssistant *assistant);
static void hildon_im_keyboard_assistant_surrounding_received(HildonIMPlugin *plugin, const gchar *surrounding, gint offset);
static void hildon_im_keyboard_assistant_list_free(gpointer data, gpointer user_data);
static void hildon_im_keyboard_assistant_input_mode_changed(HildonIMPlugin *plugin);
static void hildon_im_keyboard_assistant_character_autocase(HildonIMPlugin *plugin);
static void hildon_im_keyboard_assistant_reset(HildonIMPlugin *plugin);
static void hildon_im_keyboard_assistant_transition(HildonIMPlugin *plugin, gboolean from);
static void hildon_im_keyboard_assistant_input_mode_clear(HildonIMPlugin *plugin);
static void hildon_im_keyboard_assistant_client_settings_changed(HildonIMPlugin *plugin, const gchar *key, const GConfValue *value);
static void hildon_im_keyboard_assistant_preedit_committed(HildonIMPlugin *plugin, const gchar *committed_preedit);
static void hildon_im_keyboard_assistant_keyboard_state_changed(HildonIMPlugin *plugin);
static void hildon_im_keyboard_assistant_language_settings_changed(HildonIMPlugin *plugin, gint index);

GType
hildon_im_keyboard_assistant_get_type (void)
{
  return hildon_im_type_keyboard_assistant;
}

GObject *hildon_im_keyboard_assistant_new(HildonIMUI *ui)
{
  return g_object_new(HILDON_IM_TYPE_KEYBOARD_ASSISTANT,
                      HILDON_IM_PROP_UI_DESCRIPTION, ui, NULL);
}

HildonIMPlugin*
module_create (HildonIMUI *ui)
{
  return HILDON_IM_PLUGIN(hildon_im_keyboard_assistant_new(ui));
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
    sizeof(HildonIMKeyboardAssistantClass),
    NULL, /* base_init */
    NULL, /* base_finalize */
    (GClassInitFunc) hildon_im_keyboard_assistant_class_init,
    NULL, /* class_finalize */
    NULL, /* class_data */
    sizeof(HildonIMKeyboardAssistant),
    0, /* n_preallocs */
    (GInstanceInitFunc) hildon_im_keyboard_assistant_init,
    NULL
  };

  static const GInterfaceInfo plugin_info = {
    (GInterfaceInitFunc) hildon_im_keyboard_assistant_iface_init,
    NULL, /* interface_finalize */
    NULL, /* interface_data */
  };

  hildon_im_type_keyboard_assistant =
          g_type_module_register_type(module,
                                      GTK_TYPE_CONTAINER,
                                      "HildonIMKeyboardAssistant",
                                      &type_info,
                                      0);

  g_type_module_add_interface(module,
                              HILDON_IM_TYPE_KEYBOARD_ASSISTANT,
                              HILDON_IM_TYPE_PLUGIN,
                              &plugin_info);
}

static void 
hildon_im_keyboard_assistant_class_init(HildonIMKeyboardAssistantClass *klass)
{
  GObjectClass *object_class;

  hildon_im_keyboard_assistant_parent_class = g_type_class_peek_parent(klass);
  g_type_class_add_private(klass, sizeof(HildonIMKeyboardAssistantPrivate));

  object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = hildon_im_keyboard_assistant_set_property;
  object_class->get_property = hildon_im_keyboard_assistant_get_property;
  object_class->constructor = hildon_im_keyboard_assistant_constructor;
  object_class->finalize = hildon_im_keyboard_assistant_finalize;

  g_object_class_install_property(
        object_class, HILDON_IM_PROP_UI,
        g_param_spec_object(
          HILDON_IM_PROP_UI_DESCRIPTION,
          HILDON_IM_PROP_UI_DESCRIPTION,
          "UI that uses plugin",
          HILDON_IM_TYPE_UI,
          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
hildon_im_keyboard_assistant_iface_init(HildonIMPluginIface *iface)
{
  iface->keyboard_state_changed =
      hildon_im_keyboard_assistant_keyboard_state_changed;
  iface->disable = hildon_im_keyboard_assistant_disable;
  iface->enable = hildon_im_keyboard_assistant_enable;
  iface->language_settings_changed =
      hildon_im_keyboard_assistant_language_settings_changed;
  iface->language = hildon_im_keyboard_assistant_language;
  iface->save_data = hildon_im_keyboard_assistant_save_data;
  iface->client_widget_changed =
      hildon_im_keyboard_assistant_client_widget_changed;
  iface->settings_changed =
      hildon_im_keyboard_assistant_client_settings_changed;
  iface->input_mode_changed = hildon_im_keyboard_assistant_input_mode_changed;
  iface->key_event = hildon_im_keyboard_assistant_key_event;
  iface->clear = hildon_im_keyboard_assistant_input_mode_clear;
  iface->character_autocase = hildon_im_keyboard_assistant_character_autocase;
  iface->transition = hildon_im_keyboard_assistant_transition;
  iface->tab = hildon_im_keyboard_assistant_reset;
  iface->surrounding_received =
      hildon_im_keyboard_assistant_surrounding_received;
  iface->preedit_committed = hildon_im_keyboard_assistant_preedit_committed;
  iface->backspace = hildon_im_keyboard_assistant_reset;
  iface->enter = hildon_im_keyboard_assistant_reset;
}

static void
hildon_im_keyboard_assistant_init(HildonIMKeyboardAssistant *assistant)
{
  assistant->priv = HILDON_IM_KEYBOARD_ASSISTANT_GET_PRIVATE(
        HILDON_IM_KEYBOARD_ASSISTANT(assistant));
  assistant->priv->hwc = NULL;
}

static void
hildon_im_keyboard_assistant_enable(HildonIMPlugin *plugin, gboolean init)
{
  HildonIMKeyboardAssistantPrivate *priv;

  priv = HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;
  gconf_client_notify_add(
        priv->ui->client,"/system/osso/af/slide-open",
        (GConfClientNotifyFunc)hildon_im_keyboard_assistant_notify_cb,
        plugin, 0, 0);
  priv->input_mode = hildon_im_ui_get_current_input_mode(priv->ui);

  if (priv->insert_space_after_word)
  {
    hildon_im_ui_send_communication_message(
          priv->ui, HILDON_IM_CONTEXT_SPACE_AFTER_COMMIT);
  }
  else
  {
    hildon_im_ui_send_communication_message(
          priv->ui, HILDON_IM_CONTEXT_NO_SPACE_AFTER_COMMIT);
  }

  gtk_widget_hide(&HILDON_IM_KEYBOARD_ASSISTANT(plugin)->parent.box.container.widget);
}

static void
hildon_im_keyboard_assistant_get_property (GObject *object,
                                    guint prop_id,
                                    GValue *value,
                                    GParamSpec *pspec)
{
  HildonIMKeyboardAssistantPrivate *priv;

  priv = HILDON_IM_KEYBOARD_ASSISTANT(object)->priv;

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
hildon_im_keyboard_assistant_set_property(GObject *object,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  HildonIMKeyboardAssistantPrivate *priv;

  priv = HILDON_IM_KEYBOARD_ASSISTANT(object)->priv;

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

static void
hildon_im_keyboard_assistant_finalize(GObject *object)
{
  HildonIMKeyboardAssistantPrivate *priv;
  priv = HILDON_IM_KEYBOARD_ASSISTANT(object)->priv;

  if (G_OBJECT_CLASS(hildon_im_keyboard_assistant_parent_class)->finalize)
    G_OBJECT_CLASS(hildon_im_keyboard_assistant_parent_class)->finalize(object);

  if (priv->hwc)
  {
    g_object_unref(priv->hwc);
    priv->hwc = NULL;
  }

  g_free(priv->str1);
  g_free(priv->str2);
}

static void
hildon_im_keyboard_assistant_disable(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantPrivate *priv;

  priv = HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;
  hildon_im_ui_send_communication_message(priv->ui,
                                          HILDON_IM_CONTEXT_CANCEL_PREEDIT);
  hildon_im_keyboard_assistant_save_data(plugin);
  gtk_widget_hide(GTK_WIDGET(plugin));
}

static void
hildon_im_keyboard_assistant_client_widget_changed(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;

  hildon_im_ui_send_communication_message(
        priv->ui, HILDON_IM_CONTEXT_CONFIRM_SENTENCE_START);

  if (priv->insert_space_after_word)
  {
    hildon_im_ui_send_communication_message(
          priv->ui, HILDON_IM_CONTEXT_SPACE_AFTER_COMMIT);
  }
  else
  {
    hildon_im_ui_send_communication_message(
          priv->ui, HILDON_IM_CONTEXT_NO_SPACE_AFTER_COMMIT);
  }

  if (priv->str2)
  {
    hildon_im_word_completer_hit_word(priv->hwc, priv->str2, 1);
    g_free(priv->str1);
    priv->str1 = g_strdup(priv->str2);
    g_free(priv->str2);
    priv->str2 = NULL;
  }
}

static void
hildon_im_keyboard_assistant_language(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;

  hildon_im_keyboard_assistant_language_settings_changed(plugin,
                                                         priv->language_index);
}

static void
hildon_im_keyboard_assistant_save_data(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantPrivate *priv;
  priv = HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;;
  hildon_im_word_completer_save_data(priv->hwc);
}

const HildonIMPluginInfo *
hildon_im_plugin_get_info()
{
  static const HildonIMPluginInfo info =
  {
    "(c) 2007 Nokia Corporation. All rights reserved",  /* description */
    "hildon_keyboard_assistant",                        /* name */
    "Word Completion Plugin",                           /* menu title */
    "hildon-input-method",                              /* gettext domain */
    TRUE,                                               /* visible in menu */
    TRUE,                                               /* cached */
    HILDON_IM_TYPE_DEFAULT,                             /* UI type */
    HILDON_IM_GROUP_LATIN,                              /* group */
    -1,                                                 /* priority */
    "hildon_keyboard_assistant_scv",                    /* special character plugin */
    NULL,                                               /* help page */
    TRUE,                                               /* disable common UI buttons */
    0,                                                  /* plugin height */
    HILDON_IM_TRIGGER_KEYBOARD                          /* trigger */
  };

  return &info;
}

gchar **
hildon_im_plugin_get_available_languages(gboolean *free)
{
  *free = FALSE;
  return NULL;
}

static GObject *
hildon_im_keyboard_assistant_constructor(GType type,
                                         guint n_construct_properties,
                                         GObjectConstructParam *construct_properties)
{
  GObjectClass *object_class;
  GObject *object;

  object_class = G_OBJECT_CLASS(hildon_im_keyboard_assistant_parent_class);
  object = object_class->constructor(type, n_construct_properties,
                                     construct_properties);

  HildonIMKeyboardAssistantPrivate *priv;
  priv = HILDON_IM_KEYBOARD_ASSISTANT(object)->priv;
  priv->hwc = hildon_im_word_completer_new();
  g_object_set(priv->hwc, "max_candidates", 1,
               "min_candidate_suffix_length", 2,
               NULL);
  priv->lang[0] = NULL;
  priv->lang[1] = NULL;
  priv->language_index = 0;
  hildon_im_keyboard_assistant_read_settings(
        HILDON_IM_KEYBOARD_ASSISTANT(object));
  priv->str2 = NULL;
  priv->word = NULL;
  priv->predicted_suffix = NULL;
  priv->str1 = NULL;
  priv->b = TRUE;
  priv->input_mode = hildon_im_ui_get_current_input_mode(priv->ui);

  return object;
}

static gboolean
entry_allowed(HildonIMKeyboardAssistant *assistant)
{
  HildonIMKeyboardAssistantPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT(assistant)->priv;

  if (priv->insert_space_after_word &&
      (priv->input_mode & HILDON_GTK_INPUT_MODE_DICTIONARY))
  {
    return !(priv->input_mode & HILDON_GTK_INPUT_MODE_INVISIBLE);
  }

  return FALSE;
}

static void
hildon_im_keyboard_assistant_key_event(HildonIMPlugin *plugin,
                                       GdkEventType type, guint state,
                                       guint val, guint hardware_keycode)
{
  HildonIMKeyboardAssistantPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;

  if (entry_allowed(HILDON_IM_KEYBOARD_ASSISTANT(plugin)) &&
      type == GDK_KEY_PRESS && !(state & GDK_CONTROL_MASK))
  {
    if (val == GDK_KEY_Return || val == GDK_KP_Enter)
      priv->i12 = FALSE;
    else if (g_unichar_isprint(gdk_keyval_to_unicode(val)))
      priv->i12 = TRUE;

    hildon_im_ui_send_communication_message(
          priv->ui, HILDON_IM_CONTEXT_REQUEST_SURROUNDING);

  }
}

static void
hildon_im_keyboard_assistant_notify_cb(GConfClient *client, guint cnxn_id,
                                       GConfEntry *entry,
                                       HildonIMPlugin *user_data)
{
  HildonIMKeyboardAssistant *assistant =
      HILDON_IM_KEYBOARD_ASSISTANT(user_data);
  HildonIMKeyboardAssistantPrivate *priv = assistant->priv;
  GConfValue *value = gconf_entry_get_value(entry);

  if (value)
  {
    const char *key = gconf_entry_get_key(entry);

    if (!strcmp(key, "/system/osso/af/slide-open") &&
        value->type == GCONF_VALUE_BOOL && !gconf_value_get_bool(value))
    {
      hildon_im_keyboard_assistant_reset(HILDON_IM_PLUGIN(assistant));
    }

    if (hildon_im_word_completer_is_interesting_key(priv->hwc, key))
      hildon_im_word_completer_configure(priv->hwc, priv->ui);
  }
}

static void
hildon_im_keyboard_assistant_read_settings(HildonIMKeyboardAssistant *assistant)
{
  HildonIMKeyboardAssistantPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT(assistant)->priv;
  guint lang_index;
  gchar *s;
  GConfValue *v;

  v = gconf_client_get(priv->ui->client,
                       "/apps/osso/inputmethod/display_after_entering", NULL);

  if (v)
  {
    priv->display_after_entering = gconf_value_get_int(v);
    gconf_value_free(v);
  }

  priv->lang[0] = hildon_im_ui_get_language_setting(priv->ui, 0);
  priv->lang[1] = hildon_im_ui_get_language_setting(priv->ui, 1);

  lang_index = hildon_im_ui_get_active_language_index(priv->ui);
  priv->language_index = lang_index;

  if (lang_index > 1)
    priv->language_index = 0;

  if (priv->lang[priv->language_index])
  {
    s = g_strdup_printf(
          "/apps/osso/inputmethod/hildon-im-languages/%s/word-completion",
          priv->lang[priv->language_index]);
    v = gconf_client_get(priv->ui->client, s, NULL);

    if (v)
    {
      priv->word_completion = gconf_value_get_bool(v);
      gconf_value_free(v);
    }
    else
      priv->word_completion = FALSE;

    g_free(s);

    s = g_strdup_printf(
          "/apps/osso/inputmethod/hildon-im-languages/%s/auto-capitalisation",
          priv->lang[priv->language_index]);
    v = gconf_client_get(priv->ui->client, s, NULL);

    if (v)
    {
      hildon_im_ui_set_context_options(priv->ui, HILDON_IM_AUTOCASE,
                                       gconf_value_get_bool(v));
      hildon_im_ui_send_communication_message(
            priv->ui, HILDON_IM_CONTEXT_CONFIRM_SENTENCE_START);
      gconf_value_free(v);
    }

    g_free(s);

    s = g_strdup_printf(
          "/apps/osso/inputmethod/hildon-im-languages/%s/insert-space-after-word",
          priv->lang[priv->language_index]);
    v = gconf_client_get(priv->ui->client, s, NULL);

    if (v)
    {
      priv->insert_space_after_word = gconf_value_get_bool(v);
      gconf_value_free(v);
    }

    g_free(s);

    if (priv->insert_space_after_word)
    {
      hildon_im_ui_send_communication_message(
            priv->ui, HILDON_IM_CONTEXT_SPACE_AFTER_COMMIT);
    }
    else
    {
      hildon_im_ui_send_communication_message(
            priv->ui, HILDON_IM_CONTEXT_NO_SPACE_AFTER_COMMIT);
    }
  }

  hildon_im_word_completer_configure(priv->hwc, priv->ui);
}

static void
hildon_im_keyboard_assistant_list_free(gpointer data, gpointer user_data)
{
  g_free(data);
}

static void
hildon_im_keyboard_assistant_surrounding_received(HildonIMPlugin *plugin,
                                                  const gchar *surrounding,
                                                  gint offset)
{
  HildonIMKeyboardAssistant *assistant = HILDON_IM_KEYBOARD_ASSISTANT(plugin);
  HildonIMKeyboardAssistantPrivate *priv = assistant->priv;

  if (entry_allowed(assistant))
  {
    if (!surrounding || offset <=0 || offset > g_utf8_strlen(surrounding, -1))
    {
      hildon_im_ui_send_communication_message(priv->ui,
                                              HILDON_IM_CONTEXT_CANCEL_PREEDIT);
    }
    else
    {
      GList *list;

      g_free(priv->word);
      priv->word = NULL;
      g_free(priv->predicted_suffix);
      priv->predicted_suffix = NULL;

      list = utf8_split_in_words(surrounding, offset);

      if (list)
      {
        GList *last = g_list_last(list);
        gchar *curr_word = NULL;
        gchar *curr_word_lc = NULL;
        gchar *prev_word = NULL;
        gchar *p, *q;
        gunichar pch, qch;

        if (last && last->data)
        {
          GList *prev = last->prev;

          curr_word = g_utf8_next_char(last->data);
          curr_word_lc = g_utf8_strdown(curr_word, -1);

          if (prev && prev->data)
            prev_word = g_utf8_strdown(prev->data, -1);
        }

        p = g_utf8_offset_to_pointer(surrounding, offset - 1);
        pch = g_utf8_get_char_validated(p, -1);
        q = g_utf8_offset_to_pointer(surrounding, offset);
        qch = g_utf8_get_char_validated(q, -1);

        if (char_is_part_of_dictionary_word(p) &&
            (!q || !*q || (qch + 2 > 1 && g_unichar_isspace(qch))) && priv->b)
        {
          if (g_utf8_strlen(curr_word_lc, -1) >= priv->display_after_entering)
          {
            gchar *predicted_suffix =
                hildon_im_word_completer_get_predicted_suffix(priv->hwc,
                                                              prev_word,
                                                              curr_word,
                                                              &priv->word);
            priv->predicted_suffix = predicted_suffix;

            if (predicted_suffix && g_utf8_strlen(predicted_suffix, -1) > 0)
            {
              hildon_im_ui_send_communication_message(
                    priv->ui, HILDON_IM_CONTEXT_PREEDIT_MODE);
              hildon_im_ui_send_utf8(priv->ui, priv->predicted_suffix);
            }
            else
            {
              hildon_im_ui_send_communication_message(
                    priv->ui, HILDON_IM_CONTEXT_CANCEL_PREEDIT);
            }

            g_free(priv->str2);
            priv->str2 = g_strdup(curr_word_lc);
          }
        }
        else
        {
          if (!priv->str1 || g_strcmp0(priv->str1, curr_word_lc))
          {
            if (g_unichar_isspace(pch))
            {
              hildon_im_word_completer_hit_word(priv->hwc, curr_word_lc, TRUE);
              g_free(priv->str1);
              priv->str1 = g_strdup(curr_word_lc);
            }
            else
            {
              g_free(priv->str2);
              priv->str2 = NULL;
            }
          }
        }

        g_free(curr_word_lc);
        g_free(prev_word);
        g_list_foreach(list, hildon_im_keyboard_assistant_list_free, NULL);
        g_list_free(list);
      }
    }
  }
}

static void
hildon_im_keyboard_assistant_input_mode_changed(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantPrivate *priv;
  priv = HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;
  priv->input_mode = hildon_im_ui_get_current_input_mode(priv->ui);
}

static void
hildon_im_keyboard_assistant_character_autocase(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;
  gchar *s = g_strdup_printf(
        "/apps/osso/inputmethod/hildon-im-languages/%s/auto-capitalisation",
        priv->lang[priv->language_index]);
  GConfValue *v = gconf_client_get(priv->ui->client, s, NULL);

  if (v)
  {
    hildon_im_ui_set_context_options(priv->ui, HILDON_IM_AUTOCASE,
                                     gconf_value_get_bool(v));
    gconf_value_free(v);
  }

  g_free(s);
}

static void
hildon_im_keyboard_assistant_reset(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;

  if (priv->word)
  {
    g_free(priv->word);
    priv->word = NULL;
    g_free(priv->predicted_suffix);
    priv->predicted_suffix = NULL;
    hildon_im_ui_send_communication_message(priv->ui,
                                            HILDON_IM_CONTEXT_CANCEL_PREEDIT);
  }
}

static void
hildon_im_keyboard_assistant_transition(HildonIMPlugin *plugin, gboolean from)
{
  HildonIMKeyboardAssistantPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;

  hildon_im_ui_send_communication_message(priv->ui,
                                          HILDON_IM_CONTEXT_CANCEL_PREEDIT);
  hildon_im_ui_send_communication_message(
        priv->ui, HILDON_IM_CONTEXT_CONFIRM_SENTENCE_START);
}

static void hildon_im_keyboard_assistant_input_mode_clear(HildonIMPlugin *plugin)
{
  HildonIMKeyboardAssistantPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;
  hildon_im_ui_send_communication_message(priv->ui,
                                          HILDON_IM_CONTEXT_CANCEL_PREEDIT);
}

static void
hildon_im_keyboard_assistant_client_settings_changed(HildonIMPlugin *plugin,
                                                     const gchar *key,
                                                     const GConfValue *value)
{  
  HildonIMKeyboardAssistant *assistant = HILDON_IM_KEYBOARD_ASSISTANT(plugin);

  hildon_im_keyboard_assistant_read_settings(assistant);
  hildon_im_keyboard_assistant_reset(plugin);
}

static void
hildon_im_keyboard_assistant_preedit_committed(HildonIMPlugin *plugin,
                                               const gchar *committed_preedit)
{
  HildonIMKeyboardAssistantPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;

  if ((priv->predicted_suffix && committed_preedit) &&
      !g_ascii_strcasecmp(priv->predicted_suffix, committed_preedit))
  {
    hildon_im_word_completer_hit_word(priv->hwc, priv->word, 0);
  }
}

static void
hildon_im_keyboard_assistant_keyboard_state_changed(HildonIMPlugin *plugin)
{
  hildon_im_keyboard_assistant_save_data(plugin);
}

static void
hildon_im_keyboard_assistant_language_settings_changed(HildonIMPlugin *plugin,
                                                       gint index)
{
  HildonIMKeyboardAssistantPrivate *priv =
      HILDON_IM_KEYBOARD_ASSISTANT(plugin)->priv;

  if (g_strcmp0(hildon_im_ui_get_active_language(priv->ui),
                priv->lang[priv->language_index]))
  {
    hildon_im_keyboard_assistant_read_settings(
          HILDON_IM_KEYBOARD_ASSISTANT(plugin));
    hildon_im_keyboard_assistant_reset(plugin);
  }
}

