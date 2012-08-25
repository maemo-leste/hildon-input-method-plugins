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

gchar *hildon_im_word_completer_get_predicted_suffix(HildonIMWWordCompleter *wc, gchar *unk, const char *s, gchar **out);

gchar *hildon_im_word_completer_get_one_candidate(HildonIMWWordCompleter *wc, const gchar *unk, const gchar *p);
#endif /* __HILDON_IM_WORD_COMPLETER_H_INCLUDED__ */
