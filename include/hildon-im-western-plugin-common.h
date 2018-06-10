/**
   @file hildon-im-western-plugin-common.h

   Copyright (C) 2012 Ivaylo Dimitrov <freemangordon@abv.bg>

   This file is part of hildon-input-method-plugins.

   hildon-im-western-plugin-common.h is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation.

   hildon-im-western-plugin-common.h is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with hildon-input-method-plugins. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __HILDON_IM_WESTERN_PLUGIN_COMMON_H_INCLUDED__
#define __HILDON_IM_WESTERN_PLUGIN_COMMON_H_INCLUDED__

gchar* get_gconf_path(HildonIMUI *ui, gchar *outbuf, const gchar *lang);

gchar *get_gconf_path_with_language(HildonIMUI *ui, gchar *outbuf, const gchar *path, const gchar *lang);

gboolean char_is_part_of_dictionary_word(const gchar *c);

gboolean get_current_bool_setting(HildonIMUI *ui, unsigned int which, int index);

gchar *get_wp_setting(HildonIMUI *ui, gchar *outbuf, gint index);

gchar *get_current_wp_setting(HildonIMUI *ui, gchar *outbuf);

gboolean get_use_dictionary(HildonIMUI *ui, int index);

GList *utf8_split_in_words(const gchar *text, gint maxlen);

gboolean unichar_in_str(gunichar c, const gchar **str, guint len);

gboolean word_prefix_starts_with_mark(const gchar *p);

gboolean word_is_valid_dictionary_word(const gchar *word);

#endif /* __HILDON_IM_WESTERN_PLUGIN_COMMON_H_INCLUDED__ */
