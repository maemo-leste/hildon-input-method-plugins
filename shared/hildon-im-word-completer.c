/**
   @file hildon-im-word-completer.c

   This file may contain parts derived by disassembling of binaries under
   Nokia's copyright, see http://tablets-dev.nokia.com/maemo-dev-env-downloads.php

   The original licensing conditions apply to all those derived parts as well
   and you accept those by using this file.
*/

#include <glib.h>
#include <errno.h>
#include <string.h>

#include <gtk/gtkwidget.h>
#include <gtk/gtkcontainer.h>

#include <hildon-im-ui.h>
#include <imengines-wp.h>

#include "hildon-im-word-completer.h"

static GType hildon_im_word_completer_type = 0;
static HildonIMWordCompleter *hildon_im_word_completer = NULL;
static GObjectClass *parent_class = NULL;

enum{
  HILDON_IM_WORD_COMPLETER_PROP_LANGUAGE = 1,
  HILDON_IM_WORD_COMPLETER_PROP_SECOND_LANGUAGE,
  HILDON_IM_WORD_COMPLETER_PROP_DUAL_DICTIONARY,
  HILDON_IM_WORD_COMPLETER_PROP_MAX_CANDIDATES,
  HILDON_IM_WORD_COMPLETER_PROP_MAX_SUFFIX,
  HILDON_IM_WORD_COMPLETER_PROP_LAST
};

struct _HildonIMWordCompleterPrivate{
  gchar *lang[2];
  gboolean dual_dictionary;
  gint max_candidates;
  glong max_suffix;
  gchar *base_dir;
};

static void hildon_im_word_completer_class_init(HildonIMWordCompleterClass *klass);
static void hildon_im_word_completer_init(HildonIMWordCompleter *wc);

static GObject* hildon_im_word_completer_constructor(GType gtype, guint n_properties, GObjectConstructParam *properties);
static void hildon_im_word_completer_finalize(GObject *object);

static void hildon_im_word_completer_set_property(GObject *object,guint prop_id,const GValue *value,GParamSpec *pspec);
static void hildon_im_word_completer_get_property(GObject *object,guint prop_id,GValue *value,GParamSpec *pspec);

GType hildon_im_word_completer_get_type()
{
  static const GTypeInfo type_info = {
    sizeof(HildonIMWordCompleterClass),
    NULL, /* base_init */
    NULL, /* base_finalize */
    (GClassInitFunc) hildon_im_word_completer_class_init,
    NULL, /* class_finalize */
    NULL, /* class_data */
    sizeof(HildonIMWordCompleter),
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

static void hildon_im_word_completer_class_init(HildonIMWordCompleterClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(HildonIMWordCompleterPrivate));
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

static void hildon_im_word_completer_init(HildonIMWordCompleter *wc)
{
  HildonIMWordCompleterPrivate *priv;

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
  HildonIMWordCompleter *wc;
  HildonIMWordCompleterPrivate *priv;

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
  HildonIMWordCompleterPrivate *priv;

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
  HildonIMWordCompleterPrivate *priv;

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

void hildon_im_word_completer_save_data(HildonIMWordCompleter *wc)
{
  imengines_wp_save_dictionary(1);
  imengines_wp_save_dictionary(2);
}

void hildon_im_word_completer_configure(HildonIMWordCompleter *wc, HildonIMUI *ui)
{
  guint lang_index;

  gchar *key;
  GConfValue *value;
  const gchar* first_lang;
  const gchar *second_lang;
  const gchar *lang[2];

  g_return_if_fail(ui != NULL && wc != NULL);

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
      g_object_set(wc, "language", gconf_value_get_string(value), NULL);
      gconf_value_free(value);
    }
    else
      g_object_set(wc, "language", "", NULL);

    g_free(key);
  }
  else
    g_object_set(wc, "language", "", NULL);

  if (second_lang && *second_lang)
  {
    key = g_strdup_printf("/apps/osso/inputmethod/hildon-im-languages/%s/dictionary",second_lang);
    value = gconf_client_get(ui->client, key, NULL);

    if (value)
    {
      g_object_set(wc, "second-language", gconf_value_get_string(value), NULL);
      gconf_value_free(value);
    }
    else
      g_object_set(wc, "second-language", "", NULL);

    g_free(key);
  }
  else
    g_object_set(wc, "second-language", "", NULL);

  value = gconf_client_get(ui->client, "/apps/osso/inputmethod/dual-dictionary", NULL);

  if (value)
  {
    g_object_set(wc, "dual-dictionary", gconf_value_get_bool(value), NULL);
    gconf_value_free(value);
  }
}

const gchar *_special_chars[]={".",",",":",";","!","?"};

static gboolean hildon_im_word_completer_word_at_index(guint dict, gchar *word, guint index)
{
  gboolean result;
  gboolean exists;

  result = imengines_wp_word_exists(word, dict, &exists);

  if ( result )
    result = (exists == index);

  return result;
}

static gboolean hildon_im_word_completer_move_word(int dict, gchar *word, guint index)
{
  imengines_wp_delete_word(word, dict, index);
  return (imengines_wp_add_word(word, dict, index) == 0);
}

gboolean hildon_im_word_completer_hit_word(HildonIMWordCompleter *wc, const gchar *text, gboolean b)
{
  gboolean has_lang;
  gboolean ret;
  gchar *word;
  HildonIMWordCompleterPrivate *priv;
  gchar *p;
  gunichar last_char;
  const gchar *special_chars[6];
  const gchar* str;
  int i;

  priv = HILDON_IM_WORD_COMPLETER_GET_PRIVATE (wc);

  str = text;

  if ( !str || !*str || !g_utf8_validate(str, -1, 0))
    return FALSE;

  while ( 1 )
  {
    gunichar uc = g_utf8_get_char_validated(str, -1);

    if ( ((int)uc) <= -1 )
      break;

    if ( !g_unichar_isalnum(uc) && !g_unichar_ismark(uc) && (*str != '-') && (*str != '_') && (*str != '\'') && (*str != '&') )
      break;

    str = g_utf8_next_char(str);

    if ( !*str )
      goto go_on;
  }

  if (g_utf8_next_char(str))
    return FALSE;

go_on:

  word = g_utf8_strdown(text, -1);
  memcpy(special_chars, _special_chars, sizeof(special_chars));

  p = g_utf8_offset_to_pointer(word, g_utf8_strlen(word, -1) - 1);
  last_char = g_utf8_get_char(p);

  i = 0;

  while ( 1 )
  {
    gchar *s = g_utf8_normalize(special_chars[i], -1, G_NORMALIZE_DEFAULT);
    gunichar uc = g_utf8_get_char_validated(s, -1);
    g_free(s);

    if ( last_char == uc )
    {
      *p = 0;
      break;
    }
    i++;

    if (i == (sizeof(special_chars)/sizeof(special_chars[0])))
      break;
  }


  i = 0;
  while ( 1 )
  {
    if ( *priv->lang[0] )
    {
      has_lang = TRUE;
      if ( b )
        goto LABEL_31;
    }
    else
    {
      has_lang = (*priv->lang[1] != 0);
      if ( b )
      {
LABEL_31:
        if ( b != 1 )
          goto LABEL_32;
        if ( hildon_im_word_completer_word_at_index(1u, word, i) )
        {
          ret = hildon_im_word_completer_move_word(1, word, i);
        }
        else
        {
          if ( hildon_im_word_completer_word_at_index(2u, word, i) )
            goto LABEL_39;
          if ( !has_lang || !hildon_im_word_completer_word_at_index(0, word, i) )
          {
LABEL_32:
            ret = FALSE;
            goto LABEL_27;
          }
          ret = TRUE;
        }
        goto LABEL_27;
      }
    }
    if ( has_lang && hildon_im_word_completer_word_at_index(0, word, i) )
      goto LABEL_39;
    if ( !hildon_im_word_completer_word_at_index(1u, word, i) )
    {
      if ( !hildon_im_word_completer_word_at_index(2u, word, i) )
        goto LABEL_32;
LABEL_39:
      ret = hildon_im_word_completer_move_word(2, (gchar *)word, i);
      goto LABEL_27;
    }
    ret = hildon_im_word_completer_word_at_index(2u, word, i) ? 0 : hildon_im_word_completer_move_word(
                                                                        2,
                                                                        word,
                                                                        i);
    imengines_wp_delete_word(word, 1, i);
LABEL_27:
    ++i;
    if ( (priv->dual_dictionary != 0) < i )
      break;
    if ( ret )
      goto LABEL_42;
  }
  if ( !ret )
    ret = hildon_im_word_completer_move_word(1, word, 0);
LABEL_42:
  g_free(word);
  return ret;
}

gchar *hildon_im_word_completer_get_predicted_suffix(HildonIMWordCompleter *wc, gchar *previous_word, const gchar *current_word, gchar **out)
{
  gchar *candidate = hildon_im_word_completer_get_one_candidate(wc, previous_word, current_word);

  if (current_word && *current_word && candidate)
  {
    gchar *rv;
    size_t len = strlen(current_word);
    size_t clen = strlen(candidate);

    if ( len < clen )
      rv = g_strdup(&candidate[len]);
    else
      rv = g_strdup("");

    if ( out )
    {
      if ( len < clen )
        *out = g_strdup(candidate);
    }

    g_free(candidate);
    return rv;
  }

  return g_strdup("");
}

static gboolean str_contains_uppercase(const gchar *s)
{
  const gchar *v1;
  gunichar v2;
  gboolean v3;
  gboolean result;

  if ( s && *s )
  {
    v1 = s;
    do
    {
      v2 = g_utf8_get_char(v1);
      v3 = g_unichar_isupper(v2);
      if ( !v3 )
        break;
      v1 = g_utf8_next_char(v1);
      if ( !v1 )
        break;
    }
    while ( *v1 );
    result = v3;
  }
  else
  {
    result = 0;
  }
  return result;
}

gchar *hildon_im_word_completer_get_one_candidate(HildonIMWordCompleter *wc, const gchar *previous_word, const gchar *current_word)
{
  gchar *curtext;
  gchar *rv;
  HildonIMWordCompleterPrivate *priv;
  gchar *prevtext;
  glong len;
  int i;
  imengines_wp_candidates candidates={0,};

  priv = HILDON_IM_WORD_COMPLETER_GET_PRIVATE (wc);

  len = g_utf8_strlen(current_word, -1);
  if ( previous_word )
    prevtext = g_utf8_strdown(previous_word, -1);
  else
    prevtext = 0;
  if ( current_word )
    curtext = g_utf8_strdown(current_word, -1);
  else
    curtext = 0;
  if ( !imengines_wp_get_candidates(prevtext, curtext, &candidates) && candidates.number_of_candidates > 0 )
  {
    i = 0;
    while ( 1 )
    {
      if ( g_utf8_strlen(candidates.candidate[i], -1) - len < priv->max_suffix )
      {
        rv = 0;
      }
      else
      {
        if ( str_contains_uppercase(current_word) && g_utf8_strlen(current_word, -1) > 1 )
          rv = g_utf8_strup(candidates.candidate[i], -1);
        else
          rv = g_strdup(candidates.candidate[i]);
        if ( rv )
          goto LABEL_7;
      }
      ++i;
      if ( candidates.number_of_candidates <= i )
        goto LABEL_7;
    }
  }
  rv = 0;
LABEL_7:
  g_free(prevtext);
  g_free(curtext);
  return rv;
}

gboolean hildon_im_word_completer_is_interesting_key(HildonIMWordCompleter *wc, const gchar *key)
{
  gboolean result;

  if ( g_strcmp0(key, "/apps/osso/inputmethod/dual-dictionary")
    && g_strcmp0(key, "/apps/osso/inputmethod/hildon-im-languages/current") )
  {
    result = g_str_has_prefix(key, "/apps/osso/inputmethod/hildon-im-languages");
    if ( result )
      result = g_str_has_suffix(key, "dictionary") != 0;
  }
  else
  {
    result = TRUE;
  }
  return result;
}

void
hildon_im_word_completer_remove_word(HildonIMWordCompleter *wc,
                                     const gchar *word)
{
  gboolean exists = FALSE;

  if (imengines_wp_word_exists(word, 1, &exists))
    imengines_wp_delete_word(word, 1, exists);
}

gboolean hildon_im_word_completer_add_to_dictionary(HildonIMWordCompleter *wc, const gchar *word)
{
  gboolean exists;
  if (imengines_wp_word_exists(word, 1, &exists))
  {
    imengines_wp_delete_word(word, 1, exists);
  }
  else if (!imengines_wp_word_exists(word, 0, &exists))
  {
    return TRUE;
  }
  imengines_wp_add_word(word, 2, exists);
  return TRUE;
}

gchar **hildon_im_word_completer_get_candidates(HildonIMWordCompleter *wc, const gchar *previous_word, const gchar *current_word)
{
  gchar **rv;
  int i;
  int j;
  gchar *curtext;
  gchar *prevtext;
  imengines_wp_candidates candidates={0,};

  if ( previous_word )
    prevtext = g_utf8_strdown(previous_word, -1);
  else
    prevtext = 0;
  if ( current_word )
    curtext = g_utf8_strdown(current_word, -1);
  else
    curtext = 0;
  if ( imengines_wp_get_candidates(prevtext, curtext, &candidates) && candidates.number_of_candidates > 0 )
  {
    rv = 0;
  }
  else
  {
    rv = g_new(gchar *, candidates.number_of_candidates + 1);
    if (candidates.number_of_candidates > 0)
    {
      i = 1;
      do
      {
        if (str_contains_uppercase(current_word) && g_utf8_strlen(current_word, -1) > 1)
          rv[i - 1] = g_strdup(candidates.candidate[i - 1]);
        else
          rv[i - 1] = g_utf8_strup(candidates.candidate[i - 1], -1);
        j = i++;
      } while (candidates.number_of_candidates > j);
    }
  }
  g_free(prevtext);
  g_free(curtext);
  return rv;
}
