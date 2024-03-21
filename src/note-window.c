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

	AdwHeaderBar *header_bar;
	AdwTabView *page;
	GtkButton *add_page;
	GtkListBox *list;
};

G_DEFINE_FINAL_TYPE(NoteWindow, note_window, ADW_TYPE_APPLICATION_WINDOW)

char *filename;
JsonParser* parser;
JsonNode* root;
JsonArray* dt;
JsonArray* cont;
JsonReader* reader;
// JsonBuilder* builder;
JsonGenerator* gen;
guint note_idx;
guint note_len;

static void
note_window_class_init(NoteWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class, "/org/lion_claw/note/note-window.ui");
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, header_bar);
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, page);
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, add_page);
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

static void json_edit_by_idx(JsonArray* array, guint idx, JsonNode* element_node, gpointer user_data){
	if(idx==note_idx){
		json_node_set_string(element_node, user_data);
	}
}

static void note_save_action(GtkButton* save, GData* user_data){
	NoteWindow* self = g_datalist_get_data(&user_data, "NoteWindow");
	GtkTextIter start;
	GtkTextIter end;
	GtkTextBuffer* text_buffer = g_datalist_get_data(&user_data, "buffer");
	char *content = NULL;
	gtk_text_buffer_get_start_iter(text_buffer, &start);
	gtk_text_buffer_get_end_iter(text_buffer, &end);
	content = gtk_text_buffer_get_text(text_buffer, &start, &end, false);
	json_array_foreach_element(cont, json_edit_by_idx, content);
	AdwTabPage* page = adw_tab_view_get_selected_page(self->page);
	adw_header_bar_remove(self->header_bar, GTK_WIDGET(save));
	adw_tab_view_close_page(self->page, page);
}

static void note_edit_action(AdwActionRow* note, GData* user_data){
	NoteWindow* self = g_datalist_get_data(&user_data, "NoteWindow");
	AdwActionRow* row = g_datalist_get_data(&user_data, "selected_note");
	int idx = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
	// AdwActionRow* note = ADW_ACTION_ROW(gtk_list_box_get_selected_row(self->list));
	char* content = json_array_get_string_element(cont, idx);
	// char* date = json_array_get_string_element(dt, *idx);
	GtkTextBuffer* buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, content, -1);
	GtkWidget* text = gtk_text_view_new_with_buffer(buffer);
	AdwTabPage *edit = adw_tab_view_append(self->page, text);
	adw_tab_view_set_selected_page(self->page, edit);
	GtkButton *save_edit = GTK_BUTTON(gtk_button_new());
	gtk_button_set_icon_name(save_edit, "document-save-symbolic");
	adw_header_bar_pack_start(self->header_bar, GTK_WIDGET(save_edit));
	GData* datalist;
	g_datalist_init(&datalist);
	g_datalist_set_data(&datalist, "NoteWindow", self);
	g_datalist_set_data(&datalist, "buffer", buffer);
	g_signal_connect(save_edit, "clicked", G_CALLBACK(note_save_action), datalist);
	// gtk_button_set_icon_name(self->add_page, "go-previous-symbolic");
}

static void list_add(NoteWindow* self, const char *content, const char *date, int idx)
{
	
	AdwActionRow* note = ADW_ACTION_ROW( adw_action_row_new());
	GData* datalist;
	g_datalist_init(&datalist);
	g_datalist_set_data(&datalist, "NoteWindow", self);
	g_datalist_set_data(&datalist, "selected_note", note);
	g_signal_connect(note, "activated", G_CALLBACK(note_edit_action), datalist);
	//NOTE - 连接成功，但是由于act信号不能通过点击触发。所以无法使用。
	// 通过该函数可以证明上面的说法。 adw_action_row_activate(note);
	GtkWidget *image;
	char title[66];
	char subtitle[186];
	GDateTime *datetime = g_date_time_new_from_iso8601(date, NULL);
	if (g_date_time_difference(g_date_time_new_now_local(), datetime) / G_TIME_SPAN_HOUR > 6)
	{
		image = gtk_image_new_from_icon_name("changes-prevent-symbolic");
	}
	else
	{
		image = gtk_image_new_from_icon_name("changes-allow-symbolic");
	}
	GtkWidget* check = gtk_check_button_new();
	adw_action_row_add_suffix(note, check);
	gtk_widget_set_visible(check, false);
	adw_action_row_set_activatable_widget(note, check);
	//NOTE - 这里其实我们没有使用到这个widget, 仅仅用来显示icon,未来添加选中函数的时候可以绑定到一个checkbutton.
	// 这个函数将会在批量删除note的时候引入。 上面暂时用invisible处理了。
	int rn = get_real_n_char(content, 10);
	strncpy(title, content, rn);
	title[rn] = '\0';
	adw_action_row_add_prefix(note, image);
	adw_preferences_row_set_title(ADW_PREFERENCES_ROW(note), title);
	rn = get_real_n_char(content, 30);
	strncpy(subtitle, content, rn);
	subtitle[rn] = '\0';
	adw_action_row_set_subtitle(note, subtitle);
	adw_action_row_set_subtitle_lines(note, 2);
	adw_action_row_add_suffix(note, gtk_label_new(g_date_time_format(datetime, "%F %T")));
	gtk_list_box_append(GTK_LIST_BOX(self->list), GTK_WIDGET(note));
	g_free(datetime);
}

static void window_refresh(NoteWindow *self, int num)
{
	for (int i = 0; i < num; i++)
	{
		char *date = json_array_get_string_element(dt, i);
		char *content = json_array_get_string_element(cont, i);
		list_add(self, content, date, i);
	}
}

static void
note_window_init(NoteWindow *self)
{
	/*!SECTION note_window:
		note_window -->  adw_header_bar
					-->  gtk_scrolled_window --> gtk_list_box
	*/
	gtk_widget_init_template(GTK_WIDGET(self));
	self->list = GTK_LIST_BOX(gtk_list_box_new());
	GtkWidget *scroll = gtk_scrolled_window_new();
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), GTK_WIDGET(self->list));
	gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroll), true);
	AdwTabPage *list = adw_tab_view_append(self->page, scroll);

	filename = g_strconcat(g_get_home_dir(), "/.local/share/Note/data.json", NULL);
	GFile *file = g_file_new_for_path(filename);
	if (g_file_query_exists(file, NULL) != true)
		g_file_create(file, G_FILE_CREATE_PRIVATE, NULL, NULL);
	g_object_unref(file);
	parser = json_parser_new();
	json_parser_load_from_file(parser, filename, NULL);
	reader = json_reader_new(json_parser_get_root(parser));
	json_reader_read_member(reader, "timestamp");
	note_len = json_reader_count_elements(reader);
	g_object_unref(reader);
	// TODO - 由于打开的json不一定要立刻关闭，全局变量更有效。可以节省窗口刷新的消耗。
	root = json_parser_get_root(parser);
	JsonObject *root_obj = json_node_dup_object(root);
	JsonNode *dt_node = json_object_get_member(root_obj, "timestamp");
	dt = json_node_get_array(dt_node);
	JsonNode *cont_node = json_object_get_member(root_obj, "content");
	cont = json_node_get_array(cont_node);

	window_refresh(self, note_len);
	adw_tab_view_set_selected_page(self->page, list);
}

void note_add_init(NoteWindow *self)
{
	// NOTE - 这个函数只用来新建。
	AdwTabPage *add = adw_tab_view_append(self->page, GTK_WIDGET(gtk_text_view_new()));
	adw_tab_view_set_selected_page(self->page, add);
	gtk_button_set_icon_name(self->add_page, "go-previous-symbolic");
}

static void json_add(const char *content, const char *date)
{

	// JsonParser *parser = json_parser_new();
	// json_parser_load_from_file(parser, filename, NULL);
	// JsonNode *root = json_parser_get_root(parser);
	// JsonGenerator *gen = json_generator_new();
	// JsonObject *root_obj = json_node_dup_object(root);
	// JsonNode *dt_node = json_object_get_member(root_obj, "timestamp");
	// JsonArray *dt = json_node_get_array(dt_node);
	// JsonNode *cont_node = json_object_get_member(root_obj, "content");
	// JsonArray *cont = json_node_get_array(cont_node);
	json_array_add_string_element(dt, date);
	json_array_add_string_element(cont, content);
	json_generator_set_root(gen, root);
	gchar *str = json_generator_to_data(gen, NULL);
	if (g_file_set_contents(filename, str, -1, NULL))
	{
		printf("Write to file.\n");
	}
	else
	{
		printf("Error, check the path\n");
	}
}

void note_add_close(NoteWindow *self)
{
	// NOTE - json member: id, timestamp, content, edited_times...
	AdwTabPage *add = adw_tab_view_get_selected_page(self->page);
	GtkTextView *text = GTK_TEXT_VIEW(adw_tab_page_get_child(add));
	GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	GtkTextIter start;
	GtkTextIter end;
	char *content = NULL;
	gtk_text_buffer_get_start_iter(text_buffer, &start);
	gtk_text_buffer_get_end_iter(text_buffer, &end);
	content = gtk_text_buffer_get_text(text_buffer, &start, &end, false);
	const char *date = g_date_time_format_iso8601(g_date_time_new_now_local());
	json_add(content, date);
	adw_tab_view_close_page(self->page, add);
	gtk_button_set_icon_name(self->add_page, "list-add-symbolic");
	list_add(self, content, date, note_len);
	// TODO: 这里可以做一个优化，不要每次离开就立刻将text清理掉，改成程序关闭时。
}
