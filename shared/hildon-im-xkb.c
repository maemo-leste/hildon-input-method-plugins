#include <glib.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>
#include <X11/extensions/XInput2.h>

#include "hildon-im-xkb.h"

static Display *display;

int
hildon_im_xkb_open_display(void)
{
  int rv;
  int reason;
  int err_rtn;
  int ev_rtn;
  char *disp = getenv("DISPLAY");

  if (!disp)
    disp = ":0.0";

  display = XkbOpenDisplay(disp, &ev_rtn, &err_rtn, NULL, NULL, &reason);

  if (display)
  {
    int rc;
    int major = 2;
    int minor = 0;

    rc = XIQueryVersion(display, &major, &minor);

    if (rc == BadRequest)
    {
      g_warning("No XI2 support. (%d.%d only)\n", major, minor);
      rv = -1;
    }
    else if (rc != Success)
    {
      g_warning("Internal error\n");
      rv = -1;
    }
    else
    {
      XSynchronize(display, True);
      rv = 0;
    }
  }
  else
  {
    g_warning("Couldn't open display %d", reason);
    rv = -1;
  }

  return rv;
}

void
hildon_im_xkb_set_sticky(int sticky, const gchar *name)
{
  assert(0);
  //todo
}

gchar *
hildon_im_xkb_get_name(int id)
{
  gchar *name = NULL;
  XDeviceInfo *devices;
  int ndevices = 0;

  if (hildon_im_xkb_open_display())
    return NULL;

  devices = XListInputDevices(display, &ndevices);

  if (devices)
  {
    XDeviceInfo *device = devices;
    int i;

    for (i = 0; i < ndevices; i++)
    {
      if (device->id == id)
      {
        name = g_strdup(device->name);
        break;
      }

      device++;
    }

    XFreeDeviceList(devices);
  }
  else
    g_warning("Couldn't get devices");

  XCloseDisplay(display);

  return name;
}

GSList *
hildon_im_xkb_get_keyboard_names()
{
  assert(0);
  return NULL;
  //todo
}

void
hildon_im_xkb_print_devices()
{
  int ndevices = 0;

  if (!hildon_im_xkb_open_display())
  {
    XDeviceInfo *deviceinfo = XListInputDevices(display, &ndevices);

    if (deviceinfo)
    {
      if (ndevices > 0)
      {
        int i;

        for (i = 0; i < ndevices; i++)
        {
          g_print("ID %lu, Name: \"%s\"\n", deviceinfo->id, deviceinfo->name);
          deviceinfo++;
        }
      }

      XFreeDeviceList(deviceinfo);
    }
    else
      g_warning("Couldn't get devices");

    XCloseDisplay(display);
  }
}

void
hildon_im_xkb_set_rate(gint delay, gint interval, const gchar *name)
{
  assert(0);
  //todo
}

void
hildon_im_xkb_set_map(gchar *model, gchar *layout, const gchar *name)
{
  assert(0);
  //todo
}

const gchar *
hildon_im_xkb_get_map()
{
  XkbDescPtr desc;
  Atom atom;
  struct _XkbStateRec state;

  if (hildon_im_xkb_open_display())
    return 0;

  XkbGetState(display, XkbUseCoreKbd, &state);
  desc = XkbGetMap(display, XkbKeySymsMask, XkbUseCoreKbd);
  XkbGetNames(display, XkbGroupNamesMask, desc);
  atom = desc->names->groups[state.group];
  XkbFreeKeyboard(desc, 0, True);

  return XGetAtomName(display, atom);
}
