#include <glib.h>
#include <errno.h>
#include <string.h>

#include <gtk/gtkwidget.h>
#include <gtk/gtkcontainer.h>

#include <hildon-im-ui.h>

#include "hildon-im-western-fkb.h"
#include "hildon-im-word-completer.h"

static GType hildon_im_word_completer_type = 0;
static HildonIMWWordCompleter *hildon_im_word_completer = NULL;
static GObjectClass *parent_class = NULL;

enum{
  HILDON_IM_WORD_COMPLETER_PROP_LANGUAGE = 1,
  HILDON_IM_WORD_COMPLETER_PROP_SECOND_LANGUAGE,
  HILDON_IM_WORD_COMPLETER_PROP_DUAL_DICTIONARY,
  HILDON_IM_WORD_COMPLETER_PROP_MAX_CANDIDATES,
  HILDON_IM_WORD_COMPLETER_PROP_MAX_SUFFIX,
  HILDON_IM_WORD_COMPLETER_PROP_LAST
};

struct _HildonIMWWordCompleterPrivate{
  gchar *lang[2];
  gboolean dual_dictionary;
  gint max_candidates;
  glong max_suffix;
  gchar *base_dir;
};

static void hildon_im_word_completer_class_init(HildonIMWWordCompleterClass *klass);
static void hildon_im_word_completer_init(HildonIMWWordCompleter *wc);

static GObject* hildon_im_word_completer_constructor(GType gtype, guint n_properties, GObjectConstructParam *properties);
static void hildon_im_word_completer_finalize(GObject *object);

static void hildon_im_word_completer_set_property(GObject *object,guint prop_id,const GValue *value,GParamSpec *pspec);
static void hildon_im_word_completer_get_property(GObject *object,guint prop_id,GValue *value,GParamSpec *pspec);

GType hildon_im_word_completer_get_type()
{
  static const GTypeInfo type_info = {
    sizeof(HildonIMWWordCompleterClass),
    NULL, /* base_init */
    NULL, /* base_finalize */
    (GClassInitFunc) hildon_im_word_completer_class_init,
    NULL, /* class_finalize */
    NULL, /* class_data */
    sizeof(HildonIMWWordCompleter),
    0, /* n_preallocs */
    (GInstanceInitFunc) hildon_im_word_completer_init,
    NULL
  };

  if(!hildon_im_word_completer_type)
    hildon_im_word_completer_type = g_type_register_static(
          G_TYPE_OBJECT,
          "HildonIMWordCompleter",
          &type_info,
          0);

  return hildon_im_word_completer_type;
}

gpointer hildon_im_word_completer_new()
{
  return g_object_new(HILDON_IM_WORD_COMPLETER_TYPE, NULL);
}

static void hildon_im_word_completer_class_init(HildonIMWWordCompleterClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(HildonIMWWordCompleterPrivate));
  parent_class = g_type_class_peek_parent(klass);

  object_class->constructor = hildon_im_word_completer_constructor;
  object_class->set_property = hildon_im_word_completer_set_property;
  object_class->get_property = hildon_im_word_completer_get_property;
  object_class->finalize = hildon_im_word_completer_finalize;

  g_object_class_install_property(object_class, HILDON_IM_WORD_COMPLETER_PROP_LANGUAGE,
                                  g_param_spec_string(
                                    "language",
                                    "Language",
                                    "The language used in word completion",
                                    "en_GB",
                                    G_PARAM_WRITABLE|G_PARAM_READABLE));

  g_object_class_install_property(object_class, HILDON_IM_WORD_COMPLETER_PROP_SECOND_LANGUAGE,
                                  g_param_spec_string(
                                    "second-language",
                                    "Second Language",
                                    "The second language used in word completion",
                                    "en_GB",
                                    G_PARAM_WRITABLE|G_PARAM_READABLE));

  g_object_class_install_property(object_class, HILDON_IM_WORD_COMPLETER_PROP_DUAL_DICTIONARY,
                                  g_param_spec_boolean(
                                    "dual-dictionary",
                                    "Dual Dictionary",
                                    "Dual dictionary used in word completion",
                                    0,
                                    G_PARAM_WRITABLE|G_PARAM_READABLE));

  g_object_class_install_property(object_class, HILDON_IM_WORD_COMPLETER_PROP_MAX_CANDIDATES,
                                  g_param_spec_int(
                                     "max_candidates",
                                     "Max. candidates",
                                     "The max. number of candidates for word completion",
                                     0,
                                     2147483647,
                                     1,
                                     G_PARAM_WRITABLE|G_PARAM_READABLE));

   g_object_class_install_property(object_class, HILDON_IM_WORD_COMPLETER_PROP_MAX_SUFFIX,
                                   g_param_spec_long(
                                     "min_candidate_suffix_length",
                                     "Min. candidate length",
                                     "The minimum length of the suffix of the selected candidate.",
                                     0,
                                     2147483647,
                                     2,
                                     G_PARAM_WRITABLE|G_PARAM_READABLE));
}

static void hildon_im_word_completer_init(HildonIMWWordCompleter *wc)
{
  HildonIMWWordCompleterPrivate *priv;

  priv = HILDON_IM_WORD_COMPLETER_GET_PRIVATE (HILDON_IM_WORD_COMPLETER(wc));

  wc->priv = priv;
  priv->lang[0] = g_strdup("en_GB");
  priv->lang[1] = g_strdup("en_GB");
  priv->dual_dictionary = FALSE;
  priv->max_candidates = 1;

  imengines_wp_init("ezitext");

  priv->base_dir = g_build_filename(g_get_home_dir(), ".osso/dictionaries", NULL);;

  if ( g_mkdir_with_parents(priv->base_dir, 0755) )
    g_warning("Couldn't create directory %s: %s", priv->base_dir, strerror(errno));
  else
    imengines_wp_set_data("base-dir", priv->base_dir);

  imengines_wp_set_data("ezitext", (void *)0xBBC58F26); /* WTF ?!? */
  imengines_wp_set_prediction_language(priv->lang[0], 0);
  imengines_wp_set_prediction_language(priv->lang[0], 1);
  imengines_wp_set_max_candidates(priv->max_candidates);
  imengines_wp_attach_dictionary(1, 1);
  imengines_wp_attach_dictionary(2, 1);
  imengines_wp_attach_dictionary(0, 1);
}

static GObject* hildon_im_word_completer_constructor(GType gtype, guint n_properties, GObjectConstructParam *properties)
{
  GObject *obj;

  if ( hildon_im_word_completer )
    obj = g_object_ref(G_OBJECT(hildon_im_word_completer));
  else
  {
    obj = G_OBJECT_CLASS(parent_class)->constructor(gtype, n_properties, properties);
    hildon_im_word_completer = HILDON_IM_WORD_COMPLETER(obj);
  }

  return obj;
}

static void hildon_im_word_completer_finalize(GObject *object)
{
  HildonIMWWordCompleter *wc;
  HildonIMWWordCompleterPrivate *priv;

  wc = HILDON_IM_WORD_COMPLETER(object);
  priv = HILDON_IM_WORD_COMPLETER_GET_PRIVATE (wc);

  hildon_im_word_completer_save_data(wc);

  imengines_wp_detach_dictionary(2);
  imengines_wp_detach_dictionary(1);
  imengines_wp_detach_dictionary(0);
  imengines_wp_destroy();

  g_free(priv->base_dir);

  if (G_OBJECT_CLASS(parent_class)->finalize)
    G_OBJECT_CLASS(parent_class)->finalize(object);

  hildon_im_word_completer = 0;
}

static void hildon_im_word_completer_set_property(GObject *object,
                                      guint prop_id,
                                      const GValue *value,
                                      GParamSpec *pspec)
{
  HildonIMWWordCompleterPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WORD_COMPLETER(object));

  priv = HILDON_IM_WORD_COMPLETER_GET_PRIVATE(object);

  switch (prop_id)
  {
    case HILDON_IM_WORD_COMPLETER_PROP_LANGUAGE:
      g_free(priv->lang[0]);
      priv->lang[0] = g_value_dup_string(value);

      if(priv->lang[0] && *priv->lang[0])
      {
        imengines_wp_attach_dictionary(0,1);
        imengines_wp_set_prediction_language(priv->lang[0], 0);
        if(priv->dual_dictionary)
          imengines_wp_set_prediction_language(priv->lang[0], 1);
        else
        {
          g_free(priv->lang[1]);
          priv->lang[1] = g_strdup(priv->lang[0]);
          imengines_wp_set_prediction_language(priv->lang[1], 1);
        }

      }
      else
        imengines_wp_detach_dictionary(0);
      break;
    case HILDON_IM_WORD_COMPLETER_PROP_SECOND_LANGUAGE:
      g_free(priv->lang[1]);
      priv->lang[1] = g_value_dup_string(value);

      if(priv->lang[1] && *priv->lang[1])
      {
        if(!priv->dual_dictionary)
        {
          if(!priv->lang[0] || !*priv->lang[0])
            break;
          imengines_wp_attach_dictionary(0,1);
          imengines_wp_set_prediction_language(priv->lang[0], 1);
        }
        else
        {
          imengines_wp_attach_dictionary(0,1);
          imengines_wp_set_prediction_language(priv->lang[1], 1);
        }
      }
      else
      {
        if(!priv->lang[0] || !*priv->lang[0])
          imengines_wp_detach_dictionary(0);
        else
        {
          imengines_wp_attach_dictionary(0,1);
          imengines_wp_set_prediction_language(priv->lang[0], 0);
        }
      }
      break;
    case HILDON_IM_WORD_COMPLETER_PROP_DUAL_DICTIONARY:
    {
      gboolean dual_dictionary = g_value_get_boolean(value);

      if(priv->dual_dictionary == dual_dictionary)
        break;

      priv->dual_dictionary = dual_dictionary;

      /* hmm, what if second dictionary is not set? */
      if(dual_dictionary)
        imengines_wp_set_prediction_language(priv->lang[1], 1);
      else
        imengines_wp_set_prediction_language(priv->lang[0], 1);
      break;
    }
    case HILDON_IM_WORD_COMPLETER_PROP_MAX_CANDIDATES:
    {
      gint max_candidates = g_value_get_int(value);

      if(priv->max_candidates == max_candidates)
        break;

      priv->max_candidates = max_candidates;
      imengines_wp_set_max_candidates(max_candidates);

      break;
    }
    case HILDON_IM_WORD_COMPLETER_PROP_MAX_SUFFIX:
      priv->max_suffix = g_value_get_long(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void
hildon_im_word_completer_get_property(GObject *object,
                                      guint prop_id,
                                      GValue *value,
                                      GParamSpec *pspec)
{
  HildonIMWWordCompleterPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WORD_COMPLETER(object));

  priv = HILDON_IM_WORD_COMPLETER_GET_PRIVATE(object);

  switch (prop_id)
  {
    case HILDON_IM_WORD_COMPLETER_PROP_LANGUAGE:
      g_value_set_string(value, priv->lang[0]);
      break;
    case HILDON_IM_WORD_COMPLETER_PROP_SECOND_LANGUAGE:
      g_value_set_string(value, priv->lang[1]);
      break;
    case HILDON_IM_WORD_COMPLETER_PROP_DUAL_DICTIONARY:
      g_value_set_boolean(value, priv->dual_dictionary);
      break;
    case HILDON_IM_WORD_COMPLETER_PROP_MAX_CANDIDATES:
      g_value_set_int(value, priv->max_candidates);
      break;
    case HILDON_IM_WORD_COMPLETER_PROP_MAX_SUFFIX:
      g_value_set_long(value, priv->max_suffix);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

void hildon_im_word_completer_save_data(HildonIMWWordCompleter *hwc)
{
  imengines_wp_save_dictionary(1);
  imengines_wp_save_dictionary(2);
}

void hildon_im_word_completer_configure(HildonIMWWordCompleter *hwc, HildonIMUI *ui)
{
  guint lang_index;

  gchar *key;
  GConfValue *value;
  const gchar* first_lang;
  const gchar *second_lang;
  const gchar *lang[2];

  g_return_if_fail(ui != NULL && hwc != NULL);

  lang[0] = hildon_im_ui_get_language_setting(ui, 0);
  lang[1] = hildon_im_ui_get_language_setting(ui, 1);
  lang_index = hildon_im_ui_get_active_language_index(ui);

  first_lang = lang[lang_index];
  second_lang = lang[lang_index?0:1]; /* not a bug :) */

  if (first_lang && *first_lang)
  {
    key = g_strdup_printf("/apps/osso/inputmethod/hildon-im-languages/%s/dictionary", first_lang);
    value = gconf_client_get(ui->client, key, NULL);

    if (value)
    {
      g_object_set(hwc, "language", gconf_value_get_string(value), NULL);
      gconf_value_free(value);
    }
    else
      g_object_set(hwc, "language", "", NULL);

    g_free(key);
  }
  else
    g_object_set(hwc, "language", "", NULL);

  if (second_lang && *second_lang)
  {
    key = g_strdup_printf("/apps/osso/inputmethod/hildon-im-languages/%s/dictionary",second_lang);
    value = gconf_client_get(ui->client, key, NULL);

    if (value)
    {
      g_object_set(hwc, "second-language", gconf_value_get_string(value), NULL);
      gconf_value_free(value);
    }
    else
      g_object_set(hwc, "second-language", "", NULL);

    g_free(key);
  }
  else
    g_object_set(hwc, "second-language", "", NULL);

  value = gconf_client_get(ui->client, "/apps/osso/inputmethod/dual-dictionary", NULL);

  if (value)
  {
    g_object_set(hwc, "dual-dictionary", gconf_value_get_bool(value), NULL);
    gconf_value_free(value);
  }
}
