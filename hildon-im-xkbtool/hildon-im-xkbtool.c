/**
   @file hildon-im-xkbtool.c

   This file is part of hildon-input-method-plugins.

   This file may contain parts derived by disassembling of binaries under
   Nokia's copyright, see http://tablets-dev.nokia.com/maemo-dev-env-downloads.php

   The original licensing conditions apply to all those derived parts as well
   and you accept those by using this file.
*/

#include <stdlib.h>
#include <glib.h>
#include <gconf/gconf-client.h>
#include "hildon-im-xkb.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XInput2.h>

static Display *xdisplay;
static gchar *name;
static gboolean get_conf;
static gboolean set_conf;
static gchar *layout;
static gchar *model;
static gint delay;
static gint interval;
static gboolean list;
static gint id = -1;
static const GOptionEntry options[12] = {
{"get-conf",'g',0,G_OPTION_ARG_NONE,&get_conf,"Show the currently stored configuration",0},
{"set-conf",'s',0,G_OPTION_ARG_NONE,&set_conf,"Apply the currently stored configuration",0},
{"layout",'l',0,G_OPTION_ARG_STRING,&layout,"Set the keyboard layout","LAYOUT"},
{"model",'m',0,G_OPTION_ARG_STRING,&model,"Set the keyboard model","MODEL"},
{"interval",'i',0,G_OPTION_ARG_INT,&interval,"Set the key repeat interval","INTERVAL"},
{"delay",'d',0,G_OPTION_ARG_INT,&delay,"Set the key repeat delay","DELAY"},
{"id",0,0,G_OPTION_ARG_INT,&id,"Apply to the keyboard with ID","ID"},
{"name",0,0,G_OPTION_ARG_STRING,&name,"Apply to the keyboard with NAME","NAME"},
{"list",0,0,G_OPTION_ARG_NONE,&list,"List available keyboards",0},
{0,0,0,0,0,0,0}};

static gchar *gconf_get_string(const gchar *key)
{
  GConfClient *client;
  gchar *str;
  GError *err;

  err = 0;
#if !GLIB_CHECK_VERSION(2,35,0)
  g_type_init ();
#endif
  client = gconf_client_get_default();
  if (!client)
    return 0;
  str = gconf_client_get_string(client, key, &err);
  if (err)
  {
    g_log(0, G_LOG_LEVEL_WARNING, "Error occured when getting %s from GConf.", key);
  }
  else if (!str)
  {
    g_log(0, G_LOG_LEVEL_WARNING, "Can not get value from Gconf %s . \n", key);
  }
  g_object_unref(client);
  return str;
}

static gint gconf_get_int(const gchar *key)
{
  GConfClient *client;
  gint i;
  GError *err;

  err = 0;
#if !GLIB_CHECK_VERSION(2,35,0)
  g_type_init ();
#endif
  client = gconf_client_get_default();
  if (!client)
    return 0;
  i = gconf_client_get_int(client, key, &err);
  if (err)
    g_log(0, G_LOG_LEVEL_WARNING, "Error occured when getting %s from GConf.", key);
  g_object_unref(client);
  return i;
}

int main(int argc, const char *argv[])
{
  GOptionContext *context;
  int reason;
  int err_rtrn;
  int ev_rtrn;
  int rc;
  int major = 2;
  int minor = 0;
  char *d = getenv("DISPLAY");
  GError *error = NULL;

  if (!d)
    d = ":0.0";

  xdisplay = XkbOpenDisplay(d, &ev_rtrn, &err_rtrn, NULL, NULL, &reason);

  if (!xdisplay)
  {
    g_warning("Couldn't open display %d", reason);
    return -1;
  }

  rc = XIQueryVersion(xdisplay, &major, &minor);

  if (rc == BadRequest)
  {
    g_warning("No XI2 support. (%d.%d only)\n", major, minor);
    return -1;
  }
  else if (rc != Success)
  {
    g_warning("Internal error\n");
    return -1;
  }

  context = g_option_context_new("- Hildon IM XKB configurator");
  g_option_context_add_main_entries(context, options, 0);
  g_option_context_parse(context, &argc, (gchar ***)&argv, &error);

  if (error)
  {
    g_warning("%s\n", error->message);
    return -1;
  }

  if (id != -1 && !name)
    name = hildon_im_xkb_get_name(id);

  if (get_conf)
  {
    gchar *kb_model = gconf_get_string(HILDON_IM_GCONF_INT_KB_MODEL);
    gchar *kb_layout = gconf_get_string(HILDON_IM_GCONF_INT_KB_LAYOUT);
    int repeat_delay = gconf_get_int(HILDON_IM_GCONF_INT_KB_REPEAT_DELAY);
    int repeat_interval = gconf_get_int(HILDON_IM_GCONF_INT_KB_REPEAT_INTERVAL);

    g_print("Internal keyboard:\n");
    g_print("Model: %s\nLayout: %s\nDelay: %d\nInterval: %d\n\n",
            kb_model, kb_layout, repeat_delay, repeat_interval);

    kb_model = gconf_get_string(HILDON_IM_GCONF_EXT_KB_MODEL);
    kb_layout = gconf_get_string(HILDON_IM_GCONF_EXT_KB_LAYOUT);
    repeat_delay = gconf_get_int(HILDON_IM_GCONF_EXT_KB_REPEAT_DELAY);
    repeat_interval = gconf_get_int(HILDON_IM_GCONF_EXT_KB_REPEAT_INTERVAL);
    g_print("External keyboard:\n");
    g_print("Model: %s\nLayout: %s\nDelay: %d\nInterval: %d\n",
            kb_model, kb_layout, repeat_delay, repeat_interval);
    g_free(kb_model);
    g_free(kb_layout);
  }

  if (set_conf)
  {
    gchar *kb_model = gconf_get_string(HILDON_IM_GCONF_INT_KB_MODEL);
    gchar *kb_layout = gconf_get_string(HILDON_IM_GCONF_INT_KB_LAYOUT);

    if (!kb_model || !kb_layout)
      g_print("Error getting default layout values\n");
    else
      hildon_im_xkb_set_map(kb_model, kb_layout, NULL);

    g_free(kb_model);
    g_free(kb_layout);
  }

  if (layout && !model)
  {
    g_print("Please specify both layout and model\n");
    exit(-1);
  }

  if (layout && model)
    hildon_im_xkb_set_map(model, layout, name);

  if (delay && !interval)
  {
    g_print("Please specify both delay and interval\n");
    exit(-1);
  }

  if (delay && interval)
    hildon_im_xkb_set_rate(delay, interval, name);

  if (list)
    hildon_im_xkb_print_devices();

  return 0;
}
