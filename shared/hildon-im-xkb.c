#include <glib.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#include "hildon-im-xkb.h"
Display *display;
int hildon_im_xkb_open_display(void)
{
  int result;
  int reason;
  int err_rtn;
  int ev_rtn;
  char *disp = getenv("DISPLAY");
  if (!disp)
    disp = ":0.0";
  display = XkbOpenDisplay(disp, &ev_rtn, &err_rtn, 0, 0, &reason);
  if (display)
  {
    XQueryInputVersion(display, 2, 0);
    XSynchronize(display, 1);
    result = 0;
  }
  else
  {
    g_log(0, G_LOG_LEVEL_WARNING, "Couldn't open display %d", reason);
    result = -1;
  }
  return result;
}

void hildon_im_xkb_set_sticky(int sticky, const gchar *name)
{
  assert(0);
  //todo
}
gchar *hildon_im_xkb_get_name(void)
{
  assert(0);
  return NULL;
  //todo
}
GSList *hildon_im_xkb_get_keyboard_names()
{
  assert(0);
  return NULL;
  //todo
}
void hildon_im_xkb_print_devices()
{
  XDeviceInfo *deviceinfo;
  int count;
  int ndevices;
  ndevices = 0;
  if (!hildon_im_xkb_open_display())
  {
    deviceinfo = XListInputDevices(display, &ndevices);
    if (deviceinfo)
    {
      if (ndevices > 0)
      {
        count = 0;
        do
        {
          g_print("ID %d, Name: \"%s\"\n", deviceinfo->id, deviceinfo->name);
          ++count;
          ++deviceinfo;
        } while (ndevices > count);
      }
      XFreeDeviceList(deviceinfo);
    }
    else
    {
      g_log(0, G_LOG_LEVEL_WARNING, "Couldn't get devices");
      ndevices = 0;
    }
    XCloseDisplay(display);
  }
}
void hildon_im_xkb_set_rate(gint delay, gint interval, const gchar *name)
{
  assert(0);
  //todo
}
void hildon_im_xkb_set_map(gchar *model, gchar *layout, const gchar *name)
{
  assert(0);
  //todo
}
const gchar *hildon_im_xkb_get_map()
{
  int group;
  XkbDescPtr desc;
  Atom atom;
  struct _XkbStateRec state;
  if (hildon_im_xkb_open_display())
    return 0;
  XkbGetState(display, 0x100, &state);
  group = state.group;
  desc = XkbGetMap(display, 2, 0x100);
  XkbGetNames(display, 0x1000, desc);
  atom = desc->names->groups[group];
  XkbFreeKeyboard(desc, 0xF8001FFF, 1);
  return XGetAtomName(display, atom);
}
