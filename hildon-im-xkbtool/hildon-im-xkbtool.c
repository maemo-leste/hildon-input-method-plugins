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
#include <X11/extensions/XInput.h>

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
  g_type_init();
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
  g_type_init();
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
  const char *d;
  GOptionContext *context;
  gchar *newmodel;
  gchar *newlayout;
  gchar *int_kb_model;
  gchar *int_kb_layout;
  gint int_kb_repeat_delay;
  gint int_kb_repeat_interval;
  gchar *ext_kb_model;
  gchar *ext_kb_layout;
  gint ext_kb_repeat_delay;
  gint ext_kb_repeat_interval;
  int result;
  int reason;
  int err_rtrn;
  int ev_rtrn;
  GError *error;

  error = 0;
  d = getenv("DISPLAY");
  if ( !d )
    d = ":0.0";
  xdisplay = XkbOpenDisplay((char *)d, &ev_rtrn, &err_rtrn, 0, 0, &reason);
  if ( !xdisplay )
  {
    g_log(0, G_LOG_LEVEL_WARNING, "Couldn't open display %d", reason);
    return -1;
  }
  XQueryInputVersion(xdisplay, 2, 0);
  context = g_option_context_new("- Hildon IM XKB configurator");
  g_option_context_add_main_entries(context, options, 0);
  g_option_context_parse(context, &argc, (gchar ***)&argv, &error);
  if ( error )
  {
    g_log(0, G_LOG_LEVEL_WARNING, "%s\n", error->message);
    return -1;
  }
  if ( id != -1 && !name )
    name = hildon_im_xkb_get_name();
  if ( get_conf )
  {
    int_kb_model = gconf_get_string("/apps/osso/inputmethod/int_kb_model");
    int_kb_layout = gconf_get_string("/apps/osso/inputmethod/int_kb_layout");
    int_kb_repeat_delay = gconf_get_int("/apps/osso/inputmethod/int_kb_repeat_delay");
    int_kb_repeat_interval = gconf_get_int("/apps/osso/inputmethod/int_kb_repeat_interval");
    g_print("Internal keyboard:\n");
    g_print(
      "Model: %s\nLayout: %s\nDelay: %d\nInterval: %d\n\n",
      int_kb_model,
      int_kb_layout,
      int_kb_repeat_delay,
      int_kb_repeat_interval);
    ext_kb_model = gconf_get_string("/apps/osso/inputmethod/ext_kb_model");
    ext_kb_layout = gconf_get_string("/apps/osso/inputmethod/ext_kb_layout");
    ext_kb_repeat_delay = gconf_get_int("/apps/osso/inputmethod/ext_kb_repeat_delay");
    ext_kb_repeat_interval = gconf_get_int("/apps/osso/inputmethod/ext_kb_repeat_interval");
    g_print("External keyboard:\n");
    g_print(
      "Model: %s\nLayout: %s\nDelay: %d\nInterval: %d\n",
      ext_kb_model,
      ext_kb_layout,
      ext_kb_repeat_delay,
      ext_kb_repeat_interval);
    g_free(ext_kb_model);
    g_free(ext_kb_layout);
  }
  if ( set_conf )
  {
    newmodel = gconf_get_string("/apps/osso/inputmethod/int_kb_model");
    newlayout = gconf_get_string("/apps/osso/inputmethod/int_kb_layout");
    if ( newmodel == 0 || newlayout == 0)
      g_print("Error getting default layout values\n");
    else
      hildon_im_xkb_set_map(newmodel, newlayout, 0);
    g_free(newmodel);
    g_free(newlayout);
  }
  if ( !layout )
  {
    if ( !model )
      goto LABEL_14;
LABEL_19:
    g_print("Please specify both layout and model\n");
    exit(-1);
  }
  if ( !model )
    goto LABEL_19;
  hildon_im_xkb_set_map(model, layout, name);
LABEL_14:
  if ( delay )
  {
    if ( !interval )
    {
LABEL_16:
      g_print("Please specify both delay and interval\n");
      exit(-1);
    }
    hildon_im_xkb_set_rate(delay, interval, name);
  }
  else if ( interval )
  {
    goto LABEL_16;
  }
  result = list;
  if ( list )
  {
    hildon_im_xkb_print_devices();
    result = 0;
  }
  return result;
}
