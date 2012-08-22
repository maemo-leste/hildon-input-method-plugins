#ifndef __HILDON_IM_WESTERN_PLUGIN_COMMON_H_INCLUDED__
#define __HILDON_IM_WESTERN_PLUGIN_COMMON_H_INCLUDED__

extern gboolean *gconf_bool_settings_need_prefix;
extern gboolean *default_bool_settings;

gboolean char_is_part_of_dictionary_word(const char *c);
gchar *get_gconf_path(HildonIMUI *ui, char *outbuf, char *path);
gchar *get_gconf_path_with_language(HildonIMUI *ui, char *outbuf, char *path, char *lang);
gchar *get_wp_setting(HildonIMUI *ui, char *outbuf, gint index);

gboolean get_current_bool_setting(HildonIMUI *ui, unsigned int which, int index);
gboolean get_use_dictionary(HildonIMUI *ui, int index);

gboolean unichar_in_str(gunichar uc, const gchar *ustr, guint len);
GList *utf8_split_in_words(const gchar *text_, guint maxlen);
gboolean word_is_valid_dictionary_word(const gchar *word);
gboolean word_prefix_starts_with_mark(const gchar *word);

#endif /* __HILDON_IM_WESTERN_PLUGIN_COMMON_H_INCLUDED__ */
