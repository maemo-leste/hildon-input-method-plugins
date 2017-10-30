/**
   @file hildon-im-keyboard-monitor.h

   Copyright (C) 2017 Jonathan Wilson <jfwfreo@tpgi.com.au> 
   This file is part of hildon-input-method-plugins.

   hildon-im-keyboard-monitor.h is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation.

   hildon-im-keyboard-monitor.h is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with hildon-input-method-plugins. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __HILDON_IM_KEYBOARD_MONITOR_H__
#define __HILDON_IM_KEYBOARD_MONITOR_H__

#define HILDON_IM_TYPE_KEYBOARD_MONITOR (hildon_im_keyboard_monitor_get_type())

#define HILDON_IM_KEYBOARD_MONITOR(obj) GTK_CHECK_CAST(obj, hildon_im_keyboard_monitor_get_type(), HildonIMKeyboardMonitor)
#define HILDON_IM_KEYBOARD_MONITOR_CLASS(klass) \
        GTK_CHECK_CLASS_CAST(klass, hildon_im_keyboard_monitor_get_type, \
                             HildonIMKeyboardMonitorClass)
#define HILDON_IM_IS_KEYBOARD_MONITOR(obj) \
        GTK_CHECK_TYPE(obj, HILDON_IM_TYPE_KEYBOARD_MONITOR)
#define HILDON_IM_KEYBOARD_MONITOR_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HILDON_IM_TYPE_KEYBOARD_MONITOR,\
                                      HildonIMKeyboardMonitorPrivate))

typedef struct _HildonIMKeyboardMonitorPrivate HildonIMKeyboardMonitorPrivate;

typedef struct {
  GtkVBox parent;
  HildonIMKeyboardMonitorPrivate *priv;
} HildonIMKeyboardMonitor;

typedef struct {
  GtkVBoxClass parent;
} HildonIMKeyboardMonitorClass;

#endif /* __HILDON_IM_KEYBOARD_MONITOR_H__ */
