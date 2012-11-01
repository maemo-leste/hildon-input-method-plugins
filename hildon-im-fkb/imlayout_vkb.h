/**
  @file imlayout_vkb.h

  Copyright (C) 2011 Jonathan Wilson

  @author Jonathan wilson <jfwfreo@tpgi.com.au>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 as
  published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
typedef struct _vkb_key vkb_key;
struct _vkb_key
{
	unsigned char gtk_state;
	unsigned char unk1;
	unsigned short key_flags;
	unsigned char current_slide_key;
	unsigned int unk3;
	unsigned char byte_count;
	char *label;
	unsigned int special_font;
	unsigned char key_type;
	unsigned int left;
	unsigned int top;
	unsigned int right;
	unsigned int bottom;
	unsigned int unk9;
	unsigned char offset;
	unsigned char scancode_length;
	unsigned char unk12;
	unsigned char unk13;
	unsigned char *scancode;
	unsigned char num_sub_keys;
	vkb_key *sub_keys;
	unsigned char key_size;
} ;
#define KEY_TYPE_NORMAL 0
#define KEY_TYPE_SLIDING 1
#define KEY_TYPE_MODIFIER 2
#define KEY_TYPE_MULTIPLE 4
#define KEY_TYPE_ALPHA          0x01
#define KEY_TYPE_NUMERIC        0x02
#define KEY_TYPE_HEXA           0x04
#define KEY_TYPE_TELE           0x08
#define KEY_TYPE_SPECIAL        0x10
#define KEY_TYPE_DEAD           0x20
#define KEY_TYPE_WHITESPACE     0x40
#define KEY_TYPE_EXTRA_BYTE     0x80
#define KEY_TYPE_RAW            ((0x01<<8) | KEY_TYPE_EXTRA_BYTE)
#define KEY_TYPE_TONE           ((0x02<<8) | KEY_TYPE_EXTRA_BYTE)
#define KEY_TYPE_TAB            ((0x04<<8) | KEY_TYPE_EXTRA_BYTE)
#define KEY_TYPE_BACKSPACE      ((0x08<<8) | KEY_TYPE_EXTRA_BYTE)
#define KEY_TYPE_SHIFT          ((0x10<<8) | KEY_TYPE_EXTRA_BYTE)

typedef struct _vkb_key_section
{
	unsigned int num_keys;
	unsigned char num_rows;
	unsigned char *num_keys_in_rows;
	vkb_key *keys;
	unsigned char margin_left;
	unsigned char margin_top;
	unsigned char margin_bottom;
	unsigned char margin_right;
	unsigned int width;
	unsigned int unk0;
} vkb_key_section;

typedef struct _vkb_sub_layout
{
	unsigned int type;
	unsigned char variance_index;
	unsigned char num_key_sections;
	vkb_key_section *key_sections;
	char *label;
	unsigned int hash;
	unsigned int width;
	unsigned int height;
} vkb_sub_layout;
#define SUBLAYOUT_TYPE_LOWERCASE 0
#define SUBLAYOUT_TYPE_UPPERCASE 1
#define SUBLAYOUT_TYPE_SPECIAL 2

typedef struct _vkb_layout
{
	unsigned int type;
	unsigned int numeric;
	unsigned char num_sub_layouts;
	vkb_sub_layout *sub_layouts;
	unsigned char default_key_size;
} vkb_layout;
#define LAYOUT_TYPE_NORMAL 0
#define LAYOUT_TYPE_SPECIAL 1
#define LAYOUT_TYPE_PORTRAIT 2
#define LAYOUT_TYPE_LARGE 3
#define LAYOUT_TYPE_THUMB 4
#define LAYOUT_TYPE_PINYIN 5
#define LAYOUT_TYPE_ZHUYIN 6
#define LAYOUT_TYPE_CANGJIE 7
#define LAYOUT_TYPE_STROKE 8

typedef struct _vkb_key_size
{
	unsigned char width;
	unsigned char height;
	unsigned char baseline;
	unsigned char margin_left;
	unsigned char margin_top;
} vkb_key_size;

typedef struct _vkb_layout_collection
{
	char version;
	char num_layouts;
	char *name;
	char *lang;
	char *wc;
	char *filename;
	int *layout_types;
	int special_font;
	unsigned short *offsets;
	char num_key_sizes;
	vkb_key_size *key_sizes;
	char num_screen_modes;
	unsigned char *screen_modes;
} vkb_layout_collection;
#define SCREEN_TYPE_NORMAL 0
#define SCREEN_TYPE_THUMB 1
#define SCREEN_TYPE_ROTATED 2
typedef struct _vkb_keyboard_layout {
	int header_fd;
	int body_fd;
	vkb_layout_collection layout;
	vkb_layout *layouts;
	char *file;
	char *header_file;
	char *body_file;
	int is_key;
	int is_slide_key;
	int is_multiple_key;
	int parsing_multiple;
} vkb_keyboard_layout;
void imlayout_vkb_free_layout(vkb_layout *layout);
void imlayout_vkb_free_layout_collection(vkb_layout_collection *collection);
void imlayout_vkb_free_layout_list(GSList *list);
vkb_layout *imlayout_vkb_get_layout(vkb_layout_collection *collection, int layout_type);
GSList *imlayout_vkb_get_layout_list();
GSList *imlayout_vkb_get_layout_list_from_prefix(const gchar *path);
signed int imlayout_vkb_init_buttons(vkb_layout_collection *collection, vkb_layout *section, int width, int height);
vkb_layout_collection *imlayout_vkb_load_file(const char *fname);
void print_info(vkb_layout_collection *collection);
void print_layout_info(vkb_layout *layout);
void add_key(vkb_keyboard_layout *layout);
void add_keysection(vkb_keyboard_layout *layout_name);
void add_keysize(vkb_keyboard_layout *layout);
void add_layout(vkb_keyboard_layout *layout);
void add_rows(vkb_keyboard_layout *layout);
void add_screen(vkb_keyboard_layout *layout);
void add_slide(vkb_keyboard_layout *layout, const char *src, int len);
void add_sublayout(vkb_keyboard_layout *layout);
vkb_key *get_key(vkb_keyboard_layout *layout);
vkb_key *get_key0(vkb_keyboard_layout *layout, int no_subkey);
vkb_key_section *get_keysection(vkb_keyboard_layout *layout);
vkb_key_size *get_keysize(vkb_keyboard_layout *layout);
vkb_layout *get_layout(vkb_keyboard_layout *layout);
unsigned char *get_num_keys_in_row(vkb_keyboard_layout *layout);
unsigned char *get_screen(vkb_keyboard_layout *layout);
vkb_sub_layout *get_sublayout(vkb_keyboard_layout *layout);
void increase_num_keys_in_row(vkb_keyboard_layout *layout);
