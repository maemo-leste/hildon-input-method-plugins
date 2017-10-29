#ifndef __HILDON_IM_XKB_H_INCLUDED__
#define __HILDON_IM_XKB_H_INCLUDED__

void hildon_im_xkb_set_sticky(int sticky, const gchar *name);
gchar *hildon_im_xkb_get_name(void);
GSList *hildon_im_xkb_get_keyboard_names();
void hildon_im_xkb_print_devices();
void hildon_im_xkb_set_rate(gint delay, gint interval, const gchar *name);
void hildon_im_xkb_set_map(gchar *model, gchar *layout, const gchar *name);
const gchar *hildon_im_xkb_get_map();

#endif /* __HILDON_IM_XKB_H_INCLUDED__ */
