#ifndef __HILDON_IM_XKB_H_INCLUDED__
#define __HILDON_IM_XKB_H_INCLUDED__

#include <hildon-im-ui.h>

void hildon_im_xkb_set_sticky(int sticky, const gchar *name);
gchar *hildon_im_xkb_get_name(int id);
GSList *hildon_im_xkb_get_keyboard_names();
void hildon_im_xkb_print_devices();
void hildon_im_xkb_set_rate(gint delay, gint interval, const gchar *name);
void hildon_im_xkb_set_map(gchar *model, gchar *layout, const gchar *name);
const gchar *hildon_im_xkb_get_map();

#define HILDON_IM_GCONF_INT_KB_MODEL           HILDON_IM_GCONF_DIR "/int_kb_model"
#define HILDON_IM_GCONF_INT_KB_LAYOUT          HILDON_IM_GCONF_DIR "/int_kb_layout"
#define HILDON_IM_GCONF_INT_KB_REPEAT_DELAY    HILDON_IM_GCONF_DIR "/int_kb_repeat_delay"
#define HILDON_IM_GCONF_INT_KB_REPEAT_INTERVAL HILDON_IM_GCONF_DIR "/int_kb_repeat_interval"
#define HILDON_IM_GCONF_INT_KB_LEVEL_SHIFTED   HILDON_IM_GCONF_DIR "/int_kb_level_shifted"

#define HILDON_IM_GCONF_EXT_KB_MODEL           HILDON_IM_GCONF_DIR "/ext_kb_model"
#define HILDON_IM_GCONF_EXT_KB_LAYOUT          HILDON_IM_GCONF_DIR "/ext_kb_layout"
#define HILDON_IM_GCONF_EXT_KB_REPEAT_DELAY    HILDON_IM_GCONF_DIR "/ext_kb_repeat_delay"
#define HILDON_IM_GCONF_EXT_KB_REPEAT_INTERVAL HILDON_IM_GCONF_DIR "/ext_kb_repeat_interval"


#define HILDON_IM_GCONF_SLIDE_LAYOUT HILDON_IM_GCONF_DIR "/slide-layout"

#endif /* __HILDON_IM_XKB_H_INCLUDED__ */
