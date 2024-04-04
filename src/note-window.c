/* note-window.c
 *
 * Copyright 2024 Unknown
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"
#include "note-window.h"
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>
struct _NoteWindow
{
	AdwApplicationWindow parent_instance;
	GSettings *settings;
	AdwHeaderBar *header_bar;
	AdwTabView *page;
	GtkButton *btn;
	GtkListBox *list;
};

G_DEFINE_FINAL_TYPE(NoteWindow, note_window, ADW_TYPE_APPLICATION_WINDOW)

char *filename;
JsonParser *parser;
JsonNode *root;
JsonArray *dt;
JsonArray *cont;
JsonReader *reader;
JsonGenerator *gen;
guint note_idx;
guint note_len;
GtkTextTagTable *table = NULL;
gulong note_new_id;
gulong note_close_id;
gulong page_close_id;
gulong note_edit_to_new_id;

void note_new(NoteWindow *);
void note_close(NoteWindow *);
static void page_close(AdwTabView *, AdwTabPage *);

static void
note_window_class_init(NoteWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class, "/org/lion_claw/note/note-window.ui");
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, header_bar);
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, page);
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, btn);
}

static int get_real_n_char(const char *s, int n)
{
	// TODO - 计划将中文显示扩展到所有地方。
	int real_n = 0;
	int num = 0;
	for (int i = 0; s[i] && num < n; i++, num++)
	{
		if ((s[i] & 0x80) == 0)
		{
			real_n++;
		}
		else if ((s[i] & 0xe0) == 0xc0)
		{
			real_n += 2;
			i += 1;
		}
		else if ((s[i] & 0xf0) == 0xe0)
		{
			real_n += 3;
			i += 2;
		}
		else if ((s[i] & 0xf8) == 0xf0)
		{
			real_n += 4;
			i += 3;
		}
		else if ((s[i] & 0xfc) == 0xf8)
		{
			real_n += 5;
			i += 4;
		}
		else if ((s[i] & 0xfe) == 0xfc)
		{
			real_n += 6;
			i += 5;
		}
	}
	return real_n;
}

static void json_edit_by_idx(JsonArray *array, guint idx, JsonNode *element_node, gpointer user_data)
{
	if (idx == note_idx)
	{
		json_node_set_string(element_node, user_data);
	}
}

static void note_show(AdwActionRow *note, const char *content, const char *date, gboolean is_exist)
{
	// TODO - 该部分将为设置喜好的主题内容，有许多外观选项可以自定义。
	// TODO - 主标题的内容摘要算法。
	GSettings *settings = g_settings_new("org.lion_claw.note");
	uint title_num = g_settings_get_uint(settings, "title-num");
	uint subtitle_num = g_settings_get_uint(settings, "subtitle-num");
	uint subtitle_line = g_settings_get_uint(settings, "subtitle-line");
	int time = g_settings_get_int(settings, "lock-time");

	char title[66];
	char subtitle[186];
	int rn = get_real_n_char(content, title_num);
	strncpy(title, content, rn);
	title[rn] = '\0';
	adw_preferences_row_set_title(ADW_PREFERENCES_ROW(note), title);
	rn = get_real_n_char(content, subtitle_num);
	strncpy(subtitle, content, rn);
	subtitle[rn] = '\0';
	adw_action_row_set_subtitle(note, subtitle);
	adw_action_row_set_subtitle_lines(note, subtitle_line);
	GtkWidget *check = gtk_check_button_new();
	adw_action_row_add_suffix(note, check);
	gtk_widget_set_visible(check, false);
	adw_action_row_set_activatable_widget(note, check);
	if (!is_exist)
	{
		GtkWidget *image;
		GDateTime *datetime = g_date_time_new_from_iso8601(date, NULL);
		// TODO - 文件加锁的时间阈值。文件加锁的设置开关。
		//TODO - 文件加锁的更新机制，因为代码是在加载的时候进行判定，而打开了就将保持，也就意味着，不更新。
		if (time != -1)
		{
			if (g_date_time_difference(g_date_time_new_now_local(), datetime) / G_TIME_SPAN_HOUR > time)
			{
				image = gtk_image_new_from_icon_name("changes-prevent-symbolic");
			}
			else
			{
				image = gtk_image_new_from_icon_name("changes-allow-symbolic");
			}
			adw_action_row_add_prefix(note, image);
		}
		else
		{
			// TODO - 不用加锁。
		}
		// TODO - 时间的显示格式。可以自定义。
		adw_action_row_add_suffix(note, gtk_label_new(g_date_time_format(datetime, "%F %T")));
	}
}

static void note_save_action(GtkButton *save, NoteWindow *self)
{
	GtkTextIter start;
	GtkTextIter end;
	AdwTabPage *page = adw_tab_view_get_nth_page(self->page, 1);
	GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(adw_tab_page_get_child(page)));
	const char *date = json_array_get_string_element(dt, note_idx);
	AdwActionRow *note = ADW_ACTION_ROW(gtk_list_box_get_row_at_index(self->list, note_idx));
	const char *content = NULL;
	gtk_text_buffer_get_start_iter(text_buffer, &start);
	gtk_text_buffer_get_end_iter(text_buffer, &end);
	content = gtk_text_buffer_get_text(text_buffer, &start, &end, false);
	json_array_foreach_element(cont, json_edit_by_idx, (gpointer)content);
	note_show(note, content, date, true);
	adw_header_bar_remove(self->header_bar, GTK_WIDGET(save));
	json_generator_to_file(gen, filename, NULL);
	g_signal_handler_disconnect(self->btn, note_edit_to_new_id);
	note_new_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_new), self);
	g_signal_handler_disconnect(self->page, page_close_id);
	page_close_id = g_signal_connect(self->page, "close-page", G_CALLBACK(page_close), page);
	adw_tab_view_close_page(self->page, page);
}

static void note_edit_to_new(GData *user_data)
{
	GtkButton *save = g_datalist_get_data(&user_data, "save");
	NoteWindow *self = g_datalist_get_data(&user_data, "NoteWindow");
	AdwTabPage *page = adw_tab_view_get_nth_page(self->page, 1);
	adw_header_bar_remove(self->header_bar, GTK_WIDGET(save));
	g_signal_handler_disconnect(self->btn, note_edit_to_new_id);
	note_new_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_new), self);
	g_signal_handler_disconnect(self->page, page_close_id);
	page_close_id = g_signal_connect(self->page, "close-page", G_CALLBACK(page_close), page);
	adw_tab_view_close_page(self->page, page);
	note_new(self);
}

static void note_close_action(GtkButton* close, NoteWindow* self){
	AdwWindowTitle* title = ADW_WINDOW_TITLE(adw_header_bar_get_title_widget(self->header_bar));
	adw_window_title_set_subtitle(title, NULL);
	AdwTabPage *page = adw_tab_view_get_nth_page(self->page, 1);
	adw_header_bar_remove(self->header_bar, GTK_WIDGET(close));
	g_signal_handler_disconnect(self->btn, note_edit_to_new_id);
	note_new_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_new), self);
	g_signal_handler_disconnect(self->page, page_close_id);
	page_close_id = g_signal_connect(self->page, "close-page", G_CALLBACK(page_close), page);
	adw_tab_view_close_page(self->page, page);
	//FIXME - Adwaita-CRITICAL **: 11:03:35.567: adw_tab_view_close_page_finish: assertion 'page_belongs_to_this_view (self, page)' failed 每次关闭页面都有这个报错，目前却不影响功能。按理说各种信息应该是正确的。
}

static void note_edit_action(AdwActionRow *row, NoteWindow *self)
{
	GtkTextBuffer *buffer = NULL;
	GtkWidget *text = NULL;
	GtkButton *btn = GTK_BUTTON(gtk_button_new());
	AdwTabPage *edit = NULL;
	GData *dl;
	GDateTime* datetime=NULL;
	int time = g_settings_get_int(self->settings, "lock-time");
	const char *content = NULL;
	note_idx = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
	content = json_array_get_string_element(cont, note_idx);
	datetime = g_date_time_new_from_iso8601(json_array_get_string_element(dt, note_idx),NULL);
	buffer = gtk_text_buffer_new(table);
	text = gtk_text_view_new_with_buffer(buffer);
	edit = adw_tab_view_append(self->page, text);
	//TODO - actionrow把问题搞得复杂了，他可以添加suffix但是无法直接获得。那先这样，将编辑时间视为新的现在时间。
	if (g_date_time_difference(g_date_time_new_now_local(), datetime) / G_TIME_SPAN_HOUR > time){
		gtk_text_view_set_editable(GTK_TEXT_VIEW(text), false);
		AdwWindowTitle* title = ADW_WINDOW_TITLE(adw_header_bar_get_title_widget(self->header_bar));
		adw_window_title_set_subtitle(title, "Read Only");
		//TODO - if read only then save meaningless.
		gtk_button_set_icon_name(btn, "go-previous-symbolic");
		g_signal_connect(btn, "clicked", G_CALLBACK(note_close_action), self);
	}else{
		gtk_button_set_icon_name(btn, "document-save-symbolic");
		g_signal_connect(btn, "clicked", G_CALLBACK(note_save_action), self);
	}
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text), true);
	gtk_text_buffer_set_text(buffer, content, -1);
	adw_header_bar_pack_start(self->header_bar, GTK_WIDGET(btn));
	adw_tab_view_set_selected_page(self->page, edit);
	g_datalist_init(&dl);
	g_datalist_set_data(&dl, "NoteWindow", self);
	g_datalist_set_data(&dl, "save", btn);
	g_signal_handler_disconnect(self->btn, note_new_id);
	note_edit_to_new_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_edit_to_new), dl);
}

static void list_add(NoteWindow *self, const char *content, const char *date, int idx)
{
	AdwActionRow *note = ADW_ACTION_ROW(adw_action_row_new());
	note_show(note, content, date, false);
	gtk_list_box_append(GTK_LIST_BOX(self->list), GTK_WIDGET(note));
	g_signal_connect(note, "activated", G_CALLBACK(note_edit_action), self);
}

static void window_refresh(NoteWindow *self, int num)
{
	for (int i = 0; i < num; i++)
	{
		const char *date = json_array_get_string_element(dt, i);
		const char *content = json_array_get_string_element(cont, i);
		if (content == NULL)
		{
			list_add(self, "", date, i);
		}
		else
		{
			list_add(self, content, date, i);
		}
	}
}

static void json_init(NoteWindow* self)
{
	GFile *file = NULL;
	JsonObject *root_obj = NULL;
	JsonNode *dt_node = NULL;
	JsonNode *cont_node = NULL;
	// TODO - 文件的保存地址将设置为可改变。并且假设大量的文件条目那么将文件进行分隔将会节省时间。
	filename = g_settings_get_string(self->settings, "save-path");
	//TODO - 调试的时候用的是flatpak环境，此时使用绝对路径导致找不到文件，这是正常现象，因为flatpak会将其挂载到/run/目录下。
	// filename = g_strconcat(g_get_home_dir(), path, NULL); //FIXME - 至于这里为什么能对，是因为使用了通用的g_get_home_dir获取home路径。
	file = g_file_new_for_path(filename);
	if (g_file_query_exists(file, NULL) != true)
		g_file_create(file, G_FILE_CREATE_PRIVATE, NULL, NULL);
	g_object_unref(file);
	parser = json_parser_new();
	json_parser_load_from_file(parser, filename, NULL);
	reader = json_reader_new(json_parser_get_root(parser));
	json_reader_read_member(reader, "timestamp");
	note_len = json_reader_count_elements(reader);
	g_object_unref(reader);
	root = json_parser_get_root(parser);
	root_obj = json_node_dup_object(root);
	dt_node = json_object_get_member(root_obj, "timestamp");
	dt = json_node_get_array(dt_node);
	cont_node = json_object_get_member(root_obj, "content");
	cont = json_node_get_array(cont_node);
	gen = json_generator_new();
	json_generator_set_root(gen, root);
}

static void json_add(const char *content, const char *date)
{
	json_array_add_string_element(dt, date);
	json_array_add_string_element(cont, content);
	json_generator_to_file(gen, filename, NULL);
}

static void page_close(AdwTabView *view, AdwTabPage *page)
{
	adw_tab_view_close_page_finish(view, page, true);
}

void note_close(NoteWindow *self)
{
	AdwTabPage *add = adw_tab_view_get_nth_page(self->page, 1);
	GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(adw_tab_page_get_child(add)));
	GtkTextIter start;
	GtkTextIter end;
	char *content = NULL;
	const char *date = NULL;
	gtk_text_buffer_get_start_iter(text_buffer, &start);
	gtk_text_buffer_get_end_iter(text_buffer, &end);
	content = gtk_text_buffer_get_text(text_buffer, &start, &end, false);
	date = g_date_time_format_iso8601(g_date_time_new_now_local());
	json_add(content, date);
	gtk_button_set_icon_name(self->btn, "list-add-symbolic");
	list_add(self, content, date, note_len);
	note_new_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_new), self);
	g_signal_handler_disconnect(self->btn, note_close_id);
	g_signal_handler_disconnect(self->page, page_close_id);
	page_close_id = g_signal_connect(self->page, "close-page", G_CALLBACK(page_close), add);
	adw_tab_view_close_page(self->page, add);
}

void note_new(NoteWindow *self)
{
	PangoFontDescription* desc=NULL;
	GString* css = NULL;
	GtkCssProvider* provider=NULL;
	gboolean if_editor_c = g_settings_get_boolean(self->settings, "custom-editor-font");
	GtkTextBuffer *buffer = gtk_text_buffer_new(table);
	GtkWidget* view = gtk_text_view_new_with_buffer(buffer);
	AdwTabPage *add = adw_tab_view_append(self->page, view);
	if(if_editor_c){
		desc = pango_font_description_from_string(g_settings_get_string(self->settings, "custom-font-editor"));
		css = g_string_new("#tabview .view{font-family:'");
		g_string_append(css, pango_font_description_get_family(desc));
		g_string_append(css, "';font-size:");
		g_string_append_printf(css, "%dpt;}", pango_font_description_get_size(desc)/PANGO_SCALE);
		provider = gtk_css_provider_new();
		gtk_css_provider_load_from_string(provider, css->str);
		gtk_style_context_add_provider_for_display(gtk_widget_get_display(view), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
	adw_tab_view_set_selected_page(self->page, add);
	gtk_button_set_icon_name(self->btn, "go-previous-symbolic");
	g_signal_handler_disconnect(self->btn, note_new_id);
	note_close_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_close), self);
}

static void
note_window_init(NoteWindow *self)
{
	GtkWidget *scroll = gtk_scrolled_window_new();
	AdwTabPage *list = NULL;
	PangoFontDescription* desc=NULL;
	GString* css = NULL;
	GtkCssProvider* provider=NULL;
	gtk_widget_init_template(GTK_WIDGET(self));

	self->settings = g_settings_new("org.lion_claw.note");
	gtk_window_set_default_size(GTK_WINDOW(self), g_settings_get_uint(self->settings, "width"), g_settings_get_uint(self->settings, "height"));
	gboolean if_list_c = g_settings_get_boolean(self->settings, "custom-font");
	if(if_list_c){
		desc = pango_font_description_from_string(g_settings_get_string(self->settings, "custom-font-title"));
		css = g_string_new("#tabview .title{font-family:'");
		g_string_append(css, pango_font_description_get_family(desc));
		g_string_append(css, "';font-size:");
		g_string_append_printf(css, "%dpt;}", pango_font_description_get_size(desc)/PANGO_SCALE);

		desc = pango_font_description_from_string(g_settings_get_string(self->settings, "custom-font-subtitle"));
		g_string_append(css, "#tabview .subtitle{font-family:'");
		g_string_append(css, pango_font_description_get_family(desc));
		g_string_append(css, "';font-size:");
		g_string_append_printf(css, "%dpt;}", pango_font_description_get_size(desc)/PANGO_SCALE);

		desc = pango_font_description_from_string(g_settings_get_string(self->settings, "custom-font-border"));
		g_string_append(css, ":not(#tabview){font-family:'");
		g_string_append(css, pango_font_description_get_family(desc));
		g_string_append(css, "';font-size:");
		g_string_append_printf(css, "%dpt;}", pango_font_description_get_size(desc)/PANGO_SCALE);
		provider = gtk_css_provider_new();
		gtk_css_provider_load_from_string(provider, css->str);
		gtk_style_context_add_provider_for_display(gtk_widget_get_display(GTK_WIDGET(self)), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
	list = adw_tab_view_append(self->page, scroll);
	self->list = GTK_LIST_BOX(gtk_list_box_new());
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), GTK_WIDGET(self->list));
	gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroll), true);
	json_init(self);
	window_refresh(self, note_len);
	gtk_button_set_icon_name(self->btn, "list-add-symbolic");
	adw_tab_view_set_selected_page(self->page, list);
	note_new_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_new), self);
	page_close_id = g_signal_connect(self->page, "close-page", G_CALLBACK(page_close), list);
}