/**
   @file hildon-keyboard-assistant.h

   Copyright (C) 2017 Jonathan Wilson <jfwfreo@tpgi.com.au>

   This file is part of hildon-input-method-plugins.

   hildon-keyboard-assistant.h is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation.

   hildon-keyboard-assistant.h is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with hildon-input-method-plugins. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __HILDON_IM_KEYBOARD_ASSISTANT_H__
#define __HILDON_IM_KEYBOARD_ASSISTANT_H__

#define HILDON_IM_TYPE_KEYBOARD_ASSISTANT (hildon_im_keyboard_assistant_get_type()) 
#define HILDON_IM_KEYBOARD_ASSISTANT(obj) GTK_CHECK_CAST(obj, hildon_im_keyboard_assistant_get_type(), HildonIMKeyboardAssistant)
#define HILDON_IM_KEYBOARD_ASSISTANT_CLASS(klass) \
        GTK_CHECK_CLASS_CAST(klass, hildon_im_keyboard_assistant_get_type, \
                             HildonIMKeyboardAssistantClass)
#define HILDON_IM_IS_KEYBOARD_ASSISTANT(obj) \
        GTK_CHECK_TYPE(obj, HILDON_IM_TYPE_KEYBOARD_ASSISTANT )

typedef struct _HildonIMKeyboardAssistantPrivate HildonIMKeyboardAssistantPrivate;

typedef struct {
  GtkVBox parent;
} HildonIMKeyboardAssistant;

typedef struct {
  GtkVBoxClass parent;
} HildonIMKeyboardAssistantClass;

#endif /* __HILDON_IM_KEYBOARD_ASSISTANT_H__ */
