/**
   @file hildon-im-western-plugin-common.c

   Copyright (C) 2012 Ivaylo Dimitrov <freemangordon@abv.bg>

   This file is part of hildon-input-method-plugins.

   hildon-im-western-plugin-common.c is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation.

   hildon-im-western-plugin-common.c is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with hildon-input-method-plugins. If not, see <http://www.gnu.org/licenses/>.
*/

#include <glib.h>
#include <errno.h>
#include <string.h>


#include <hildon-im-ui.h>

#define SETTINGS_BOOL_MAX 5

gboolean gconf_bool_settings_need_prefix[]=
{
  FALSE,
  TRUE,
  TRUE,
  TRUE,
  TRUE
};

gboolean default_bool_settings[]=
{
  FALSE,
  TRUE,
  TRUE,
  TRUE,
  FALSE
};

const gchar *gconf_bool_settings[]=
{
  "/apps/osso/inputmethod/dual-dictionary",
  "/word-completion",
  "/auto-capitalisation",
  "/next-word-prediction",
  "/insert-space-after-word"
};

static const gchar *marks[]={"(",")","[","]","{","}","\"","\xC2\xBF","?","\xC2\xA1","!"};

gchar* get_gconf_path(HildonIMUI *ui, gchar *outbuf, const gchar *lang)
{
  g_snprintf(outbuf, 0xFFu, "%s/%s%s", "/apps/osso/inputmethod/hildon-im-languages", hildon_im_ui_get_active_language(ui), lang);
  return outbuf;
}

gchar *get_gconf_path_with_language(HildonIMUI *ui, gchar *outbuf, const gchar *path, const gchar *lang)
{
  g_snprintf(outbuf, 0xFFu, "%s/%s%s", "/apps/osso/inputmethod/hildon-im-languages", lang, path);
  return outbuf;
}

gboolean char_is_part_of_dictionary_word(const gchar *c)
{
  gunichar uc;

  if (c && *c)
  {
    uc = g_utf8_get_char_validated(c, -1);

    if ( (((int)uc) > -1) &&
         (g_unichar_isalnum(uc) || g_unichar_ismark(uc) || (*c == '-') || (*c == '_') || (*c == '\'')))
      return TRUE;
    else
      return (*c == '&');
  }
  return FALSE;
}

gboolean get_current_bool_setting(HildonIMUI *ui, unsigned int which, int index)
{
  GConfValue *value;
  gboolean rv;
  gchar key[256];

  g_return_val_if_fail(which >= 0 && which < SETTINGS_BOOL_MAX,FALSE);


  rv = default_bool_settings[which];
  if ( gconf_bool_settings_need_prefix[which] )
  {
    const gchar *ls = hildon_im_ui_get_language_setting(ui, index);

    if (ls)
      get_gconf_path_with_language(ui, key, gconf_bool_settings[which], ls);
    else
      get_gconf_path(ui, key, gconf_bool_settings[which]);

    value = gconf_client_get(ui->client, key, NULL);
  }
  else
    value = gconf_client_get(ui->client, gconf_bool_settings[which], NULL);

  if ( value )
    rv = gconf_value_get_bool(value);

  return rv;
}

gchar *get_wp_setting(HildonIMUI *ui, gchar *outbuf, gint index)
{
  const gchar *lang;
  GConfValue *value;
  gchar key[256];

  *outbuf = 0;

  lang = hildon_im_ui_get_language_setting(ui, index);

  if ( !lang || !*lang )
    return outbuf;

  get_gconf_path_with_language(ui, key, "/dictionary", lang);
  value = gconf_client_get(ui->client, key, NULL);

  if ( value )
  {
    lang = gconf_value_get_string(value);

    if ( !lang )
      return outbuf;
  }

  g_snprintf(outbuf, 0xFFu, "%s", lang);

  return outbuf;
}

gchar *get_current_wp_setting(HildonIMUI *ui, gchar *outbuf)
{
  return get_wp_setting(ui, outbuf, hildon_im_ui_get_active_language_index(ui));
}

gboolean get_use_dictionary(HildonIMUI *ui, int index)
{
  char outbuf[256];

  get_wp_setting(ui, outbuf, index);
  return outbuf[0] != 0;
}

GList *utf8_split_in_words(const gchar *text, gint maxlen)
{
  GList *list;
  GString *string;
  int i;

  if ( !text || !*text )
    return 0;

  list = NULL;
  string = NULL;
  i = 0;

  do
  {
    gunichar uc = g_utf8_get_char(text);
    ++i;

    if (g_unichar_isspace(uc) && string)
    {
      list = g_list_append(list, g_strdup(string->str));
      g_string_free(string, TRUE);
      string = NULL;
    }
    else
    {
      char buf[7]={0,};
      gssize len = g_unichar_to_utf8(uc, buf);

      if ( string )
        g_string_append_len(string, buf, len);
      else
        string = g_string_new_len(buf, len);
    }

    if (maxlen > 0 && i >= maxlen )
      break;

    text = g_utf8_next_char(text);
  }
  while ( *text );

  if ( !string )
    return list;


  list = g_list_append(list, g_strdup(string->str));
  g_string_free(string, TRUE);
  return list;
}

gboolean word_is_valid_dictionary_word(const gchar *word)
{
  if ( !word || !*word )
    return FALSE;

  if ( !g_unichar_isalpha(g_utf8_get_char(word)) )
    return FALSE;

  while ( 1 )
  {
    if ( !char_is_part_of_dictionary_word(word) )
      break;

    word = g_utf8_next_char(word);

    if ( !*word )
      return TRUE;
  }

  return FALSE;
}

gboolean unichar_in_str(gunichar c, const gchar **str, guint len)
{
  int i;

  if ( len )
  {
    i = 0;
    while ( 1 )
    {
      gchar * s = g_utf8_normalize(str[i], -1, G_NORMALIZE_DEFAULT);
      gunichar uc = g_utf8_get_char_validated(s, -1);
      g_free(s);

      if ( uc == c )
        return TRUE;

      i++;

      if ( i == len )
        break;
    }
  }

  return FALSE;
}

gboolean word_prefix_starts_with_mark(const gchar *p)
{
  const gchar *str[sizeof(marks)/sizeof(marks[0])];

  memcpy(str, marks, sizeof(str));
  return unichar_in_str(g_utf8_get_char(p), str, sizeof(marks)/sizeof(marks[0]));
}

