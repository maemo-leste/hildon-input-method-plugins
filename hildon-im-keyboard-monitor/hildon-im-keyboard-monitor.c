/**
   @file hildon-im-keyboard-monitor.c

   This file is part of hildon-input-method-plugins.

   This file may contain parts derived by disassembling of binaries under
   Nokia's copyright, see http://tablets-dev.nokia.com/maemo-dev-env-downloads.php

   The original licensing conditions apply to all those derived parts as well
   and you accept those by using this file.
*/

#include <string.h>
#include <libintl.h>
#include <locale.h>
#include <glib.h>

#include <gtk/gtkbox.h>

#include <hildon/hildon.h>

#include <hildon-im-widget-loader.h>
#include <hildon-im-plugin.h>
#include <hildon-im-ui.h>
#include <hildon-im-languages.h>
#include <hildon-im-common.h>

#include <hildon-im-vkbrenderer.h>

#include "hildon-im-keyboard-monitor.h"
#include "hildon-im-western-plugin-common.h"
#include "hildon-im-xkb.h"
