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
static gboolean has_second_language;
static char *selected_language;

struct keyboard_layout
{
  gchar *language;
  gchar *layout;
};

static struct keyboard_layout keyboard_layouts[] =
{
  { "Čeština", "cz" },
  { "Dansk, Norsk", "dano" },
  { "Deutsch", "de" },
  { "English, Nederlands", "us" },
  { "Español, Français (Canada), Português", "ptes" },
  { "Français (France)", "fr" },
  { "Italiano", "it" },
  { "Polski", "pl" },
  { "Suomi, Svenska", "fise" },
  { "Suisse, Schweiz", "ch" },
  { "Русский", "ru" }
};

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
  second_dictionary_picker = NULL;
  first_dictionary_picker = NULL;
  dual_dictionary_cbutton = NULL;
  cb_layout = NULL;
  word_completion_cbutton = NULL;
  auto_caps_cbutton = NULL;
  use_fkb_cbutton = NULL;
  insert_space_cbutton = NULL;
  has_second_language = FALSE;
  selected_language = NULL;

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
            g_free(candidate_dict);
            break;
          }

          g_free(candidate_dict);

          if (!gtk_tree_model_iter_next(model, &iter))
            break;
        }
      }

      g_free(dict);
    }
  }
}

static GtkWidget *
cvim_settings_create_language_general_widget(HildonIMSettingsPlugin *plugin,
                                             gint *width)
{
  GSList *l;
  gchar *lc;
  GtkWidget *vbox;
  GType gtype;
  selected_language = hildon_im_settings_plugin_manager_get_internal_value(
        CVIM_SETTINGS(plugin)->manager, "SelectedLanguage", &gtype);

  vbox = gtk_vbox_new(FALSE, FALSE);

  word_completion_cbutton = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
  gtk_button_set_label(GTK_BUTTON(word_completion_cbutton),
                       dgettext("osso-applet-textinput",
                                "tein_fi_settings_word_completion"));
  gtk_box_pack_start(GTK_BOX(vbox), word_completion_cbutton, FALSE, FALSE, 0);

  auto_caps_cbutton = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
  gtk_button_set_label(GTK_BUTTON(auto_caps_cbutton),
                       dgettext("osso-applet-textinput",
                                "tein_fi_settings_auto_capitalization"));
  gtk_box_pack_start(GTK_BOX(vbox), auto_caps_cbutton, FALSE, FALSE, 0);

  insert_space_cbutton = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
  gtk_button_set_label(GTK_BUTTON(insert_space_cbutton),
                       dgettext("osso-applet-textinput",
                                "tein_fi_settings_space_after_word"));
  gtk_box_pack_start(GTK_BOX(vbox), insert_space_cbutton, FALSE, FALSE, 0);

  *width = 0;
  l = hildon_im_get_available_languages();
  lc = ((HildonIMLanguage *)l->data)->language_code;

  hildon_check_button_set_active(
        HILDON_CHECK_BUTTON(auto_caps_cbutton),
        get_language_bool(lc, "/auto-capitalisation", TRUE));

  hildon_check_button_set_active(
        HILDON_CHECK_BUTTON(insert_space_cbutton),
        get_language_bool(lc, "/insert-space-after-word", FALSE));

  hildon_check_button_set_active(
        HILDON_CHECK_BUTTON(word_completion_cbutton),
        get_language_bool(lc, "/word-completion", TRUE));

  hildon_im_free_available_languages(l);

  gtk_widget_show_all(vbox);

  return vbox;
}

static GtkWidget *
cvim_settings_create_language_additional_widget(HildonIMSettingsPlugin *plugin,
                                                gint *width)
{
  gpointer sec_lang;
  GType gtype;

  dual_dictionary_cbutton = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
  gtk_button_set_label(GTK_BUTTON(dual_dictionary_cbutton),
                       dgettext("osso-applet-textinput",
                                "tein_fi_dual_dictionary_use"));
  sec_lang = hildon_im_settings_plugin_manager_get_internal_value(
        CVIM_SETTINGS(plugin)->manager, "SecondaryLanguage", &gtype);

  hildon_check_button_set_active(
        HILDON_CHECK_BUTTON(dual_dictionary_cbutton),
        get_language_bool(NULL, HILDON_IM_GCONF_DIR "/dual-dictionary", FALSE));

  cvim_settings_value_changed(plugin, "SecondaryLanguage", gtype, sec_lang);

  *width = 0;

  gtk_widget_show(dual_dictionary_cbutton);

  return dual_dictionary_cbutton;
}

static gint
language_compare(HildonIMLanguage *a, HildonIMLanguage *b)
{
  return g_utf8_collate(a->description, b->description);
}

static GtkWidget *
cvim_settings_create_language_picker_widget(gchar *language)
{
  GSList *available_languages;
  GtkListStore *list_store;
  gchar *dict = NULL;
  GSList *l;
  GtkTreeIter *selected_iter = NULL;
  HildonTouchSelectorColumn *column;
  GtkWidget *picker;
  GtkWidget *selector;
  GtkTreeIter iter;

  available_languages = hildon_im_get_available_languages();
  picker = hildon_picker_button_new(HILDON_SIZE_FINGER_HEIGHT,
                                    HILDON_BUTTON_ARRANGEMENT_VERTICAL);
  hildon_button_set_alignment(HILDON_BUTTON(picker), 0.0, 0.5, 1.0, 0.0);
  hildon_button_set_title(HILDON_BUTTON(picker),
                          dgettext("osso-applet-textinput",
                                   "tein_fi_settings_dictionary"));

  selector = hildon_touch_selector_new();

  available_languages = g_slist_sort(available_languages,
                                     (GCompareFunc)language_compare);

  list_store = gtk_list_store_new(2, 64, 64);
  dict = get_language_dictionary(language);

  for (l = available_languages; l; l = l->next)
  {
    HildonIMLanguage *lang = l->data;

    gtk_list_store_append(list_store, &iter);
    gtk_list_store_set(list_store, &iter,
                       0, lang->description,
                       1, lang->language_code,
                       -1);

    if (dict && !selected_iter &&
        !g_ascii_strcasecmp(lang->language_code, dict))
    {
      selected_iter = gtk_tree_iter_copy(&iter);
    }
  }

  g_free(dict);
  hildon_im_free_available_languages(available_languages);
  gtk_list_store_append(list_store, &iter);
  gtk_list_store_set(list_store, &iter,
                     0, dgettext("osso-applet-textinput",
                                 "tein_fi_word_completion_language_empty"),
                     1, NULL,
                     -1);

  if (!selected_iter)
    selected_iter = gtk_tree_iter_copy(&iter);

  column = hildon_touch_selector_append_text_column(
        HILDON_TOUCH_SELECTOR(selector), GTK_TREE_MODEL(list_store), TRUE);
  g_object_set(G_OBJECT(column), "text-column", 0, NULL);

  if (selected_iter)
  {
    hildon_touch_selector_select_iter(HILDON_TOUCH_SELECTOR(selector), 0,
                                      selected_iter, TRUE);
    gtk_tree_iter_free(selected_iter);
  }

  hildon_picker_button_set_selector(HILDON_PICKER_BUTTON(picker),
                                    HILDON_TOUCH_SELECTOR(selector));

  return picker;
}

static GtkWidget *
cvim_settings_create_language_widget(HildonIMSettingsPlugin *plugin,
                                     HildonIMSettingsCategory category,
                                     gint *width)
{
  GType gtype;
  GtkWidget **widget;

  *width = 0;

  selected_language = hildon_im_settings_plugin_manager_get_internal_value(
        CVIM_SETTINGS(plugin)->manager, "SelectedLanguage", &gtype);

  if (category == HILDON_IM_SETTINGS_PRIMARY_LANGUAGE_SETTINGS_WIDGET)
    widget = &first_dictionary_picker;
  else if (category == HILDON_IM_SETTINGS_SECONDARY_LANGUAGE_SETTINGS_WIDGET)
    widget = &second_dictionary_picker;
  else
    return NULL;

  if (!*widget)
    *widget = cvim_settings_create_language_picker_widget(selected_language);

  if (*widget)
    gtk_widget_show(*widget);

  return *widget;
}

static GtkWidget *
cvim_settings_create_onscreen_widget(HildonIMSettingsPlugin *plugin,
                                     gint *width)
{
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

  use_fkb_cbutton = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);

  gtk_button_set_label(GTK_BUTTON(use_fkb_cbutton),
                       dgettext("osso-applet-textinput",
                                "tein_fi_use_virtual_keyboard"));

  hildon_check_button_set_active(
        HILDON_CHECK_BUTTON(use_fkb_cbutton),
        gconf_client_get_bool(get_gconf(),
                              HILDON_IM_GCONF_DIR "/use_finger_kb", NULL));

  gtk_box_pack_start(GTK_BOX(vbox), use_fkb_cbutton, FALSE, FALSE, 0);

  *width = 0;

  gtk_widget_show_all(vbox);

  return vbox;
}

static GtkWidget *
cvim_settings_create_hardware_widget(HildonIMSettingsPlugin *plugin,
                                     gint *width)
{
  GConfValue *val;
  int i;
  GtkListStore *list_store;
  GtkTreeIter *selected_layout = NULL;
  HildonTouchSelectorColumn *column;
  gchar *int_kb_layout;
  GtkWidget *selector;
  GtkTreeIter iter;
  GtkWidget *vbox;

  *width = 0;

  if (!gconf_client_get_bool(get_gconf(),
                             HILDON_IM_GCONF_DIR "/have-internal-keyboard",
                             NULL))
  {
    return NULL;
  }

  vbox = gtk_vbox_new(FALSE, 0);

  cb_layout = hildon_picker_button_new(HILDON_SIZE_FINGER_HEIGHT,
                                       HILDON_BUTTON_ARRANGEMENT_VERTICAL);
  hildon_button_set_title(HILDON_BUTTON(cb_layout),
                          dgettext("osso-applet-textinput",
                                   "tein_fi_keyboard_layout"));
  hildon_button_set_alignment(HILDON_BUTTON(cb_layout), 0.0, 0.5, 1.0, 0.0);

  selector = hildon_touch_selector_new();

  val = gconf_client_get(get_gconf(),
                         HILDON_IM_GCONF_DIR "/int_kb_layout", NULL);

  if (val)
  {
    int_kb_layout = g_strdup(gconf_value_get_string(val));
    gconf_value_free(val);
  }
  else
  {
    g_warning("there is no gconf value available from [%s] \n",
              HILDON_IM_GCONF_DIR "/int_kb_layout");

    int_kb_layout = g_strdup("us");
  }

  list_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

  for (i = 0; i < G_N_ELEMENTS(keyboard_layouts); i++)
  {
    const gchar *layout = keyboard_layouts[i].layout;

    gtk_list_store_append(list_store, &iter);
    gtk_list_store_set(list_store, &iter,
                       0, keyboard_layouts[i].language,
                       1, layout,
                       -1);

    if (!g_ascii_strcasecmp(int_kb_layout, layout))
      selected_layout = gtk_tree_iter_copy(&iter);
  }

  column = hildon_touch_selector_append_text_column(
        HILDON_TOUCH_SELECTOR(selector), GTK_TREE_MODEL(list_store),
        TRUE);
  g_object_set(G_OBJECT(column), "text-column", 0, NULL);

  if (selected_layout)
  {
    hildon_touch_selector_select_iter(HILDON_TOUCH_SELECTOR(selector), 0,
                                      selected_layout, TRUE);
    gtk_tree_iter_free(selected_layout);
  }

  hildon_picker_button_set_selector(HILDON_PICKER_BUTTON(cb_layout),
                                    HILDON_TOUCH_SELECTOR(selector));
  g_free(int_kb_layout);

  gtk_box_pack_start(GTK_BOX(vbox), cb_layout, FALSE, FALSE, 0);
  gtk_widget_show_all(vbox);

  return vbox;
}

static GtkWidget *
cvim_settings_create_widget(HildonIMSettingsPlugin *plugin,
                            HildonIMSettingsCategory category,
                            GtkSizeGroup *size_group, gint *width)
{
  switch (category)
  {
    case HILDON_IM_SETTINGS_HARDWARE:
      return cvim_settings_create_hardware_widget(plugin, width);
    case HILDON_IM_SETTINGS_ONSCREEN:
      return cvim_settings_create_onscreen_widget(plugin, width);
    case HILDON_IM_SETTINGS_LANGUAGE_GENERAL:
      return cvim_settings_create_language_general_widget(plugin, width);
    case HILDON_IM_SETTINGS_LANGUAGE_ADDITIONAL:
      return cvim_settings_create_language_additional_widget(plugin, width);
    case HILDON_IM_SETTINGS_PRIMARY_LANGUAGE_SETTINGS_WIDGET:
    case HILDON_IM_SETTINGS_SECONDARY_LANGUAGE_SETTINGS_WIDGET:
      return cvim_settings_create_language_widget(plugin, category, width);
    default:
      break;
  }

  return NULL;
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
  }

  gconf_client_set_bool(
        get_gconf(), HILDON_IM_GCONF_DIR "/use_finger_kb",
        hildon_check_button_get_active(HILDON_CHECK_BUTTON(use_fkb_cbutton)),
        NULL);

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
