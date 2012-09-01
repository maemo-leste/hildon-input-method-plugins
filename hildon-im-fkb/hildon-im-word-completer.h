/**
   @file hildon-im-word-completer.h

   Copyright (C) 2012 Ivaylo Dimitrov <freemangordon@abv.bg>

   This file is part of hildon-input-method-plugins.

   hildon-im-word-completer.h is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation.

   hildon-im-word-completer.h is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with hildon-input-method-plugins. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __HILDON_IM_WORD_COMPLETER_H_INCLUDED__
#define __HILDON_IM_WORD_COMPLETER_H_INCLUDED__

#define HILDON_IM_WORD_COMPLETER_TYPE (hildon_im_word_completer_get_type())

#define HILDON_IM_WORD_COMPLETER(obj) GTK_CHECK_CAST(obj, hildon_im_word_completer_get_type(), HildonIMWWordCompleter)
#define HILDON_IM_WORD_COMPLETER_CLASS(klass) \
        GTK_CHECK_CLASS_CAST(klass, hildon_im_word_completer_get_type, \
                             HildonIMWWordCompleterClass)
#define HILDON_IM_IS_WORD_COMPLETER(obj) \
        GTK_CHECK_TYPE(obj, HILDON_IM_WORD_COMPLETER_TYPE )
#define HILDON_IM_WORD_COMPLETER_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HILDON_IM_WORD_COMPLETER_TYPE,\
                                      HildonIMWWordCompleterPrivate))

typedef struct _HildonIMWWordCompleterPrivate HildonIMWWordCompleterPrivate;

typedef struct {
  GObject parent;
  HildonIMWWordCompleterPrivate * priv;
} HildonIMWWordCompleter;

typedef struct {
  GObjectClass parent;
} HildonIMWWordCompleterClass;


gpointer hildon_im_word_completer_new(void);

void hildon_im_word_completer_save_data(HildonIMWWordCompleter *hwc);

void hildon_im_word_completer_configure(HildonIMWWordCompleter *hwc, HildonIMUI *ui);

gboolean hildon_im_word_completer_hit_word(HildonIMWWordCompleter *wc, const gchar *str, gboolean unk);

gchar *hildon_im_word_completer_get_predicted_suffix(HildonIMWWordCompleter *wc, gchar *previous_word, const gchar *current_word, gchar **out);

gchar *hildon_im_word_completer_get_one_candidate(HildonIMWWordCompleter *wc, const gchar *previous_word, const gchar *current_word);

gboolean hildon_im_word_completer_is_interesting_key(HildonIMWWordCompleter *wc, const gchar *key);

void hildon_im_word_completer_remove_word(HildonIMWWordCompleter *wc, const gchar *word);

void hildon_im_word_completer_save_data(HildonIMWWordCompleter *wc);

#endif /* __HILDON_IM_WORD_COMPLETER_H_INCLUDED__ */
