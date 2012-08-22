/**
   @file hildon-im-western-fkb.c

   Copyright (C) 2012 Ivaylo Dimitrov <freemangordon@abv.bg>

   This file is part of hildon-input-method-plugins.

   Libbme is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation.

   Libbme is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with libhildon-im-vkbrenderer3. If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <gtk/gtkwidget.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtktextbuffer.h>
#include <gtk/gtktextview.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkalignment.h>

#include <hildon/hildon.h>

#include <hildon-im-widget-loader.h>
#include <hildon-im-plugin.h>
#include <hildon-im-ui.h>
#include <hildon-im-languages.h>
#include <hildon-im-common.h>

#include <hildon-im-vkbrenderer.h>

#include "hildon-im-western-fkb.h"
#include "hildon-im-western-plugin-common.h"
#include "hildon-im-word-completer.h"

typedef struct {
  HildonIMUI *ui;
  GtkWindow *fkb_window;
  GtkWidget *vkb;
  GtkWidget *textview;
  PangoLayout *pango_layout;
  GtkTextBuffer *text_buffer;
  int field_18;
  unsigned int active_sub_layout;
  int field_20;
  gboolean shift_locked;
  HildonGtkInputMode current_input_mode;
  HildonGtkInputMode current_default_input_mode;
  gboolean input_mode_dictionary;
  int field_34;
  int field_38;
  gint offset;
  gchar *surrounding;
  gint surrounding_offset;
  gint active_language;
  gchar *language[NUM_LANGUAGES];
  HildonVKBRendererLayoutInfo *layout_info;

  GtkWidget *control_menu;
  GtkWidget *button_language[NUM_LANGUAGES];
  GtkWidget *button_common_menu_cut;
  GtkWidget *button_common_menu_copy;
  GtkWidget *button_common_menu_paste;
  GtkWidget *numbers_button;
  GtkWidget *menu_button;
  GtkWidget *enter_button;
  GtkWidget *repeating_button;

  int field_80;
  GtkWidget *pressed_repeating_button;
  int field_88;
  guint repeat_start_timer;
  gchar *predicted_suffix;
  gpointer field_94;
  gchar* predicted_word;
  gchar* field_9C;
  GtkTextTag *tag_fg;
  GtkTextTag *tag_bg;
  gboolean dual_dictionary;
  int field_AC;
  int field_B0;
  HildonIMWWordCompleter* hwc;
  gboolean auto_capitalisation;
  gboolean word_completion;
  gboolean insert_space_after_word;
  gboolean display_after_entering;
  GString *str;
  int field_CC;
} HildonIMWesternFKBPrivate;

static GType hildon_im_western_fkb_type = 0;
static GtkWidgetClass *parent_class = NULL;

static void hildon_im_western_fkb_class_init(HildonIMWesternFKBClass *klass);
static void hildon_im_western_fkb_init(HildonIMWesternFKB *fkb);
static void hildon_im_western_fkb_iface_init(HildonIMPluginIface *iface);
static void hildon_im_western_fkb_set_property(GObject *object,guint prop_id,const GValue *value,GParamSpec *pspec);
static void hildon_im_western_fkb_get_property (GObject *object,guint prop_id,GValue *value,GParamSpec *pspec);
static void hildon_im_western_fkb_finalize(GObject *obj);

static void hildon_im_western_fkb_enable(HildonIMPlugin *plugin, gboolean init);
static void hildon_im_western_fkb_disable(HildonIMPlugin *plugin);
static void hildon_im_western_fkb_destroy(GtkObject *object);
static void hildon_im_western_fkb_client_widget_changed(HildonIMPlugin *plugin);
static void hildon_im_western_fkb_select_region(HildonIMPlugin *plugin, gint start, gint end);
static void hildon_im_western_fkb_surrounding_received(HildonIMPlugin *plugin, const gchar *surrounding, gint offset);
static void hildon_im_western_fkb_transition(HildonIMPlugin *plugin, gboolean from);

/* helper functions */
static void fkb_window_create(HildonIMWesternFKB *fkb);
static void fkb_set_layout(HildonIMWesternFKB *fkb, unsigned int sub_layout);
static void fkb_delete_selection(HildonIMWesternFKB *fkb, gboolean clear_wc, gboolean redirect);
static void fkb_backspace(HildonIMWesternFKB *fkb, gboolean unk);
static void fkb_enter(HildonIMWesternFKB *self);

static void set_layout(HildonIMWesternFKB *fkb);
static void temp_text_clear(HildonIMWesternFKB *fkb);
static void insert_text_with_tag(HildonIMWesternFKB *fkb, GtkTextIter *iter, gchar *text, GtkTextTag *tag);

static gboolean word_completion_clear(HildonIMWesternFKB *fkb);
static void word_completion_reset(HildonIMWesternFKB *fkb);

/* callbacks */
static void update_input_mode_and_layout(HildonIMWesternFKB *self);

static void numbers_button_release(GObject *obj, void *data);
static gboolean delete_fkb_cb(GtkWidget *widget, GdkEvent *event, gpointer data);

static void menu_button_cb(GtkWidget *widget, void *data);
static void menu_item_selected(GtkWidget *button, gpointer data);
static void language_item_selected_cb(GtkWidget *button, gpointer data);
static void clipboard_targets_received_callback(GtkClipboard *clipboard, GdkAtom *atoms, gint n_atoms, gpointer data);
static void paste_received(GtkClipboard *clipboard, const gchar *text, gpointer data);

static void hildon_im_western_fkb_language_settings_changed(HildonIMPlugin *plugin, gint index);
static void hildon_im_western_fkb_language(HildonIMPlugin *plugin);
static void hildon_im_western_fkb_save_data(HildonIMPlugin *plugin);

static gboolean repeating_button_repeat_start(void *data);
static gboolean repeating_button_repeat(void *data);
static void repeating_button_pressed(GtkWidget *widget, void *data);
static void repeating_button_released(GtkWidget *widget, void *data);
static void repeating_button_enter(GtkWidget *widget, void *data);
static void repeating_button_leave(GtkWidget *widget, void *data);

static gboolean textview_key_press_release_cb(GtkWidget *widget, GdkEventKey *event, gpointer data);
static gboolean textview_button_release_cb(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean textview_button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data);

static void temp_input_cb(HildonVKBRenderer *vkb, gchar *input, gboolean unk, gpointer data);
static void illegal_input_cb(HildonVKBRenderer *vkb, gchar *input, gpointer data);
static void input(HildonVKBRenderer *vkb, gchar *input, gboolean unk, gpointer data);

/* the rest */
GType
hildon_im_western_fkb_get_type (void)
{
  return hildon_im_western_fkb_type;
}

static GObject *hildon_im_western_fkb_new(HildonIMUI *ui)
{
  return g_object_new(HILDON_IM_WESTERN_FKB_TYPE,
                                 HILDON_IM_PROP_UI_DESCRIPTION,
                                 ui,
                                 0);
}

HildonIMPlugin*
module_create (HildonIMUI *ui)
{
  return HILDON_IM_PLUGIN (hildon_im_western_fkb_new (ui));
}

void
module_exit(void)
{
  /* empty */
}

void
module_init(GTypeModule *module)
{
  static const GTypeInfo type_info = {
    sizeof(HildonIMWesternFKBClass),
    NULL, /* base_init */
    NULL, /* base_finalize */
    (GClassInitFunc) hildon_im_western_fkb_class_init,
    NULL, /* class_finalize */
    NULL, /* class_data */
    sizeof(HildonIMWesternFKB),
    0, /* n_preallocs */
    (GInstanceInitFunc) hildon_im_western_fkb_init,
    NULL
  };

  static const GInterfaceInfo plugin_info = {
    (GInterfaceInitFunc) hildon_im_western_fkb_iface_init,
    NULL, /* interface_finalize */
    NULL, /* interface_data */
  };

  hildon_im_western_fkb_type =
          g_type_module_register_type(module,
                                      GTK_TYPE_CONTAINER,
                                      "HildonIMWesternFKB",
                                      &type_info,
                                      0);

  g_type_module_add_interface(module,
                              HILDON_IM_WESTERN_FKB_TYPE,
                              HILDON_IM_TYPE_PLUGIN,
                              &plugin_info);
}

static void hildon_im_western_fkb_class_init(HildonIMWesternFKBClass *klass)
{
  GObjectClass *object_class;
  GtkObjectClass *gtk_object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;

  parent_class = g_type_class_peek_parent(klass);
  g_type_class_add_private(klass, sizeof(HildonIMWesternFKBPrivate));

  object_class = G_OBJECT_CLASS(klass);
  gtk_object_class = GTK_OBJECT_CLASS(klass);
  widget_class = GTK_WIDGET_CLASS(klass);
  container_class = GTK_CONTAINER_CLASS(klass);

  object_class->set_property = hildon_im_western_fkb_set_property;
  object_class->get_property = hildon_im_western_fkb_get_property;
  object_class->finalize = hildon_im_western_fkb_finalize;

  gtk_object_class->destroy = hildon_im_western_fkb_destroy;

  g_object_class_install_property(object_class, HILDON_IM_PROP_UI,
                                  g_param_spec_object(
                                    HILDON_IM_PROP_UI_DESCRIPTION,
                                    HILDON_IM_PROP_UI_DESCRIPTION,
                                    "UI that uses plugin",
                                    HILDON_IM_TYPE_UI,
                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
hildon_im_western_fkb_clear(HildonIMPlugin *plugin)
{
}
static void
hildon_im_western_fkb_iface_init(HildonIMPluginIface *iface)
{
  iface->enable = hildon_im_western_fkb_enable;
  iface->disable = hildon_im_western_fkb_disable;
  iface->clear = hildon_im_western_fkb_clear;
  iface->client_widget_changed = hildon_im_western_fkb_client_widget_changed;
  iface->save_data = hildon_im_western_fkb_save_data;
  iface->language_settings_changed = hildon_im_western_fkb_language_settings_changed;
  iface->language = hildon_im_western_fkb_language;
  iface->settings_changed = hildon_im_western_fkb_settings_changed;
  iface->select_region = hildon_im_western_fkb_select_region;
  iface->surrounding_received = hildon_im_western_fkb_surrounding_received;
  iface->transition = hildon_im_western_fkb_transition;
}

static void hildon_im_western_fkb_init(HildonIMWesternFKB *fkb)
{
  HildonIMWesternFKBPrivate *priv;
  GtkWidget *vkb;
  PangoFontDescription *font;
  GtkRequisition dimension;

  dimension.width = HILDON_IM_WESTERN_FKB_WIDTH;
  dimension.height = HILDON_IM_WESTERN_FKB_HEIGHT;

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE (HILDON_IM_WESTERN_FKB(fkb));


  GTK_OBJECT_FLAGS(GTK_OBJECT(GTK_WIDGET(fkb))) |= 0x20u;

  priv->language[0] = NULL;
  priv->language[1] = NULL;
  priv->field_18 = 0;
  priv->field_88 = 0;
  priv->repeat_start_timer = 0;
  priv->pressed_repeating_button = NULL;
  priv->layout_info = NULL;
  priv->active_sub_layout = 0;
  priv->surrounding = NULL;
  priv->surrounding_offset = 0;

  vkb = hildon_im_widget_load("vkbrenderer",
                              "vkb_renderer",
                              "dimension",
                              &dimension,
                              "repeat_interval",
                              175,
                              "gesture_range",
                              40,
                              NULL);

  priv->vkb = vkb;
  gtk_widget_set_name(GTK_WIDGET(vkb), "osso-im-fkb-renderer");
  g_object_set(priv->vkb, "style_normal", "hildon-im-button", NULL);
  g_object_set(priv->vkb, "style_special", "hildon-im-button", NULL);
  g_object_set(priv->vkb, "style_slide", "osso-im-fkb-slide-key", NULL);
  g_object_set(priv->vkb, "style_backspace", "hildon-im-backspace-button", NULL);
  g_object_set(priv->vkb, "style_shift", "hildon-im-shift-button", NULL);

  g_signal_connect_data(priv->vkb, "input", G_CALLBACK(input), fkb, 0, 0);
  g_signal_connect_data(priv->vkb, "temp-input", G_CALLBACK(temp_input_cb), fkb, 0, 0);
  g_signal_connect_data(priv->vkb, "illegal_input", G_CALLBACK(illegal_input_cb), fkb, 0, 0);

  priv->predicted_suffix = 0;
  priv->field_94 = 0;
  priv->predicted_word = NULL;
  priv->field_9C = NULL;
  priv->field_B0 = 0;

  priv->hwc = hildon_im_word_completer_new();
  g_object_set(priv->hwc, "max_candidates", 1, "min_candidate_suffix_length", 2, NULL);

  priv->pango_layout = pango_layout_new(gdk_pango_context_get());

  font = pango_font_description_from_string("Nokia Sans 28.5");
  pango_layout_set_font_description(priv->pango_layout, font);
  pango_font_description_free(font);

  priv->str = 0;
  priv->field_CC = 0;
}

static gint get_text_buffer_offset(HildonIMWesternFKB *fkb)
{
  HildonIMWesternFKBPrivate *priv;
  GtkTextMark * text_mark;
  GtkTextIter iter;

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);
  text_mark = gtk_text_buffer_get_selection_bound(priv->text_buffer);

  if ( text_mark )
  {
    gtk_text_buffer_get_iter_at_mark(priv->text_buffer, &iter, text_mark);
    return gtk_text_iter_get_offset(&iter);
  }
  return 0;
}

static gboolean get_text_bounds_from_tag(HildonIMWesternFKB *fkb, GtkTextIter *start, GtkTextIter *end, GtkTextTag *tag)
{
  HildonIMWesternFKBPrivate *priv;

  g_return_val_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb),FALSE);

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  gtk_text_buffer_get_start_iter(priv->text_buffer,start);

  if ( gtk_text_iter_begins_tag(start, tag) || gtk_text_iter_forward_to_tag_toggle(start, tag))
  {
    memcpy(end, start, sizeof(GtkTextIter));
    return gtk_text_iter_forward_to_tag_toggle(end, tag);
  }

  return FALSE;
}

static gboolean word_completion_get_bounds(HildonIMWesternFKB *fkb, GtkTextIter *start, GtkTextIter *end)
{
  g_return_val_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb),FALSE);

  return get_text_bounds_from_tag(fkb, end, start, HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb)->tag_fg);
}

static gboolean word_completion_clear(HildonIMWesternFKB *fkb)
{
  HildonIMWesternFKBPrivate *priv;
  GtkTextIter start;
  GtkTextIter end;
  gboolean rv = FALSE;

  g_return_val_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb),FALSE);

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  if(word_completion_get_bounds(fkb, &start, &end))
  {
    gtk_text_buffer_delete(priv->text_buffer, &start, &end);
    rv = TRUE;
  }

  g_free(priv->predicted_suffix);
  priv->predicted_suffix = NULL;

  g_free(priv->field_94);
  priv->field_94 = NULL;

  return rv;
}

static void word_completion_reset(HildonIMWesternFKB *fkb)
{
  HildonIMWesternFKBPrivate *priv;
  gint lang_index;
  const gchar *active_lang;
  const gchar *lang[2];

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);


  lang[0] = hildon_im_ui_get_language_setting(priv->ui, 0);
  lang[1] = hildon_im_ui_get_language_setting(priv->ui, 1);
  lang_index = hildon_im_ui_get_active_language_index(priv->ui);

  if ( lang_index > 1 )
    lang_index = 0;

  hildon_im_word_completer_configure(priv->hwc, priv->ui);

  active_lang = lang[lang_index];

  if (active_lang)
  {
    gchar *key;
    GConfValue *value;

    key = g_strdup_printf("/apps/osso/inputmethod/hildon-im-languages/%s/word-completion", active_lang);
    value = gconf_client_get(priv->ui->client, key, NULL);

    if (value)
    {
      priv->word_completion = gconf_value_get_bool(value);
      gconf_value_free(value);
    }

    g_free(key);

    key = g_strdup_printf("/apps/osso/inputmethod/hildon-im-languages/%s/auto-capitalisation", active_lang);
    value = gconf_client_get(priv->ui->client, key, NULL);

    if ( value ) /* possible bug? */
      gconf_value_free(value);

    g_free(key);

    key = g_strdup_printf("/apps/osso/inputmethod/hildon-im-languages/%s/insert-space-after-word", active_lang);
    value = gconf_client_get(priv->ui->client, key, NULL);

    if (value)
    {
      priv->field_AC = gconf_value_get_bool(value); /* possible bug? */
      gconf_value_free(value);
    }

    g_free(key);
  }


  word_completion_clear(fkb);
}

static void word_completion_hit_word(HildonIMWesternFKB *fkb, gchar *word)
{
  gchar *word_lc;
  HildonIMWesternFKBPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  if ( word && *word )
  {
    word_lc = g_utf8_strdown(word, -1);
    if ( priv->predicted_word && !g_strcmp0(word_lc, priv->predicted_word) )
    {
      g_free(word_lc);
    }
    else
    {
      hildon_im_word_completer_hit_word(priv->hwc, word_lc, 1);
      g_free(priv->predicted_word);
      priv->predicted_word = word_lc;
    }
  }
}

static gchar *get_input_text(HildonIMWesternFKB *fkb, gboolean no_autocomplete)
{
  HildonIMWesternFKBPrivate *priv;
  gchar *rv;
  GtkTextIter word_start,word_end;
  GtkTextIter start,end;

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  gtk_text_buffer_get_bounds(priv->text_buffer, &start, &end);
  if ( !no_autocomplete && word_completion_get_bounds(fkb, &word_end, &word_start) )
  {
    gchar *s1 = gtk_text_buffer_get_text(priv->text_buffer, &start, &word_end, 0);
    gchar *s2 = gtk_text_buffer_get_text(priv->text_buffer, &word_start, &end, 0);
    rv = g_strconcat(s1, s2, NULL);
    g_free(s1);
    g_free(s2);
  }
  else
  {
    rv = gtk_text_buffer_get_text(priv->text_buffer, &start, &end, 0);
  }
  return rv;
}

static void show_text_view(HildonIMWesternFKB *fkb)
{
  HildonIMWesternFKBPrivate *priv;
  gchar *text;
  PangoFontDescription *font;
  gint pixels_above_lines;
  int height;
  int width;

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);
  text = get_input_text(fkb, 1);
  pango_layout_set_text(priv->pango_layout, text, -1);
  pango_layout_get_pixel_size(priv->pango_layout, &width, &height);
  g_free(text);

  if ( priv->textview->allocation.width < width || priv->textview->allocation.height < height )
  {
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(priv->textview),
                                       gtk_text_buffer_get_selection_bound(priv->text_buffer));
    if ( priv->field_18 == 2 )
      return;

    font = pango_font_description_from_string("Nokia Sans 18");
    priv->field_18 = 2;
    pixels_above_lines = 0;
  }
  else
  {
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(priv->textview),
                                       gtk_text_buffer_get_selection_bound(priv->text_buffer));
    if ( priv->field_18 == 1 )
      return;

    font = pango_font_description_from_string("Nokia Sans 28.5");
    priv->field_18 = 1;
    pixels_above_lines = 10;
  }

  gtk_widget_modify_font(priv->textview, font);
  pango_font_description_free(font);

  gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(priv->textview),
                                       pixels_above_lines);

  gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(priv->textview),
                                     gtk_text_buffer_get_insert(priv->text_buffer));
}

static void
hildon_im_western_fkb_set_surrounding_offset(HildonIMWesternFKB *fkb)
{
  HildonIMWesternFKBPrivate *priv;
  gint offset;
  gboolean relative;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);
  offset = get_text_buffer_offset(fkb);

  if ( hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_REDIRECT )
  {
    offset -= priv->offset;
    relative = TRUE;
  }
  else
  {
    relative = FALSE;
  }

  hildon_im_ui_send_surrounding_offset(priv->ui, relative, offset);
}

static void
hildon_im_western_fkb_enable(HildonIMPlugin *plugin, gboolean init)
{
  HildonIMWesternFKBPrivate *priv;
  guint active_language_index;
  HildonGtkInputMode mode;
  HildonIMWesternFKB *fkb;
  guint index;
  GtkTextIter end;
  GtkTextIter start;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(plugin));

  fkb = HILDON_IM_WESTERN_FKB(plugin);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  g_free((gpointer)priv->predicted_suffix);
  priv->predicted_suffix = 0;
  priv->str = g_string_new("");
  priv->field_38 = 1;

  hildon_im_ui_parse_rc_file(priv->ui, "/usr/share/hildon-input-method/himrc");

  active_language_index = hildon_im_ui_get_active_language_index(priv->ui);
  if(active_language_index>1)
  {
    active_language_index = 0;
    index = 1;
  }
  else
    index = active_language_index?0:1;

  priv->dual_dictionary = get_current_bool_setting(priv->ui, 0, active_language_index);
  priv->auto_capitalisation = get_current_bool_setting(priv->ui, 2u, active_language_index);
  priv->insert_space_after_word = get_current_bool_setting(priv->ui, 4u, active_language_index);
  priv->word_completion = get_current_bool_setting(priv->ui, 1u, active_language_index) ||
                          (priv->dual_dictionary && get_current_bool_setting(priv->ui, 1u, index));

  priv->display_after_entering = gconf_client_get_int(
                                   priv->ui->client,
                                   "/apps/osso/inputmethod/display_after_entering",
                                   0);

  priv->current_input_mode = hildon_im_ui_get_current_input_mode(priv->ui);
  priv->current_default_input_mode = hildon_im_ui_get_current_default_input_mode(priv->ui);
  mode = priv->current_input_mode;

  priv->input_mode_dictionary =
      mode & HILDON_GTK_INPUT_MODE_INVISIBLE? FALSE : (mode&HILDON_GTK_INPUT_MODE_DICTIONARY) != 0;

  priv->field_34 = 0;

  if ( priv->fkb_window )
  {
    gtk_text_buffer_get_bounds(priv->text_buffer, &start, &end);
    gtk_text_buffer_delete(priv->text_buffer, &start, &end);
  }
  else
    fkb_window_create(fkb);

  hildon_vkb_renderer_clear_dead_key(HILDON_VKB_RENDERER(priv->vkb));
  hildon_im_western_fkb_language(plugin);

  gtk_widget_set_sensitive(priv->menu_button, TRUE);
  priv->offset = get_text_buffer_offset(fkb);

  gtk_widget_show_all(GTK_WIDGET(priv->fkb_window));

  gdk_window_set_transient_for(GTK_WIDGET(priv->fkb_window)->window,
                               gtk_widget_get_root_window(GTK_WIDGET(priv->fkb_window)));

  hildon_im_ui_play_sound(priv->ui, HILDON_IM_FINGER_TRIGGER_SOUND);
  pango_layout_set_width(priv->pango_layout, 1000 * priv->textview->allocation.width);
  pango_layout_set_wrap(priv->pango_layout, PANGO_WRAP_WORD_CHAR);
  hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_REQUEST_SURROUNDING_FULL);
  update_input_mode_and_layout(fkb);
  hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_CONFIRM_SENTENCE_START);
  hildon_im_western_fkb_set_surrounding_offset(fkb);
}

static gboolean
dialog_delete_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  return TRUE;
}
static GtkWidget*
fkb_create_control_menu(HildonIMWesternFKB *fkb)
{
  const gchar *first_lang;
  const gchar *second_lang;
  GtkBox *hbox;
  GtkBox *common_menu_box;
  GtkBox *language_box;
  HildonIMWesternFKBPrivate *priv;
  GtkWidget *dialog;

  g_return_val_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb),NULL);

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  dialog = gtk_dialog_new();

  gtk_window_set_title(GTK_WINDOW(dialog),
                       dcgettext(0, "inpu_nc_common_menu_method", LC_MESSAGES));

  g_object_ref(dialog);
  g_object_ref_sink(GTK_OBJECT(dialog));
  g_signal_connect_data(dialog, "delete-event", G_CALLBACK(dialog_delete_cb), 0, 0, 0);

  hbox = GTK_BOX(gtk_hbox_new(1, 10));

  common_menu_box = GTK_BOX(gtk_vbox_new(1, 0));
  language_box = GTK_BOX(gtk_vbox_new(1, 0));

  /* cut */
  priv->button_common_menu_cut = hildon_button_new_with_text(HILDON_SIZE_FINGER_HEIGHT, 0,
                                    dcgettext(0, "inpu_nc_common_menu_cut", LC_MESSAGES), 0);

  g_signal_connect_data(G_OBJECT(priv->button_common_menu_cut), "clicked", (GCallback)menu_item_selected, fkb, 0, 0);
  GTK_OBJECT(priv->button_common_menu_cut)->flags &= 0xFFFFF7FFu;
  gtk_box_pack_start(common_menu_box, priv->button_common_menu_cut, 1, 1, 0);

  /* copy */
  priv->button_common_menu_copy = hildon_button_new_with_text(HILDON_SIZE_FINGER_HEIGHT, 0,
                                    dcgettext(0, "inpu_nc_common_menu_copy", LC_MESSAGES), 0);
  g_signal_connect_data(G_OBJECT(priv->button_common_menu_copy), "clicked", (GCallback)menu_item_selected, fkb, 0, 0);

  GTK_OBJECT(priv->button_common_menu_copy)->flags &= 0xFFFFF7FFu;
  gtk_box_pack_start(common_menu_box, priv->button_common_menu_copy, 1, 1, 0);

  /* paste */
  priv->button_common_menu_paste = hildon_button_new_with_text(HILDON_SIZE_FINGER_HEIGHT, 0,
                                    dcgettext(0, "inpu_nc_common_menu_paste", LC_MESSAGES), 0);

  g_signal_connect_data(G_OBJECT(priv->button_common_menu_paste), "clicked", (GCallback)menu_item_selected, fkb, 0, 0);

  GTK_OBJECT(priv->button_common_menu_paste)->flags &= 0xFFFFF7FFu;
  gtk_box_pack_start(common_menu_box, priv->button_common_menu_paste, 1, 1, 0);

  /* language selection */
  first_lang = hildon_im_ui_get_language_setting(priv->ui, 0);
  second_lang = hildon_im_ui_get_language_setting(priv->ui, 1);

  if (first_lang && *first_lang && second_lang && *second_lang)
  {
    gchar *lang_desc;
    GtkWidget* radio_button;

    lang_desc = hildon_im_get_language_description(first_lang);
    priv->button_language[0] = radio_button =
        hildon_gtk_radio_button_new(HILDON_SIZE_FINGER_HEIGHT, NULL);

    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(radio_button), FALSE);
    gtk_button_set_label(GTK_BUTTON(radio_button), lang_desc);
    gtk_button_set_alignment(GTK_BUTTON(radio_button), 0.0, 0.5);
    GTK_OBJECT(radio_button)->flags &= ~0x80;

    gtk_box_pack_start(language_box, radio_button, 1, 1, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_button), (priv->active_language == 0));

    g_signal_connect_data(radio_button, "toggled", (GCallback)language_item_selected_cb, fkb, 0, 0);
    g_free(lang_desc);

    lang_desc = hildon_im_get_language_description(second_lang);

    priv->button_language[1] = radio_button =
        hildon_gtk_radio_button_new(HILDON_SIZE_FINGER_HEIGHT, gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_button)));;

    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(radio_button), FALSE);
    gtk_button_set_label(GTK_BUTTON(radio_button), lang_desc);
    gtk_button_set_alignment(GTK_BUTTON(radio_button), 0.0, 0.5);
    GTK_OBJECT(radio_button)->flags &= ~0x80;

    gtk_box_pack_start(language_box, radio_button, 1, 1, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_button), (priv->active_language == 1));
    g_signal_connect_data(radio_button, "toggled", (GCallback)language_item_selected_cb, fkb, 0, 0);
    g_free(lang_desc);

    gtk_box_pack_start(language_box, gtk_event_box_new(), 1, 1, 0);
  }
  else
  {
    priv->button_language[0]=
    priv->button_language[1]= NULL;
  }

  gtk_box_pack_start(hbox, GTK_WIDGET(common_menu_box), 1, 1, 0);
  gtk_box_pack_start(hbox, GTK_WIDGET(language_box), 1, 1, 0);

  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), GTK_WIDGET(hbox), 1, 1, 0);

  gtk_widget_show_all(GTK_DIALOG(dialog)->vbox);

  if (priv->current_input_mode & HILDON_GTK_INPUT_MODE_INVISIBLE)
  {
    gtk_widget_set_sensitive(priv->button_common_menu_cut, FALSE);
    gtk_widget_set_sensitive(priv->button_common_menu_copy, FALSE);
  }
  return dialog;
}

static void
repeating_button_connect_signals(HildonIMWesternFKB *fkb, GtkWidget *obj)
{
  g_signal_connect_data(G_OBJECT(obj), "pressed", (GCallback)repeating_button_pressed, fkb, 0, 0);
  g_signal_connect_data(G_OBJECT(obj), "released", (GCallback)repeating_button_released, fkb, 0, 0);
  g_signal_connect_data(G_OBJECT(obj), "enter", (GCallback)repeating_button_enter, fkb, 0, 0);
  g_signal_connect_data(G_OBJECT(obj), "leave", (GCallback)repeating_button_leave, fkb, 0, 0);
}

static void
fkb_window_create(HildonIMWesternFKB *fkb)
{
  HildonIMWesternFKBPrivate *priv;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *hbox2;
  GtkWidget *vbox2;
  GtkWidget *hbox1;
  GtkWidget *vbox1;

  GtkWidget *button;

  GtkWidget *alignment;
  GtkWidget *scrolled_window;
  GtkStyle *textviewstyle;

  GtkTextIter iter;

  GtkIconTheme *icon_theme;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  icon_theme = gtk_icon_theme_get_default(); /* unused, but still :) */

  priv->fkb_window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_type_hint(GTK_WINDOW(priv->fkb_window), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_decorated(GTK_WINDOW(priv->fkb_window), FALSE);
  gtk_widget_set_name(GTK_WIDGET(priv->fkb_window), "osso-im-fkb-window");
  gtk_widget_set_size_request(GTK_WIDGET(priv->fkb_window), -1, 500);

  g_signal_connect_data(G_OBJECT(priv->fkb_window), "delete-event", (GCallback)delete_fkb_cb, fkb, 0, 0);

  hbox = gtk_hbox_new(0, 0);
  gtk_container_add(GTK_CONTAINER(priv->fkb_window), hbox);

  vbox1 = gtk_vbox_new(0, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vbox1, 1, 1, 4u);

  vbox = gtk_vbox_new(0, 0);

  gtk_container_add(GTK_CONTAINER(vbox1), vbox);

  alignment = gtk_alignment_new(0.0, 0.0, 1.0, 1.0);
  gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 6u, 0, 0, 0);

  scrolled_window = gtk_scrolled_window_new(0, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  priv->textview = gtk_text_view_new();
  gtk_widget_set_name(priv->textview, "him-textview");
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(priv->textview), 10);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(priv->textview), GTK_WRAP_WORD_CHAR);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv->textview), TRUE);

  gtk_container_add(GTK_CONTAINER(scrolled_window), priv->textview);

  gtk_container_add(GTK_CONTAINER(alignment), scrolled_window);

  g_signal_connect_data(G_OBJECT(priv->textview), "key-press-event", (GCallback)textview_key_press_release_cb, fkb, 0, 0);
  g_signal_connect_data(G_OBJECT(priv->textview), "key-release-event", (GCallback)textview_key_press_release_cb, fkb, 0, 0);
  g_signal_connect_data(G_OBJECT(priv->textview), "button-press-event", (GCallback)textview_button_press_cb, fkb, 0, 0);
  g_signal_connect_data(G_OBJECT(priv->textview), "button-release-event", (GCallback)textview_button_release_cb, fkb, 0, G_CONNECT_AFTER);
  g_signal_connect_data(G_OBJECT(priv->textview), "drag-data-received", (GCallback)textview_drag_data_received, fkb, 0, 0);
  g_signal_connect_data(G_OBJECT(priv->textview), "drag-end", (GCallback)textview_drag_end_cb, fkb, 0, 0);

  gtk_container_add(GTK_CONTAINER(vbox), alignment);

  priv->text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview));

  textviewstyle = gtk_widget_get_style(priv->textview);
  if ( !textviewstyle )
    textviewstyle = gtk_widget_get_default_style();

  gtk_text_buffer_get_end_iter(priv->text_buffer, &iter);
  gtk_text_buffer_create_mark(priv->text_buffer, "completion", &iter, 0);
  priv->tag_fg = gtk_text_buffer_create_tag(
                  priv->text_buffer,
                  0,
                  "background-gdk", &textviewstyle->bg[3],
                  "foreground-gdk", &textviewstyle->fg[3],
                  "underline", 1,
                  "editable", 0,
                  NULL);
  priv->tag_bg = gtk_text_buffer_create_tag(
        priv->text_buffer,
        0,
        "foreground-gdk", &textviewstyle->bg[3],
        "editable", 0,
        NULL);

  hbox2 = gtk_hbox_new(0, 2);
  vbox2 = gtk_vbox_new(0, 0);
  priv->control_menu = fkb_create_control_menu(fkb);

  /* repeating */
  button = hildon_gtk_button_new(HILDON_SIZE_FINGER_HEIGHT);
  gtk_widget_set_size_request(button, 324, -1);
  gtk_widget_set_name(button, "hildon-im-alt-button");
  repeating_button_connect_signals(fkb, button);
  priv->repeating_button = button;
  gtk_box_pack_start(GTK_BOX(hbox2), button, 0, 0, 0);

  /* numbers */
  button = hildon_gtk_toggle_button_new(HILDON_SIZE_FINGER_HEIGHT);
  gtk_widget_set_size_request(button, 108, -1);
  gtk_widget_set_name(button, "hildon-im-alt-button");
  g_signal_connect_data(button, "released", (GCallback)numbers_button_release, fkb, 0, 0);
  hildon_helper_set_logical_font(button, "X-LargeSystemFont");
  priv->numbers_button = button;

  gtk_box_pack_start(GTK_BOX(vbox2), button, 1, 1, 0);
  gtk_box_pack_start(GTK_BOX(hbox2), vbox2, 0, 0, 0);

  /* menu */
  button = hildon_gtk_button_new(HILDON_SIZE_FINGER_HEIGHT);
  gtk_widget_set_size_request(button, 108, -1);
  gtk_widget_set_name(button, "hildon-im-alt-button");
  gtk_container_add(GTK_CONTAINER(button),
                    gtk_image_new_from_icon_name("keyboard_menu", (GtkIconSize)-1));
  g_signal_connect_data(button, "clicked", (GCallback)menu_button_cb, fkb, 0, 0);
  priv->menu_button = button;

  /* enter */
  button = hildon_gtk_button_new(HILDON_SIZE_FINGER_HEIGHT);
  gtk_widget_set_size_request(button, 108, -1);
  repeating_button_connect_signals(fkb, button);
  gtk_widget_set_name(button, "hildon-im-alt-button");
  gtk_container_add(GTK_CONTAINER(button),
                    gtk_image_new_from_icon_name("keyboard_enter", (GtkIconSize)-1));
  priv->enter_button = button;

  gtk_widget_set_size_request(priv->vkb, 800, 210);
  hbox1 = gtk_hbox_new(0, 0);

  gtk_box_pack_start(GTK_BOX(hbox1), priv->menu_button, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(hbox1), hbox2, 1, 0, 0);
  gtk_box_pack_start(GTK_BOX(hbox1), priv->enter_button, 0, 0, 0);

  gtk_container_set_border_width(GTK_CONTAINER(hbox1), 0);
  gtk_container_set_border_width(GTK_CONTAINER(hbox2), 0);
  gtk_widget_set_size_request(hbox1, -1, 70);

  gtk_box_pack_end(GTK_BOX(vbox1), hbox1, 0, 0, 0);
  gtk_box_pack_end(GTK_BOX(vbox1), priv->vkb, 0, 0, 0);

  gtk_widget_realize(GTK_WIDGET(priv->vkb));
}

static void
numbers_button_release(GObject *obj, void *data)
{
  HildonIMWesternFKB *fkb; // edi@5
  HildonIMWesternFKBPrivate *priv; // esi@5

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb));

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(obj)))
  {
    hildon_im_ui_set_level_locked(priv->ui, 1);
    priv->shift_locked = hildon_im_ui_get_shift_locked(priv->ui);
    hildon_im_ui_set_shift_locked(priv->ui, 0);
    hildon_im_ui_set_shift_sticky(priv->ui, 0);
  }
  else
  {
    hildon_im_ui_set_level_locked(priv->ui, 0);
    hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_LEVEL_UNLOCKED);
    hildon_im_ui_set_level_sticky(priv->ui, 0);
    hildon_im_ui_send_communication_message(priv->ui, 28);
    hildon_im_ui_set_shift_locked(priv->ui, priv->shift_locked);
  }
  set_layout(fkb);
}

static void
fkb_set_layout(HildonIMWesternFKB *fkb, unsigned int sub_layout)
{
  HildonIMWesternFKBPrivate *priv;
  gboolean numbers_button_active;
  gboolean shift_active;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  if ( priv->active_sub_layout != sub_layout )
  {
    shift_active = hildon_vkb_renderer_get_shift_active(HILDON_VKB_RENDERER(priv->vkb));

    hildon_vkb_renderer_set_shift_active(HILDON_VKB_RENDERER(priv->vkb),
                                           sub_layout == 1);
    if (sub_layout)
    {
      switch ( sub_layout )
      {
        case 1:
          g_object_set(priv->vkb, "sub", 1, NULL);
          numbers_button_active = FALSE;
          shift_active = TRUE;
          break;
        case 3:
          g_object_set(priv->vkb, "sub", 3, NULL);
          numbers_button_active = TRUE;
          shift_active = TRUE;
          break;
        case 2:
          g_object_set(priv->vkb, "sub", 2, NULL);
          numbers_button_active = TRUE;
          shift_active = FALSE;
          break;
        default:
          numbers_button_active = FALSE;
          break;
      }
    }
    else
    {
      g_object_set(priv->vkb, "sub", 0, NULL);
      numbers_button_active = FALSE;
      shift_active = FALSE;
    }
    hildon_vkb_renderer_set_shift_active(HILDON_VKB_RENDERER(priv->vkb), shift_active);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->numbers_button), numbers_button_active);
    priv->active_sub_layout = sub_layout;
    gtk_widget_queue_draw(priv->vkb);
  }
}

static void
set_layout(HildonIMWesternFKB *fkb)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMInternalModifierMask mask;

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  mask = hildon_im_ui_get_mask(priv->ui);

  if ( !(mask & HILDON_IM_LEVEL_STICKY_MASK) && !(mask & HILDON_IM_LEVEL_LOCK_MASK) )
  {
    if ( !(mask & HILDON_IM_SHIFT_STICKY_MASK) && !(mask & HILDON_IM_SHIFT_LOCK_MASK) )
    {
      if ( !priv->auto_capitalisation ||
           priv->field_20 ||
           !(priv->current_input_mode & HILDON_GTK_INPUT_MODE_AUTOCAP ) )
      {
        fkb_set_layout(fkb, 0);
        priv->field_20 = 0;
        return;
      }
      if ( !hildon_im_common_check_auto_cap(get_input_text(fkb, 0),
                                            get_text_buffer_offset(fkb)) )
      {
        fkb_set_layout(fkb, 0);
        return;
      }
    }
    fkb_set_layout(fkb, 1);
    return;
  }
  if ( mask & HILDON_IM_SHIFT_STICKY_MASK || mask & HILDON_IM_SHIFT_LOCK_MASK )
    fkb_set_layout(fkb, 3);
  else
    fkb_set_layout(fkb, 2);
}

static void
hildon_im_western_fkb_get_property (GObject *object,
                                    guint prop_id,
                                    GValue *value,
                                    GParamSpec *pspec)
{
  HildonIMWesternFKBPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(object));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(object);

  switch (prop_id)
  {
    case HILDON_IM_PROP_UI:
      g_value_set_object(value, priv->ui);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void
hildon_im_western_fkb_set_property(GObject *object,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  HildonIMWesternFKBPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(object));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(object);

  switch (prop_id)
  {
    case HILDON_IM_PROP_UI:
      priv->ui = g_value_get_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void hildon_im_western_fkb_finalize(GObject *object)
{
  HildonIMWesternFKBPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(object));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(object);

  if ( priv->hwc )
  {
    g_object_unref(priv->hwc);
    priv->hwc = NULL;
  }
  g_free((gpointer)priv->predicted_suffix);
  g_free((gpointer)priv->field_94);
  g_free((gpointer)priv->predicted_word);
  g_free((gpointer)priv->field_9C);

  if (G_OBJECT_CLASS(parent_class)->finalize)
    G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void hildon_im_western_fkb_destroy(GtkObject *object)
{
  HildonIMWesternFKBPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(object));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(object);

  hildon_im_western_fkb_save_data(HILDON_IM_PLUGIN(priv->ui));

  if ( priv->fkb_window )
  {
    gtk_widget_destroy(GTK_WIDGET(priv->fkb_window));
    priv->fkb_window = NULL;
  }
  if ( priv->control_menu )
  {
    gtk_widget_destroy(priv->control_menu);
    g_object_unref(priv->control_menu);
    priv->control_menu = NULL;
  }
  if ( priv->layout_info )
  {
    layout_info_free(priv->layout_info);
    priv->layout_info = NULL;
  }
  if ( priv->pango_layout )
  {
    g_object_unref(priv->pango_layout);
    priv->pango_layout = NULL;
  }

  if (GTK_OBJECT_CLASS(parent_class)->destroy)
    GTK_OBJECT_CLASS(parent_class)->destroy(object);
}

static void hildon_im_western_fkb_hide_fkb_window(HildonIMWesternFKB *fkb)
{
  HildonIMWesternFKBPrivate *priv;

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  gtk_widget_hide(GTK_WIDGET(priv->fkb_window));
  hildon_im_ui_restore_previous_mode(priv->ui);
}

static void
hildon_im_western_fkb_disable(HildonIMPlugin *plugin)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;

  gchar *surrounding;
  GtkTextIter end;
  GtkTextIter start;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(plugin));

  fkb = HILDON_IM_WESTERN_FKB(plugin);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(plugin);

  if (priv->repeat_start_timer)
  {
    g_source_remove(priv->repeat_start_timer);
    priv->repeat_start_timer = 0;
  }
  if ( priv->field_CC )
  {
    g_source_remove(priv->field_CC);
    priv->field_CC = 0;
  }
  if ((priv->current_input_mode & HILDON_GTK_INPUT_MODE_INVISIBLE) &&
       priv->str)
  {
    surrounding = priv->str->str;
    if ( !priv->field_38 )
      goto out;
  }
  else
  {
    surrounding = get_input_text(fkb, 0);
    word_completion_hit_word(fkb, priv->field_9C);
    if ( !priv->field_38 )
      goto out;
  }
  if (hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_SURROUNDING)
  {
    hildon_im_ui_send_surrounding_content(priv->ui, surrounding);
    hildon_im_western_fkb_set_surrounding_offset(fkb);
  }
out:
  gtk_text_buffer_get_start_iter(priv->text_buffer, &start);
  gtk_text_buffer_get_end_iter(priv->text_buffer, &end);
  gtk_text_buffer_delete(priv->text_buffer, &start, &end);
  if ( !hildon_im_ui_get_input_window(priv->ui) )
    hildon_im_western_fkb_hide_fkb_window(fkb);

  g_free((gpointer)priv->surrounding);
  priv->surrounding = NULL;
  priv->surrounding_offset = 0;

  if ( surrounding )
    g_free(surrounding);
  if ( priv->str )
  {
    g_string_free(priv->str, FALSE);
    priv->str = 0;
  }
  if ( hildon_im_ui_get_level_locked(priv->ui) )
  {
    hildon_im_ui_set_level_locked(priv->ui, FALSE);
    hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_LEVEL_UNLOCKED);
    hildon_im_ui_set_shift_locked(priv->ui, priv->shift_locked);
  }
  else
  {
    if ( hildon_im_ui_get_shift_locked(priv->ui) )
      hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_SHIFT_LOCKED);
    else
      hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_SHIFT_UNLOCKED);
  }
}

static void
hildon_im_western_fkb_client_widget_changed(HildonIMPlugin *plugin)
{
  HildonIMWesternFKB *fkb;
  HildonIMWesternFKBPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(plugin));

  fkb = HILDON_IM_WESTERN_FKB(plugin);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(plugin);

  temp_text_clear(fkb);
  word_completion_clear(fkb);

  g_free((gpointer)priv->predicted_suffix);
  priv->predicted_suffix = NULL;

  update_input_mode_and_layout(fkb);
  hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_REQUEST_SURROUNDING_FULL);
}


static
gboolean temp_text_get_bounds(HildonIMWesternFKB *fkb, GtkTextIter *start, GtkTextIter *end)
{
  HildonIMWesternFKBPrivate *priv;

  g_return_val_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb),FALSE);

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  return get_text_bounds_from_tag(fkb, start, end, priv->tag_bg);

}

static void
temp_text_clear(HildonIMWesternFKB *fkb)
{
  HildonIMWesternFKBPrivate *priv;
  GtkTextIter start;
  GtkTextIter end;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  if(temp_text_get_bounds(fkb, &start, &end))
    gtk_text_buffer_delete(priv->text_buffer, &start, &end);
}

static
gboolean delete_fkb_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  hildon_im_western_fkb_hide_fkb_window(HILDON_IM_WESTERN_FKB(data));
  return TRUE;
}

static void
menu_item_selected(GtkWidget *button, gpointer data)
{
  HildonIMWesternFKBPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(data);

  if ( priv->button_common_menu_cut == button )
    gtk_dialog_response(GTK_DIALOG(priv->control_menu), 0);
  else if (priv->button_common_menu_copy == button )
    gtk_dialog_response(GTK_DIALOG(priv->control_menu), 1);
  else if (priv->button_common_menu_paste == button )
    gtk_dialog_response(GTK_DIALOG(priv->control_menu), 2);
  else
    gtk_dialog_response(GTK_DIALOG(priv->control_menu), -1);
}

static void
language_item_selected_cb(GtkWidget *button, gpointer data)
{
  HildonIMWesternFKBPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(data);

  if ( priv->button_language[0] == button &&
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
    gtk_dialog_response(GTK_DIALOG(priv->control_menu), 3);
  else if ( priv->button_language[1] == button &&
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
    gtk_dialog_response(GTK_DIALOG(priv->control_menu), 4);
  else
    gtk_dialog_response(GTK_DIALOG(priv->control_menu), -1);
}

static void
hildon_im_western_fkb_language_settings_changed(HildonIMPlugin *plugin, gint index)
{
  HildonIMWesternFKBPrivate *priv;
  GtkWidget *button;
  const gchar *language;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(plugin));
  g_return_if_fail(index >= 0 && index < NUM_LANGUAGES);

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(plugin);

  language = hildon_im_ui_get_language_setting(priv->ui, index);
  if ( g_ascii_strcasecmp(priv->language[index], language) )
  {
    button = gtk_bin_get_child(GTK_BIN(index?
                                      priv->button_language[1]:
                                      priv->button_language[0]));

    if (GTK_IS_LABEL(button))
    {
      gchar* s= hildon_im_get_language_description(language);
      gtk_label_set_text(GTK_LABEL(button), s);
      g_free(s);
    }

    hildon_im_western_fkb_language(plugin);

    if ( priv->language[index] )
      g_free(priv->language[index]);
    priv->language[index] = g_strdup(language);

    hildon_im_word_completer_configure(priv->hwc, priv->ui);
  }
}

static void hildon_im_western_fkb_language(HildonIMPlugin *plugin)
{
  HildonIMWesternFKB *fkb;
  HildonIMWesternFKBPrivate *priv;
  const gchar* lang;
  gchar *vkb_file;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(plugin));

  fkb = HILDON_IM_WESTERN_FKB(plugin);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  lang = hildon_im_ui_get_active_language(priv->ui);
  if ( !priv->language[priv->active_language]
    || g_ascii_strcasecmp(lang, priv->language[priv->active_language]) )
  {
    priv->active_language = hildon_im_ui_get_active_language_index(priv->ui);
    vkb_file = g_strconcat("/usr/share/keyboards", "/", lang, ".vkb", NULL);
    g_object_set(priv->vkb,
                 "collection", vkb_file,
                 "layout", 4,
                 NULL);

    gtk_widget_queue_draw(priv->vkb);

    if ( priv->button_language[priv->active_language] )
    {
      if ( !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->button_language[priv->active_language])))
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->button_language[priv->active_language]), TRUE);
    }
    word_completion_reset(fkb);
    g_free(vkb_file);
  }
  if ( priv->language[0] )
    g_free(priv->language[0]);
  if ( priv->language[1] )
    g_free(priv->language[1]);

  priv->language[0] = g_strdup(hildon_im_ui_get_language_setting(priv->ui, 0));
  priv->language[1] = g_strdup(hildon_im_ui_get_language_setting(priv->ui, 1));

  if ( priv->layout_info )
  {
    layout_info_free(priv->layout_info);
    priv->layout_info = 0;
  }
  g_object_get(priv->vkb, "subs", &priv->layout_info, NULL);

  if ( priv->layout_info )
  {
    gtk_button_set_label(GTK_BUTTON(priv->numbers_button),
                         priv->layout_info->label[2]);
  }
}

static void
hildon_im_western_fkb_save_data(HildonIMPlugin *plugin)
{
  HildonIMWesternFKBPrivate *priv;
  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(plugin));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(HILDON_IM_WESTERN_FKB(plugin));

  if (priv->hwc)
    hildon_im_word_completer_save_data(priv->hwc);
}

static void
text_buffer_asterisk_fill(HildonIMWesternFKBPrivate *priv)
{
  glong len;
  gchar *str;

  len = g_utf8_strlen(priv->str->str, -1);
  str = (gchar *)g_malloc(len);
  memset(str, '*', len);
  gtk_text_buffer_set_text(priv->text_buffer, str, len);
  g_free(str);
  priv->field_CC = 0;
}

static void
hildon_im_western_fkb_completion_language_changed(HildonIMPlugin *plugin)
{
  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(plugin));
  word_completion_reset(HILDON_IM_WESTERN_FKB(plugin));
}

static void
hildon_im_western_fkb_select_region(HildonIMPlugin *plugin, gint start, gint end)
{
  HildonIMWesternFKBPrivate *priv;
  GtkTextIter iter_start;
  GtkTextIter iter_end;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(plugin));
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(HILDON_IM_WESTERN_FKB(plugin));

  gtk_text_buffer_get_iter_at_offset(priv->text_buffer, &iter_start, start);
  gtk_text_buffer_get_iter_at_offset(priv->text_buffer, &iter_end, end);
  gtk_text_buffer_select_range(priv->text_buffer, &iter_end, &iter_start);
}

static void hildon_im_western_fkb_surrounding_received(HildonIMPlugin *plugin, const gchar *surrounding, gint offset)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;
  GtkTextIter iter;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(plugin));

  fkb = HILDON_IM_WESTERN_FKB(plugin);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  g_free(priv->surrounding);
  priv->surrounding = g_strdup(surrounding);
  priv->surrounding_offset = offset;
  if (priv->current_input_mode & HILDON_GTK_INPUT_MODE_INVISIBLE )
  {
    gint len = g_utf8_strlen(surrounding, -1);
    gchar *str = g_malloc(len + 1);
    if ( str )
    {
      memset(str, '*', len);
      str[len] = 0;
      gtk_text_buffer_set_text(priv->text_buffer, str, -1);
    }
  }
  else
    gtk_text_buffer_set_text(priv->text_buffer, surrounding, -1);

  gtk_text_buffer_get_iter_at_offset(priv->text_buffer, &iter, offset);
  gtk_text_buffer_place_cursor(priv->text_buffer, &iter);

  show_text_view(fkb);

  if (priv->current_input_mode & HILDON_GTK_INPUT_MODE_INVISIBLE )
  {
    priv->str = g_string_assign(priv->str, surrounding);
    priv->auto_capitalisation = 0;
  }

  priv->field_20 = 0;
  priv->shift_locked = hildon_im_ui_get_shift_locked(priv->ui);

  if ( priv->active_sub_layout == 2 )
  {
    hildon_im_ui_set_level_locked(priv->ui, TRUE);
    hildon_im_ui_set_shift_locked(priv->ui, FALSE);
    set_layout(fkb);
  }
  else if ( priv->active_sub_layout == 3 )
  {
    hildon_im_ui_set_level_locked(priv->ui, TRUE);
    hildon_im_ui_set_shift_locked(priv->ui, TRUE);
    set_layout(fkb);
  }
  else
  {
    if ( hildon_im_ui_get_level_locked(priv->ui) || hildon_im_ui_get_level_sticky(priv->ui) )
    {
      hildon_im_ui_set_shift_locked(priv->ui, FALSE);
      hildon_im_ui_set_shift_sticky(priv->ui, FALSE);
    }
    set_layout(fkb);
  }
}

static void
hildon_im_western_fkb_transition(HildonIMPlugin *plugin, gboolean from)
{
  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(plugin));

  hildon_im_ui_clear_plugin_buffer(HILDON_IM_WESTERN_FKB_GET_PRIVATE(HILDON_IM_WESTERN_FKB(plugin))->ui);
}

static void insert_text_with_tag(HildonIMWesternFKB *fkb, GtkTextIter *iter, gchar *text, GtkTextTag *tag)
{
  HildonIMWesternFKBPrivate *priv;
  GtkTextIter tmp_iter;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  gtk_text_buffer_get_iter_at_mark(priv->text_buffer,
                                   &tmp_iter,
                                   gtk_text_buffer_get_insert(priv->text_buffer));

  gtk_text_buffer_insert_with_tags(priv->text_buffer, iter, text, -1, tag, NULL);
  gtk_text_buffer_get_iter_at_offset(priv->text_buffer, &tmp_iter, gtk_text_iter_get_offset(&tmp_iter));
  gtk_text_buffer_place_cursor(priv->text_buffer, &tmp_iter);

}

static void
temp_text_insert(HildonIMWesternFKB *fkb, GtkTextIter *iter, gchar *text)
{
  HildonIMWesternFKBPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  if ( gtk_text_buffer_get_selection_bounds(priv->text_buffer, 0, 0) )
    fkb_delete_selection(fkb, 1, 1);

  insert_text_with_tag(fkb, iter, text, priv->tag_bg);
}

static void
temp_input_cb(HildonVKBRenderer *vkb, gchar *input, gboolean unk, gpointer data)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  GtkTextIter iter;

  temp_text_clear(fkb);
  if ( input && g_utf8_strlen(input, -1) )
  {
    word_completion_clear(fkb);
    gtk_text_buffer_get_iter_at_mark(priv->text_buffer,
                                     &iter,
                                     gtk_text_buffer_get_insert(priv->text_buffer));
    temp_text_insert(fkb,&iter,input);
  }
}

static void
illegal_input_cb(HildonVKBRenderer *vkb, gchar *input, gpointer data)
{
  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  hildon_im_ui_play_sound(HILDON_IM_WESTERN_FKB_GET_PRIVATE(HILDON_IM_WESTERN_FKB(data))->ui,
                          HILDON_IM_ILLEGAL_INPUT_SOUND);
}

static void
repeating_button_pressed(GtkWidget *widget, void *data)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  priv->field_88 = 0;
  priv->pressed_repeating_button = widget;
  priv->field_80 = 1;
  priv->repeat_start_timer = g_timeout_add(800, repeating_button_repeat_start, fkb);
}

static gboolean
repeating_button_repeat_start(void *data)
{

  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;

  g_return_val_if_fail(HILDON_IM_IS_WESTERN_FKB(data),FALSE);

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  repeating_button_process_click(fkb, priv->pressed_repeating_button);
  priv->field_88 = 1;
  priv->repeat_start_timer = g_timeout_add(167, repeating_button_repeat, fkb);

  return FALSE;
}

static gboolean
repeating_button_repeat(void *data)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;

  g_return_val_if_fail(HILDON_IM_IS_WESTERN_FKB(data),FALSE);

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  repeating_button_process_click(fkb, priv->pressed_repeating_button);
  return TRUE;
}

static void
repeating_button_released(GtkWidget *widget, void *data)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  if ( priv->repeat_start_timer )
  {
    g_source_remove(priv->repeat_start_timer);
    priv->repeat_start_timer = 0;
  }

  if ((priv->pressed_repeating_button == widget) && priv->field_80 )
  {
    priv->pressed_repeating_button = 0;
    if ( !priv->field_88 )
      repeating_button_process_click(fkb, widget);
    priv->field_88 = 0;
  }
}

static void
repeating_button_enter(GtkWidget *widget, void *data)
{
  HildonIMWesternFKBPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(HILDON_IM_WESTERN_FKB(data));

  if ( priv->pressed_repeating_button == widget )
    priv->field_80 = 1;
  priv->field_B0 = 0;
}

static void
repeating_button_leave(GtkWidget *widget, void *data)
{
  HildonIMWesternFKBPrivate *priv;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(HILDON_IM_WESTERN_FKB(data));

  if ( priv->pressed_repeating_button == widget )
  {
    priv->field_80 = 0;
    priv->field_88 = 1;
    if ( priv->repeat_start_timer )
    {
      g_source_remove(priv->repeat_start_timer);
      priv->repeat_start_timer = 0;
    }
  }
}

static gboolean
textview_key_press_release_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{

  g_return_val_if_fail(HILDON_IM_IS_WESTERN_FKB(data),FALSE);

  if ( event->keyval == GDK_KP_Enter )
    return TRUE;

  if ( event->keyval > GDK_KP_Enter )
  {
    if ( event->keyval - GDK_F7 <= 1 )
      return TRUE;
    hildon_im_western_fkb_hide_fkb_window(HILDON_IM_WESTERN_FKB(data));
  }

  if ( event->keyval - GDK_BackSpace > 1 )
    hildon_im_western_fkb_hide_fkb_window(HILDON_IM_WESTERN_FKB(data));

  return TRUE;
}

static gboolean
textview_button_release_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;

  g_return_val_if_fail(HILDON_IM_IS_WESTERN_FKB(data),FALSE);

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  return TRUE;
}

static glong g_utf8_len_from_offset(const gchar *str, glong offset)
{
  return g_utf8_offset_to_pointer(str, offset) - str;
}

static void fkb_delete_selection(HildonIMWesternFKB *fkb, gboolean clear_wc, gboolean redirect)
{
  HildonIMWesternFKBPrivate *priv;
  gint start;
  gint end;
  gint text_buffer_offset;
  GtkTextIter iter_end;
  GtkTextIter iter_start;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  gtk_text_buffer_get_selection_bounds(priv->text_buffer, &iter_start, &iter_end);
  text_buffer_offset = get_text_buffer_offset(fkb);

  start = gtk_text_iter_get_offset(&iter_start);
  end = gtk_text_iter_get_offset(&iter_end);

  temp_text_clear(fkb);

  if ( priv->current_input_mode & HILDON_GTK_INPUT_MODE_INVISIBLE )
  {
    gint start_len = g_utf8_len_from_offset(priv->str->str, start);
    gint end_len = g_utf8_len_from_offset(priv->str->str, end);
    priv->str = g_string_erase(priv->str, start_len, end_len - start_len);
  }

  if ( redirect && hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_REDIRECT )
  {
    if ((text_buffer_offset == start) && (end - start))
      hildon_im_ui_send_surrounding_offset(priv->ui, TRUE, end - start);

    for(;start<end;start++)
      hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_HANDLE_BACKSPACE);
  }

  if (clear_wc)
  {
    gtk_text_buffer_delete_selection(priv->text_buffer, TRUE, TRUE);
    word_completion_clear(fkb);
  }
}

static void menu_button_cb(GtkWidget *widget, void *data)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;
  gint selection;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);


  if (priv->current_input_mode & HILDON_GTK_INPUT_MODE_INVISIBLE)
  {
    gboolean sel_bounds = gtk_text_buffer_get_selection_bounds(priv->text_buffer, NULL, NULL);
    gtk_widget_set_sensitive(priv->button_common_menu_cut, sel_bounds);
    gtk_widget_set_sensitive(priv->button_common_menu_copy, sel_bounds);
  }


  gtk_clipboard_request_targets(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
                                clipboard_targets_received_callback,
                                fkb);

  gtk_window_set_modal(GTK_WINDOW(priv->control_menu), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(priv->control_menu),
                               priv->fkb_window);
  selection = gtk_dialog_run(GTK_DIALOG(priv->control_menu));

  gtk_widget_hide(GTK_WIDGET(priv->control_menu));

  switch ( selection )
  {
    case 0:
      if(hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_REDIRECT)
        fkb_delete_selection(fkb, 0, 1);

      gtk_text_buffer_cut_clipboard(priv->text_buffer,
                                    gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
                                    TRUE);
      break;
    case 1:

    gtk_text_buffer_copy_clipboard(priv->text_buffer,
                                   gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
    break;
    case 2:
      gtk_clipboard_request_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
                                 paste_received,
                                 fkb);
      break;
    case 3:
      g_warning("%d %d", 0, priv->active_language);
      if ( priv->active_language )
        hildon_im_ui_set_active_language_index(priv->ui, 0);
      break;
    case 4:
      g_warning("%d %d", 1, priv->active_language);
      if ( priv->active_language != 1 )
        hildon_im_ui_set_active_language_index(priv->ui, 1);
      break;
    default:
      break;
  }

  show_text_view(fkb);
}

static void clipboard_targets_received_callback(GtkClipboard *clipboard, GdkAtom *atoms, gint n_atoms, gpointer data)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;
  gboolean sensitive = FALSE;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  if ( n_atoms > 0 )
  {
    int i;
    for(i=0;i<n_atoms;i++)
      if(atoms[i] == gdk_atom_intern("UTF8_STRING", 0) ||
         atoms[i] == gdk_atom_intern("COMPOUND_TEXT", 0) ||
         atoms[i] == GDK_TARGET_STRING )
      {
        sensitive = TRUE;
        break;
      }
  }

  gtk_widget_set_sensitive(priv->button_common_menu_paste, sensitive);
}

static void paste_received(GtkClipboard *clipboard, const gchar *text, gpointer data)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  temp_text_clear(fkb);

  if ( text && g_utf8_validate(text, -1, 0) )
  {
    glong len = g_utf8_strlen(text, strlen(text));
    HildonGtkInputMode mode = priv->current_input_mode;
    const gchar *p = text;

    if ( !len )
      goto LABEL_34;

    while ( 1 )
    {
      gunichar uc = g_utf8_get_char(p);
      if ( g_unichar_isalpha(uc) || uc == ' ' )
      {
        if ( mode & HILDON_GTK_INPUT_MODE_ALPHA )
          goto LABEL_14;

        if ( mode & HILDON_GTK_INPUT_MODE_HEXA && g_unichar_isxdigit(uc))
        {
          if ( mode & HILDON_GTK_INPUT_MODE_MULTILINE )
            goto LABEL_15;
          goto LABEL_22;
        }
      }
      else
      {
        if ( g_unichar_isdigit(uc) )
        {
          if (mode & (HILDON_GTK_INPUT_MODE_NUMERIC | HILDON_GTK_INPUT_MODE_HEXA | HILDON_GTK_INPUT_MODE_TELE))
            goto LABEL_14;
        }
        else
        {
          if (mode & HILDON_GTK_INPUT_MODE_SPECIAL || (mode & HILDON_GTK_INPUT_MODE_NUMERIC && uc == '-'))
            goto LABEL_14;
        }
      }
      if ( !(mode & HILDON_GTK_INPUT_MODE_TELE) || !strchr("#*+p0123456789", uc) )
        return;
LABEL_14:
      if (mode & HILDON_GTK_INPUT_MODE_MULTILINE)
        goto LABEL_15;
LABEL_22:
      if ((g_unichar_break_type(uc) - 1) <= 1 )
      {
        gchar * word = g_strndup(text, p - text);

        if (word)
        {
          word_completion_update_candidate(fkb, word);
          if ( hildon_im_ui_get_commit_mode(priv->ui) == 1 )
            hildon_im_ui_send_utf8(priv->ui, word);

          g_free((gpointer)word);
LABEL_26:
          word_completion_clear(fkb);

          if ( priv->field_CC )
          {
            g_source_remove(priv->field_CC);
            text_buffer_asterisk_fill(priv);
          }
          return;
        }
LABEL_34:
        word_completion_update_candidate(fkb, text);
        if ( hildon_im_ui_get_commit_mode(priv->ui) == 1 )
          hildon_im_ui_send_utf8(priv->ui, text);
        goto LABEL_26;
      }
LABEL_15:
      len--;

      if ( !len )
        goto LABEL_34;

      p = g_utf8_next_char(p);
    }
  }
}

static void update_input_mode_and_layout(HildonIMWesternFKB *self)
{
  HildonIMWesternFKBPrivate *priv;
  HildonGtkInputMode current_default_input_mode;
  HildonGtkInputMode current_input_mode;
  int v7;
  int mode;
  unsigned int sub_layout;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(self));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(self);

  priv->current_input_mode = hildon_im_ui_get_current_input_mode(priv->ui);
  current_default_input_mode = hildon_im_ui_get_current_default_input_mode(priv->ui);
  priv->current_default_input_mode = current_default_input_mode;
  current_input_mode = priv->current_input_mode;
  v7 = current_input_mode & current_default_input_mode;
  if ( !v7 )
    v7 = priv->current_input_mode;
  if ( !(v7 & 9) )
  {
    if ( v7 & 0x12 )
    {
      sub_layout = 2;
      goto LABEL_6;
    }
    if ( v7 & HILDON_GTK_INPUT_MODE_SPECIAL )
    {
      sub_layout = 3;
      goto LABEL_6;
    }
  }
  sub_layout = 0;
LABEL_6:
  mode = -(current_input_mode & 1) & 0x61;
  if ( current_input_mode & HILDON_GTK_INPUT_MODE_NUMERIC )
    mode |= 2u;
  if ( current_input_mode & 4 )
    mode |= HILDON_GTK_INPUT_MODE_TELE;
  if ( current_input_mode & 8 )
    mode |= HILDON_GTK_INPUT_MODE_SPECIAL;
  if ( current_input_mode & 0x10 )
    mode |= HILDON_GTK_INPUT_MODE_HEXA;

  g_object_set(priv->vkb, "mode", mode, NULL);
  fkb_set_layout(self, sub_layout);

  if ( priv->layout_info )
  {
    layout_info_free(priv->layout_info);
    priv->layout_info = NULL;
  }

  g_object_get(priv->vkb, "subs", &priv->layout_info, NULL);
}

static void input(HildonVKBRenderer *vkb, gchar *input, gboolean unk, gpointer data)
{
  HildonIMWesternFKB *fkb;
  HildonIMWesternFKBPrivate *priv;
  gint text_buffer_offset;
  guint pressed_key_mode;
  gboolean v21 = FALSE;
  gchar *dead_key;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(data));

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  if ( input )
  {
    if ( hildon_im_ui_get_shift_sticky(priv->ui) )
    {
      hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_SHIFT_UNSTICKY);
      hildon_im_ui_set_shift_sticky(priv->ui, 0);
    }
    if ( hildon_im_ui_get_level_sticky(priv->ui) )
    {
      hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_LEVEL_UNSTICKY);
      hildon_im_ui_set_level_sticky(priv->ui, 0);
    }
    if ( priv->field_B0 && priv->field_AC)
      v21 = TRUE;

    priv->field_B0 = 0;
    text_buffer_offset = get_text_buffer_offset(fkb);

    if ( *input && (*input != ' '))
    {
      if ( *input != '\n' )
      {
        if ( *input != '\b' )
        {
          if ( priv->field_34 && text_buffer_offset == priv->offset )
            fkb_backspace(fkb, 1);

          if (unk && v21 && hildon_im_common_should_be_appended_after_letter(input) && (text_buffer_offset == priv->offset))
          {
            fkb_backspace(fkb, 1);
            word_completion_update_candidate(fkb, input);
            word_completion_update_candidate(fkb, " ");

            if(hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_REDIRECT)
            {
              hildon_im_ui_send_utf8(priv->ui, input);
              hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_HANDLE_SPACE);
            }
          }
          else
          {
            if ( priv->field_34 && unk && (text_buffer_offset == priv->offset) )
            {
              word_completion_update_candidate(fkb, input);

              if(hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_REDIRECT)
                hildon_im_ui_send_utf8(priv->ui, input);
            }
          }
          priv->field_34 = (unk == 0);
          if ( !unk )
          {
            if ( v21 )
              priv->field_B0 = 1;
          }
          priv->offset = get_text_buffer_offset(fkb);
          goto LABEL_22;
        }
LABEL_42:
        fkb_backspace(fkb, 1);
        return;
      }
      fkb_enter(fkb);
    }
    else
    {
      pressed_key_mode = hildon_vkb_renderer_get_pressed_key_mode(HILDON_VKB_RENDERER(priv->vkb));
      dead_key = hildon_vkb_renderer_get_dead_key(HILDON_VKB_RENDERER(priv->vkb));

      if ( pressed_key_mode & 0x800 )
        goto LABEL_42;

      if ( pressed_key_mode & 0x1000 )
      {
        if ( hildon_vkb_renderer_get_shift_active(HILDON_VKB_RENDERER(priv->vkb)) )
        {
          hildon_im_ui_set_shift_locked(priv->ui, 1);
        }
        else
        {
          hildon_im_ui_set_shift_locked(priv->ui, 0);
          hildon_im_ui_set_shift_sticky(priv->ui, 0);
          priv->field_20 = 1;
        }
LABEL_22:
        set_layout(fkb);
        return;
      }

      if(dead_key)
      {
        hildon_vkb_renderer_clear_dead_key(HILDON_VKB_RENDERER(priv->vkb));
        word_completion_update_candidate(fkb, dead_key);
        if ( hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_REDIRECT )
          hildon_im_ui_send_utf8(priv->ui, dead_key);
        g_free(dead_key);
      }
      else
      {
        if ( pressed_key_mode & 0x40 )
        {
          word_completion_update_candidate(fkb, " ");
          if ( hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_REDIRECT )
            hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_HANDLE_SPACE);
          goto LABEL_22;
        }
      }
    }
  }
}

static void fkb_backspace(HildonIMWesternFKB *self, gboolean unk)
{
  HildonIMWesternFKBPrivate *priv;
  GtkTextIter iter;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(self));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(self);

  hildon_vkb_renderer_clear_dead_key(HILDON_VKB_RENDERER(priv->vkb));
  temp_text_clear(self);

  if ( !word_completion_clear(self) )
  {
    if ( gtk_text_buffer_get_selection_bounds(priv->text_buffer, 0, 0) )
    {
      fkb_delete_selection(self, 1, 1);
      show_text_view(self);
      set_layout(self);
    }
    else
    {
      gtk_text_buffer_get_iter_at_mark(priv->text_buffer,
                                       &iter,
                                       gtk_text_buffer_get_insert(priv->text_buffer));

      if ( priv->current_input_mode & HILDON_GTK_INPUT_MODE_INVISIBLE )
      {
        const gchar *str = priv->str->str;
        gchar* p = g_utf8_offset_to_pointer(priv->str->str, get_text_buffer_offset(self));
        gchar* prev = g_utf8_find_prev_char(str, p);

        if (prev >= str)
        {
         if (p > prev)
           priv->str = g_string_erase(priv->str, prev - str, p - prev);
        }
      }
      if ( unk && hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_REDIRECT )
      {
        if ( gtk_text_iter_get_offset(&iter) > 0 )
          hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_HANDLE_BACKSPACE);
      }
      gtk_text_buffer_backspace(priv->text_buffer, &iter, 1, 1);
      show_text_view(self);
      set_layout(self);
    }
  }
}

static void fkb_enter(HildonIMWesternFKB *self)
{
  HildonIMWesternFKBPrivate *priv;
  gchar *surrounding_content;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(self));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(self);

  hildon_vkb_renderer_clear_dead_key(HILDON_VKB_RENDERER(priv->vkb));
  temp_text_clear(self);
  word_completion_clear(self);

  if (priv->current_input_mode & HILDON_GTK_INPUT_MODE_MULTILINE )
  {
    word_completion_update_candidate(self, "\n");

    if ( hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_REDIRECT )
      hildon_im_ui_send_utf8(priv->ui, "\n");

    show_text_view(self);
    set_layout(self);
  }
  else
  {
    if ( hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_SURROUNDING )
    {
      if((priv->current_input_mode & HILDON_GTK_INPUT_MODE_INVISIBLE) && priv->str )
      {
        surrounding_content = priv->str->str;
      }
      else
      {
        surrounding_content = get_input_text(self, 0);
        word_completion_hit_word(self, priv->field_9C);
      }

      hildon_im_ui_send_surrounding_content(priv->ui, surrounding_content);
      hildon_im_western_fkb_set_surrounding_offset(self);
      priv->field_38 = 0;
    }
    hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_ENTER_ON_FOCUS);
    hildon_im_western_fkb_hide_fkb_window(self);
  }
}
static void word_completion_complete(HildonIMWesternFKB* fkb)
{
  HildonIMWesternFKBPrivate *priv;
  GtkTextIter iter;
  gunichar uc;

  g_return_if_fail(HILDON_IM_IS_WESTERN_FKB(fkb));

  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  if (priv->predicted_suffix)
  {

    gtk_text_buffer_insert_at_cursor(priv->text_buffer,
                                     priv->predicted_suffix,
                                     strlen(priv->predicted_suffix));

    if ( hildon_im_ui_get_commit_mode(priv->ui) == 1 )
      hildon_im_ui_send_utf8(priv->ui, priv->predicted_suffix);

    gtk_text_buffer_get_iter_at_mark(priv->text_buffer,
                                     &iter,
                                     gtk_text_buffer_get_insert(priv->text_buffer));
    gtk_text_iter_forward_chars(&iter,
                                g_utf8_strlen(priv->predicted_suffix, -1));
    gtk_text_buffer_place_cursor(priv->text_buffer,
                                 &iter);
    uc = gtk_text_iter_get_char(&iter);

    if ( priv->insert_space_after_word )
    {
      if ( g_unichar_type(uc) != G_UNICODE_SPACE_SEPARATOR )
      {
        if ((uc != 9) && !g_unichar_ispunct(uc))
        {
          gtk_text_buffer_insert_at_cursor(priv->text_buffer, " ", 1);
          priv->field_B0 = 1;

          if(hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_REDIRECT)
            hildon_im_ui_send_utf8(priv->ui, " ");
        }
      }
    }
    hildon_im_word_completer_hit_word((HildonIMWWordCompleter *)priv->hwc, priv->field_94, 0);
    g_free(priv->predicted_word);
    priv->predicted_word = g_strdup(priv->field_94);
    word_completion_clear(fkb);
  }
}

gboolean textview_button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  HildonIMWesternFKBPrivate *priv;
  HildonIMWesternFKB *fkb;
  GdkRegion *region;
  GtkTextIter end;
  GtkTextIter start;
  GdkRectangle clipbox;
  GdkRectangle end_location;
  GdkRectangle start_location;
  GdkRectangle rect;
  gint buffer_y;
  gint buffer_x;
  int v19;

  g_return_val_if_fail(HILDON_IM_IS_WESTERN_FKB(data),FALSE);

  fkb = HILDON_IM_WESTERN_FKB(data);
  priv = HILDON_IM_WESTERN_FKB_GET_PRIVATE(fkb);

  priv->offset = get_text_buffer_offset(fkb);

  if ( !priv->predicted_suffix || !*priv->predicted_suffix || !word_completion_get_bounds(fkb, &start, &end) )
    goto LABEL_7;

  gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(priv->textview),
                                        gtk_text_view_get_window_type(GTK_TEXT_VIEW(priv->textview),event->window),
                                        event->x, event->y, &buffer_x, &buffer_y);

  region = gdk_region_new();

  gtk_text_view_get_iter_location(GTK_TEXT_VIEW(priv->textview), &start, &start_location);

  gtk_text_view_get_iter_location(GTK_TEXT_VIEW(priv->textview), &end, &end_location);

  v19 = end_location.y / end_location.height + 1 - start_location.y / start_location.height;
  rect.x = start_location.x;
  rect.y = start_location.y;
  rect.height = start_location.height;

  if ( v19 == 1 )
  {
    rect.width = end_location.width + end_location.x - rect.x;
    gdk_region_union_with_rect(region, &rect);
  }
  else
  {
    rect.width = GTK_WIDGET(priv->textview)->allocation.width - rect.x;
    gdk_region_union_with_rect(region, &rect);

    if ( v19 > 1 )
    {
      rect.x = 0;
      rect.y = end_location.y;
      rect.width = end_location.width + end_location.x;
      rect.height = end_location.height;
      gdk_region_union_with_rect(region, &rect);

      if ( v19 != 2 )
      {
        rect.x = 0;
        rect.y = start_location.height + start_location.y;
        rect.width = GTK_WIDGET(priv->textview)->allocation.width;
        rect.height = start_location.height * (v19 - 2);
        gdk_region_union_with_rect(region, &rect);
      }
    }
  }

  gdk_region_get_clipbox(region, &clipbox);

  if ( clipbox.width <= 69 )
    gdk_region_shrink(region, -((70 - clipbox.width) >> 1), 0);

  if ( clipbox.height <= 69 )
    gdk_region_shrink(region, 0, -((70 - clipbox.height) >> 1));

  if ( !gdk_region_point_in(region, buffer_x, buffer_y) )
  {
    word_completion_clear(fkb);
    gdk_region_destroy(region);
LABEL_7:
    priv->field_B0 = 0;

    GTK_WIDGET_CLASS(widget)->button_press_event(widget, event);

    if ( hildon_im_ui_get_commit_mode(priv->ui) == HILDON_IM_COMMIT_REDIRECT )
      hildon_im_western_fkb_set_surrounding_offset(fkb);

    return TRUE;
  }

  word_completion_complete(fkb);
  priv->offset = get_text_buffer_offset(fkb);
  gdk_region_destroy(region);

  return TRUE;
}
