#include <hildon-im-settings-plugin.h>
#include <hildon-im-languages.h>
#include <hildon/hildon.h>

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

static GtkWidget *second_dictionary_picker;
static GtkWidget *first_dictionary_picker;
static GtkWidget *dual_dictionary_cbutton;
static GtkWidget *cb_layout;
static GtkWidget *word_completion_cbutton;
static GtkWidget *auto_caps_cbutton;
static GtkWidget *use_fkb_cbutton;
static GtkWidget *insert_space_cbutton;
gboolean has_second_language;

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

static GConfClient *
get_gconf()
{
  static GConfClient *gconf;

  if (!gconf)
    gconf = gconf_client_get_default();

  return gconf;
}

static gboolean
get_language_bool(const gchar *language, const gchar *key,
                  gboolean default_value)
{
  GConfValue *val;
  gboolean rv;

  if (language)
  {
    gchar *gconf_key =
        g_strconcat(HILDON_IM_GCONF_LANG_DIR, "/", language, key, NULL);

    val = gconf_client_get(get_gconf(), gconf_key, NULL);
    g_free(gconf_key);
  }
  else
    val = gconf_client_get(get_gconf(), key, NULL);

  if (val && val->type == GCONF_VALUE_BOOL)
    rv = gconf_value_get_bool(val);
  else
    rv = default_value;

  if (val)
    gconf_value_free(val);

  return rv;
}

static void
set_language_bool(const gchar *lang, const gchar *setting, gboolean val)
{
  if (lang && setting)
  {
    gchar *key =
        g_strconcat(HILDON_IM_GCONF_LANG_DIR, "/", lang, setting, NULL);

    gconf_client_set_bool(get_gconf(), key, val, NULL);
    g_free(key);
  }
}

static gchar *
get_language_dictionary(const gchar *lang)
{
  gchar *key;
  GConfValue *val;
  gchar *dict;

  if (!lang || !*lang)
    return NULL;

  key = g_strconcat(HILDON_IM_GCONF_LANG_DIR, "/", lang, "/dictionary", NULL);
  val = gconf_client_get(get_gconf(), key, NULL);
  g_free(key);

  if (!val || val->type != GCONF_VALUE_STRING)
  {
    if (val)
      gconf_value_free(val);

    return g_strdup(lang);
  }

  dict = g_strdup(gconf_value_get_string(val));
  gconf_value_free(val);

  return dict;
}

static void
set_language_dictionary(HildonIMSettingsPlugin *plugin, const gchar *language)
{
  gchar *lang_id;
  GType gtype;

  lang_id = hildon_im_settings_plugin_manager_get_internal_value(
        CVIM_SETTINGS(plugin)->manager, language, &gtype);

  if (lang_id && *lang_id)
  {
    GtkWidget *picker;
    HildonTouchSelector *selector;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *key;
    gchar *dict = NULL;

    if (g_ascii_strcasecmp(language, HILDON_IM_SETTINGS_PRIMARY_LANGUAGE))
      picker = second_dictionary_picker;
    else
      picker = first_dictionary_picker;

    selector = hildon_picker_button_get_selector(HILDON_PICKER_BUTTON(picker));

    model = hildon_touch_selector_get_model(HILDON_TOUCH_SELECTOR(selector), 0);
    hildon_touch_selector_get_selected(selector, 0, &iter);
    gtk_tree_model_get(model, &iter, 1, &dict, -1);

    key = g_strconcat(HILDON_IM_GCONF_LANG_DIR, "/", lang_id, "/dictionary",
                      NULL);

    if (!dict)
    {
      dict = "";
      gconf_client_set_string(get_gconf(), key, dict, NULL);
    }
    else
    {
      gconf_client_set_string(get_gconf(), key, dict, NULL);
      g_free(dict);
    }

    g_free(key);
  }
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
                            GType gtype, gpointer value)
{
  GtkWidget *picker;
  gchar *lang;

  if (!g_ascii_strcasecmp(key, "PrimaryLanguage"))
    picker = first_dictionary_picker;
  else if (!g_ascii_strcasecmp(key, "SecondaryLanguage"))
  {
    has_second_language = FALSE;

    if(value)
      has_second_language = TRUE;

    if (dual_dictionary_cbutton)
      gtk_widget_set_sensitive(dual_dictionary_cbutton, has_second_language);

    picker = second_dictionary_picker;

    if (picker)
      g_object_set(G_OBJECT(picker), "visible", has_second_language, NULL);

    if (!has_second_language)
      return;
  }
  else
    return;

  if (!picker)
    return;

  lang = hildon_im_settings_plugin_manager_get_internal_value(
        CVIM_SETTINGS(plugin)->manager, key, &gtype);

  if (lang)
  {
    HildonTouchSelector * selector =
        hildon_picker_button_get_selector(HILDON_PICKER_BUTTON(picker));
    GtkTreeModel *model = hildon_touch_selector_get_model(selector, 0);
    GtkTreeIter iter;

    if (model)
    {
      gchar *dict = get_language_dictionary(lang);

      if (gtk_tree_model_get_iter_first(model, &iter))
      {
        while (1)
        {
          gchar *candidate_dict = NULL;

          gtk_tree_model_get(model, &iter, 1, &candidate_dict, -1);

          if (!g_ascii_strcasecmp(candidate_dict, dict))
          {
            hildon_touch_selector_select_iter(HILDON_TOUCH_SELECTOR(selector),
                                              0, &iter, TRUE);
            break;
          }

          if (!gtk_tree_model_iter_next(model, &iter))
            break;
        }
      }

      g_free(dict);
    }
  }
}

static void
cvim_settings_save_data(HildonIMSettingsPlugin *plugin,
                        HildonIMSettingsCategory where)
{
  GSList *available_languages;
  GSList *l;

  gconf_client_set_bool(
        get_gconf(), HILDON_IM_GCONF_DIR "/dual-dictionary",
        hildon_check_button_get_active(
          HILDON_CHECK_BUTTON(dual_dictionary_cbutton)),
        NULL);

  if (gconf_client_get_bool(get_gconf(),
                            HILDON_IM_GCONF_DIR "/have-internal-keyboard",
                            NULL))
  {
    HildonTouchSelector *selector =
        hildon_picker_button_get_selector(HILDON_PICKER_BUTTON(cb_layout));
    GtkTreeModel *model =
        hildon_touch_selector_get_model(HILDON_TOUCH_SELECTOR(selector), 0);
    GtkTreeIter iter;
    gchar *int_kb_layout;

    hildon_touch_selector_get_selected(selector, 0, &iter);
    gtk_tree_model_get(model, &iter, 1, &int_kb_layout, -1);

    gconf_client_set_string(get_gconf(), HILDON_IM_GCONF_DIR "/int_kb_layout",
                            int_kb_layout, NULL);

    g_free(int_kb_layout);

    gconf_client_set_bool(
          get_gconf(), HILDON_IM_GCONF_DIR "/use_finger_kb",
          hildon_check_button_get_active(HILDON_CHECK_BUTTON(use_fkb_cbutton)),
          NULL);
  }

  available_languages = hildon_im_get_available_languages();

  for (l = available_languages; l; l = l->next)
  {
    const gchar *lang = ((HildonIMLanguage *)l->data)->language_code;
    gboolean active;

    active = hildon_check_button_get_active(
          HILDON_CHECK_BUTTON(auto_caps_cbutton));
    set_language_bool(lang, "/auto-capitalisation", active);

    active = hildon_check_button_get_active(
          HILDON_CHECK_BUTTON(word_completion_cbutton));
    set_language_bool(lang, "/word-completion", active);

    active = hildon_check_button_get_active(
          HILDON_CHECK_BUTTON(insert_space_cbutton));
    set_language_bool(lang, "/insert-space-after-word", active);
  }

  hildon_im_free_available_languages(available_languages);
  set_language_dictionary(plugin, HILDON_IM_SETTINGS_PRIMARY_LANGUAGE);

  if (has_second_language)
    set_language_dictionary(plugin, HILDON_IM_SETTINGS_SECONDARY_LANGUAGE);
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
