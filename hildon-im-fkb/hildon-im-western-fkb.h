/**
   @file hildon-im-western-fkb.h

   Copyright (C) 2012 Ivaylo Dimitrov <freemangordon@abv.bg>

   This file is part of hildon-input-method-plugins.

   hildon-im-western-fkb.h is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation.

   hildon-im-western-fkb.h is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with hildon-input-method-plugins. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __HILDON_IM_WESTERN_FKB_H__
#define __HILDON_IM_WESTERN_FKB_H__

#define HILDON_IM_WESTERN_FKB_TYPE (hildon_im_western_fkb_get_type())

#define HILDON_IM_WESTERN_FKB(obj) GTK_CHECK_CAST(obj, hildon_im_western_fkb_get_type(), HildonIMWesternFKB)
#define HILDON_IM_WESTERN_FKB_CLASS(klass) \
        GTK_CHECK_CLASS_CAST(klass, hildon_im_western_fkb_get_type, \
                             HildonIMWesternFKBClass)
#define HILDON_IM_IS_WESTERN_FKB(obj) \
        GTK_CHECK_TYPE(obj, HILDON_IM_WESTERN_FKB_TYPE )

#define HILDON_IM_WESTERN_FKB_WIDTH 800
#define HILDON_IM_WESTERN_FKB_HEIGHT 210

typedef struct {
  GtkContainer parent;
} HildonIMWesternFKB;

typedef struct {
  GtkContainerClass parent;
} HildonIMWesternFKBClass;

#define NUM_LANGUAGES 2

#endif /* __HILDON_IM_WESTERN_FKB_H__ */
