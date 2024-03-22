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
	GtkButton *btn;
	GtkListBox *list;
	// AdwTabBar* tabar;
};

G_DEFINE_FINAL_TYPE(NoteWindow, note_window, ADW_TYPE_APPLICATION_WINDOW)

char *filename;
JsonParser* parser;
JsonNode* root;
JsonArray* dt;
JsonArray* cont;
JsonGenerator* gen;
JsonBuilder* builder;
guint note_idx;
guint note_len;
GtkTextTagTable* table=NULL;
gulong note_new_id;
gulong note_close_id;
gulong page_close_id;
gulong note_edit_to_new_id;

void note_new(NoteWindow*);
void note_close(NoteWindow*);
static void page_close(AdwTabView*, AdwTabPage*);

static void
note_window_class_init(NoteWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class, "/org/lion_claw/note/note-window.ui");
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, header_bar);
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, page);
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, btn);
	// gtk_widget_class_bind_template_child(widget_class, NoteWindow, tabar);
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

static void note_show(AdwActionRow * note, const char* content, const char* date, gboolean is_exist){
	char title[66];
	char subtitle[186];
	int rn = get_real_n_char(content, 10);
	strncpy(title, content, rn);
	title[rn] = '\0';
	adw_preferences_row_set_title(ADW_PREFERENCES_ROW(note), title);
	rn = get_real_n_char(content, 30);
	strncpy(subtitle, content, rn);
	subtitle[rn] = '\0';
	adw_action_row_set_subtitle(note, subtitle);
	adw_action_row_set_subtitle_lines(note, 2);
	if(!is_exist){
		GtkWidget *image;
		GDateTime *datetime = g_date_time_new_from_iso8601(date, NULL);
		GtkWidget* check = gtk_check_button_new();
		if (g_date_time_difference(g_date_time_new_now_local(), datetime) / G_TIME_SPAN_HOUR > 6)
		{
			image = gtk_image_new_from_icon_name("changes-prevent-symbolic");
		}
		else
		{
			image = gtk_image_new_from_icon_name("changes-allow-symbolic");
		}
		adw_action_row_add_suffix(note, check);
		adw_action_row_set_activatable_widget(note, check);
		adw_action_row_add_prefix(note, image);
		adw_action_row_add_suffix(note, gtk_label_new(g_date_time_format(datetime, "%F %T")));
		//NOTE - 这里其实我们没有使用到这个widget, 仅仅用来显示icon,未来添加选中函数的时候可以绑定到一个checkbutton.
		// 这个函数将会在批量删除note的时候引入。 上面暂时用invisible处理了。
	}
}

static void note_save_action(GtkButton* save, NoteWindow* self){
	GtkTextIter start;
	GtkTextIter end;
	AdwTabPage* page = adw_tab_view_get_nth_page(self->page, 1);
	GtkTextBuffer* text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(adw_tab_page_get_child(page)));
	const char* date=json_array_get_string_element(dt, note_idx);
	AdwActionRow* note = ADW_ACTION_ROW(gtk_list_box_get_row_at_index(self->list, note_idx));
	const char* content = NULL;
	gtk_text_buffer_get_start_iter(text_buffer, &start);
	gtk_text_buffer_get_end_iter(text_buffer, &end);
	content = gtk_text_buffer_get_text(text_buffer, &start, &end, false);
	json_array_foreach_element(cont, json_edit_by_idx, (gpointer)content);
	note_show(note, content, date, true);
	adw_header_bar_remove(self->header_bar, GTK_WIDGET(save));
	adw_tab_view_close_page(self->page, page);
	json_generator_to_file(gen, filename, NULL);
	g_signal_handler_disconnect(self->btn, note_edit_to_new_id);
	note_new_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_new), self);
	g_signal_handler_disconnect(self->page, page_close_id);
	page_close_id = g_signal_connect(self->page, "close-page", G_CALLBACK(page_close), page);
}

static void note_edit_to_new(GData* user_data){
	GtkButton* save = g_datalist_get_data(&user_data, "save");
	NoteWindow* self = g_datalist_get_data(&user_data, "NoteWindow");
	AdwTabPage* page = adw_tab_view_get_nth_page(self->page, 1);
	adw_header_bar_remove(self->header_bar, GTK_WIDGET(save));
	adw_tab_view_close_page(self->page, page);
	g_signal_handler_disconnect(self->btn, note_edit_to_new_id);
	note_new_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_new), self);
	g_signal_handler_disconnect(self->page, page_close_id);
	page_close_id = g_signal_connect(self->page, "close-page", G_CALLBACK(page_close), page);
	note_new(self);
}

static void note_edit_action(AdwActionRow* row, NoteWindow* self){
	GtkTextBuffer* buffer = NULL;
	GtkWidget* text = NULL;
	GtkButton* btn = GTK_BUTTON(gtk_button_new());
	AdwTabPage *edit = NULL;
	GData* dl;
	const char* content = NULL;
	note_idx = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
	content = json_array_get_string_element(cont, note_idx);
	buffer = gtk_text_buffer_new(table);
	text = gtk_text_view_new_with_buffer(buffer);
	edit = adw_tab_view_append(self->page, text);
	gtk_text_buffer_set_text(buffer, content, -1);
	gtk_button_set_icon_name(btn, "document-save-symbolic");
	adw_header_bar_pack_start(self->header_bar, GTK_WIDGET(btn));
	adw_tab_view_set_selected_page(self->page, edit);
	g_signal_connect(btn, "clicked", G_CALLBACK(note_save_action), self);
	g_datalist_init(&dl);
	g_datalist_set_data(&dl, "NoteWindow", self);
	g_datalist_set_data(&dl, "save", btn);
	g_signal_handler_disconnect(self->btn, note_new_id);
	note_edit_to_new_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_edit_to_new), dl);
}

static void list_add(NoteWindow* self, const char *content, const char *date, int idx)
{
	AdwActionRow* note = ADW_ACTION_ROW( adw_action_row_new());
	//NOTE - 引入datalist非常好地解决了导入参数不够的问题。并且也没有引入不成熟的方案。
	//NOTE - 连接成功，但是由于act信号不能通过点击触发。所以无法使用。
	// 通过该函数可以证明上面的说法。 adw_action_row_activate(note);
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
		if(content==NULL){
			list_add(self, "", date, i);
		}else{
			list_add(self, content, date, i);
		}
	}
}

static void json_init(void){
	GFile* file=NULL;
	JsonObject* root_obj=NULL;
	JsonNode* dt_node=NULL;
	JsonNode* cont_node=NULL;
	filename = g_strconcat(g_get_home_dir(), "/.local/share/Note/data.json", NULL);
	file = g_file_new_for_path(filename);
	gen = json_generator_new();
	parser = json_parser_new();
	json_parser_load_from_file(parser, filename, NULL);
	if (g_file_query_exists(file, NULL) != true || 
		(root = json_parser_get_root(parser))==NULL
		){
		builder = json_builder_new();
		json_builder_begin_object(builder);
		json_builder_set_member_name(builder, "timestamp");
		json_builder_begin_array(builder);
		json_builder_end_array(builder);
		json_builder_set_member_name(builder, "content");
		json_builder_begin_array(builder);
		json_builder_end_array(builder);
		json_builder_end_object(builder);
		root = json_builder_get_root(builder);
		json_generator_set_root(gen, root);
		json_generator_to_file(gen, filename, NULL);
	}else{
		json_generator_set_root(gen, root);
	}
	root_obj = json_node_dup_object(root);
	dt_node = json_object_get_member(root_obj, "timestamp");
	dt = json_node_get_array(dt_node);
	cont_node = json_object_get_member(root_obj, "content");
	cont = json_node_get_array(cont_node);
	note_len = json_array_get_length(dt);
	g_object_unref(file);
}

static void json_add(const char *content, const char *date)
{
	json_array_add_string_element(dt, date);
	json_array_add_string_element(cont, content);
	json_generator_to_file(gen, filename, NULL);
}

static void page_close(AdwTabView* view, AdwTabPage* page){
	adw_tab_view_close_page_finish(view, page, true);
}

void note_close(NoteWindow* self)
{
	AdwTabPage *add = adw_tab_view_get_nth_page(self->page, 1);
	GtkTextBuffer* text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(adw_tab_page_get_child(add)));
	GtkTextIter start;
	GtkTextIter end;
	char* content=NULL;
	const char* date=NULL;
	gtk_text_buffer_get_start_iter(text_buffer, &start);
	gtk_text_buffer_get_end_iter(text_buffer, &end);
	content = gtk_text_buffer_get_text(text_buffer, &start, &end, false);
	date = g_date_time_format_iso8601(g_date_time_new_now_local());
	json_add(content, date);
	gtk_button_set_icon_name(self->btn, "list-add-symbolic");
	list_add(self, content, date, note_len);
	adw_tab_view_close_page(self->page, add);
	note_new_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_new), self);
	g_signal_handler_disconnect(self->btn, note_close_id);
	g_signal_handler_disconnect(self->page, page_close_id);
	page_close_id = g_signal_connect(self->page, "close-page", G_CALLBACK(page_close), add);
	//TODO: 这里可以做一个优化，不要每次离开就立刻将text清理掉，改成程序关闭时。
}

void note_new(NoteWindow* self)
{
	GtkTextBuffer* buffer = gtk_text_buffer_new(table);
	AdwTabPage *add = adw_tab_view_append(self->page, GTK_WIDGET(gtk_text_view_new_with_buffer(buffer)));
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
	gtk_widget_init_template(GTK_WIDGET(self));
	list = adw_tab_view_append(self->page, scroll);
	self->list = GTK_LIST_BOX(gtk_list_box_new());
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), GTK_WIDGET(self->list));
	gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroll), true);
	json_init();
	window_refresh(self, note_len);
	gtk_button_set_icon_name(self->btn, "list-add-symbolic");
	adw_tab_view_set_selected_page(self->page, list);
	note_new_id = g_signal_connect_swapped(self->btn, "clicked", G_CALLBACK(note_new), self);
	page_close_id = g_signal_connect(self->page, "close-page", G_CALLBACK(page_close), list);
}