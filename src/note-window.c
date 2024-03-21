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

	/* Template widgets */
	AdwHeaderBar *header_bar;
	AdwTabView *page;
	GtkButton *add_page;
	GtkListBox *list;
};

struct _Note
{
	GtkBox parent_instance;

	AdwActionRow *title;
	GtkLabel *date;
};

G_DEFINE_FINAL_TYPE(NoteWindow, note_window, ADW_TYPE_APPLICATION_WINDOW)

G_DEFINE_FINAL_TYPE(Note, note_note, GTK_TYPE_BOX)

char *filename;
static void
note_window_class_init(NoteWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class, "/org/lion_claw/note/note-window.ui");
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, header_bar);
	// gtk_widget_class_bind_template_child(widget_class, NoteWindow, note_list);
	// gtk_widget_class_bind_template_child(widget_class, NoteWindow, note_add);
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, page);
	gtk_widget_class_bind_template_child(widget_class, NoteWindow, add_page);
	// gtk_widget_class_bind_template_child(widget_class, NoteWindow, list);
}

static void note_note_class_init(NoteClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	gtk_widget_class_set_template_from_resource(widget_class, "/org/lion_claw/note/note.ui");
	gtk_widget_class_bind_template_child(widget_class, Note, title);
	gtk_widget_class_bind_template_child(widget_class, Note, date);
}

static void note_note_init(Note *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
	// self->title = ADW_ACTION_ROW(adw_action_row_new());
	// self->date = GTK_LABEL(gtk_label_new("2024/03/20"));
}

static int get_real_n_char(const char* s, int n){
	// int i=0,j=0;
	// while(s[i]){
	// 	if((s[i]&0xc0)!=0x80){
	// 		j++;
	// 	}
	// }
	// return j;
	//TODO - 计划将中文显示扩展到所有地方。
	int real_n=0;
	// int len = strlen(s);
	int num = 0;
	for(int i=0;s[i]&&num<n;i++, num++){
		
		if((s[i]&0x80)==0){
			real_n++;
		}else if((s[i]&0xe0)==0xc0){
			real_n+=2;
			i+=1;
		}else if((s[i]&0xf0)==0xe0){
			real_n+=3;
			i+=2;
		}else if((s[i]&0xf8)==0xf0){
			real_n+=4;
			i+=3;
		}else if((s[i]&0xfc)==0xf8){
			real_n+=5;
			i+=4;
		}else if((s[i]&0xfe)==0xfc){
			real_n+=6;
			i+=5;
		}
	}
	return real_n;
}

static void list_add(GtkWidget* list_box, const char* content, const char* date){
	Note *note = g_object_new(NOTE_TYPE_NOTE, NULL);
	GtkWidget* image;
	char title[66];
	char subtitle[186];
	GDateTime* datetime = g_date_time_new_from_iso8601(date, NULL);
	if(g_date_time_difference(g_date_time_new_now_local(), datetime)/G_TIME_SPAN_HOUR > 6){
		// adw_action_row_set_icon_name(note->title,"changes-prevent-symbolic");
		image = gtk_image_new_from_icon_name("changes-prevent-symbolic");
	}else{
		// adw_action_row_set_icon_name(note->title,"changes-allow-symbolic");
		image = gtk_image_new_from_icon_name("changes-allow-symbolic");
	}
	int rn = get_real_n_char(content, 10);
	strncpy(title, content, rn);
	title[rn]='\0';
	adw_action_row_add_prefix(note->title,image);
	//TODO - title 不支持中文，这是个问题。我知道了，utf8本身支持中文，但是由于我截断了，所以编码失败。
	// pango_parse_markup(title, 10, 0,&attr_list, &markup, NULL,NULL);
	// adw_preferences_row_set_use_markup(ADW_PREFERENCES_ROW(note->title), true);
	adw_preferences_row_set_title(ADW_PREFERENCES_ROW(note->title), title);
	rn = get_real_n_char(content, 30);
	strncpy(subtitle, content, rn);
	subtitle[rn]='\0';
	//FIXME - 但是副标题却可以支持中文。而且不用任何改动。由于副标题的容量很大，但是每次测试我只有一点文本。
	adw_action_row_set_subtitle(note->title, subtitle);
	adw_action_row_set_subtitle_lines(note->title, 2);
	gtk_label_set_label(note->date, g_date_time_format(datetime, "%F %T"));
	gtk_list_box_append(GTK_LIST_BOX(list_box), GTK_WIDGET(note));
	g_free(datetime);
}

static void window_refresh(NoteWindow *self){
	//TODO - 刷新主体窗口内容，用来显示发生的变化。
	filename = g_strconcat(g_get_home_dir(), "/.local/share/Note/data.json", NULL);
	// strcat(filename, "/Documents/file.txt"); // 代码会导致不能自动释放资源。
	GFile *file = g_file_new_for_path(filename);
	if (g_file_query_exists(file, NULL) != true)
		// if(g_file_open_readwrite(file, NULL, NULL)==G_IO_ERROR_NOT_FOUND)
		// {
		g_file_create(file, G_FILE_CREATE_PRIVATE, NULL, NULL);
	// g_file_set_contents(filename, "", -1, NULL);
	// }
	g_object_unref(file);
	JsonParser* parser = json_parser_new();
	json_parser_load_from_file(parser, filename, NULL);
	// GFileIOStream *fd;
	// if((fd=g_file_open_readwrite(filename, NULL, NULL)==G_IO_ERROR_NOT_FOUND)){
	// 	g_file_create(file, G_FILE_CREATE_PRIVATE, NULL, NULL);
	// 	g_file_new_for_path()
	// }
	// g_free(filename);
	JsonReader *reader = json_reader_new(json_parser_get_root(parser));
	json_reader_read_member(reader, "timestamp");
	int num = json_reader_count_elements(reader);
	// reader = json_reader_new(json_parser_get_root(parser));
	g_object_unref(reader);
	// g_free(parser);
	// JsonParser* parser = json_parser_new();
	// json_parser_load_from_file(parser, filename, NULL);
	//TODO - 由于打开的json不一定要立刻关闭，全局变量更有效。可以节省窗口刷新的消耗。
	JsonNode* root = json_parser_get_root(parser);
	JsonObject* root_obj = json_node_dup_object(root);
	JsonNode* dt_node = json_object_get_member(root_obj, "timestamp");
	JsonArray* dt = json_node_get_array(dt_node);
	JsonNode* cont_node = json_object_get_member(root_obj, "content");
	JsonArray* cont = json_node_get_array(cont_node);

	printf("%d\n", num);
	// PangoAttrList* attr_list=pango_attr_list_new();
	// char* markup;
	for(int i=0;i<num;i++){
		// json_reader_read_member(reader, "timestamp");
		// json_reader_read_element(reader, i);
		// char* date = json_reader_get_string_value(reader);
		// json_reader_end_element(reader);
		// json_reader_end_member(reader);
		
		// json_reader_read_member(reader, "content");
		// json_reader_read_element(reader, i);
		// char* content = json_reader_get_string_value(reader);
		// json_reader_end_element(reader);
		// json_reader_end_member(reader);
		
		// g_object_unref (reader);
		// g_object_unref (parser);
		char* date = json_array_get_string_element(dt, i);
		char* content = json_array_get_string_element(cont, i);
		list_add(GTK_WIDGET(self->list), content, date);
	}

}


static void
note_window_init(NoteWindow *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
	// self->header_bar = ADW_HEADER_BAR(adw_header_bar_new());
	// self->page=ADW_TAB_VIEW(adw_tab_view_new());
	
	// gtk_label_set_label(note->date, "2024/3/20");
	// note->title = ADW_ACTION_ROW(adw_action_row_new());
	// gtk_list_box_append(GTK_LIST_BOX(list_box), GTK_WIDGET(note));
	self->list = GTK_LIST_BOX(gtk_list_box_new());
	GtkWidget* scroll = gtk_scrolled_window_new();
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), GTK_WIDGET(self->list));
	gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroll), true);
	AdwTabPage *list = adw_tab_view_append(self->page, scroll);
	window_refresh(self);
	adw_tab_view_set_selected_page(self->page, list);
	// gtk_widget_show(GTK_WIDGET(self->page));
	// json_array_unref(cont);
	// json_array_unref(dt);
	// json_node_unref(dt_node);
	// json_node_unref(cont_node);
	// json_object_unref(root_obj);
	// json_node_unref(root);
	// g_object_unref(parser);
}

void note_add_init(NoteWindow *self)
{
	// 这个函数只用来新建。

	/* gtk_widget_set_visible (GTK_WIDGET (self->header_bar), (gboolean) FALSE); */
	/* free (self->note_list); */
	// GtkTextView * text = gtk_text_view_new_with_buffer(gtk_text_buffer_new(gtk_text_tag_table_new()))
	// GtkTextView *text;
	// GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
	// gchar *content;
	// g_file_get_contents(filename, &content, NULL, NULL);
	// gtk_text_buffer_set_text(buffer, content, -1);
	// g_free(content);
	// text = GTK_TEXT_VIEW(gtk_text_view_new_with_buffer(buffer));

	// AdwTabPage *add = adw_tab_view_append(self->page, GTK_WIDGET(text));
	AdwTabPage *add = adw_tab_view_append(self->page, GTK_WIDGET(gtk_text_view_new()));
	adw_tab_view_set_selected_page(self->page, add);
	gtk_button_set_icon_name(self->add_page, "go-previous-symbolic");
	// GtkListBox *list_box = GTK_LIST_BOX(gtk_stack_page_get_child(self->note_list));
	// Note *note = g_object_new(NOTE_TYPE_NOTE, NULL);
	// // g_date_time_new_now_local()
	// gtk_list_box_append(list_box, GTK_WIDGET(note));
	// printf("I am here\n");
}

static void json_add(const char* content, const char* date){
	
	JsonParser* parser = json_parser_new();
	json_parser_load_from_file(parser, filename, NULL);
	JsonNode* root = json_parser_get_root(parser);

	// JsonBuilder *builder = json_builder_new();
	JsonGenerator *gen = json_generator_new();
	// JsonNode* root = json_builder_get_root(builder);
	// JsonReader *reader = json_reader_new(root);
	JsonObject* root_obj = json_node_dup_object(root);
	JsonNode* dt_node = json_object_get_member(root_obj, "timestamp");
	JsonArray* dt = json_node_get_array(dt_node);
	JsonNode* cont_node = json_object_get_member(root_obj, "content");
	JsonArray* cont = json_node_get_array(cont_node);
	
	// json_builder_begin_object(builder);

	// json_builder_set_member_name(builder, "timestamp");
	// json_builder_begin_array(builder);
	// json_builder_add_string_value(builder, g_date_time_format_iso8601(g_date_time_new_now_local()));
	// json_builder_end_array(builder);

	// json_builder_set_member_name(builder, "content");
	// json_builder_begin_array(builder);
	// json_builder_add_string_value(builder, content);
	// json_builder_end_array(builder);
	
	// json_builder_end_object(builder);

	json_array_add_string_element(dt, date);
	json_array_add_string_element(cont, content);

	json_generator_set_root(gen, root);
	gchar *str = json_generator_to_data(gen, NULL);
	// printf("%s", filename);
	// if(json_generator_to_file(gen, filename, NULL)){
	if(g_file_set_contents(filename, str, -1, NULL)){
		printf("Write to file.\n");
	}else{
		printf("Error, check the path\n");
	}
	// json_node_free(root);
	// g_object_unref(gen);
	// json_array_unref(cont);
	// json_array_unref(dt);
	// json_node_unref(dt_node);
	// json_node_unref(cont_node);
	// json_object_unref(root_obj);
	// json_node_unref(root);
	// g_object_unref(parser);
	// g_object_unref(builder);
}


void note_add_close(NoteWindow *self)
{
	/*NOTE - json member: id, timestamp, content, edited_times...
	*/
	AdwTabPage *add = adw_tab_view_get_selected_page(self->page);
	GtkTextView *text = GTK_TEXT_VIEW(adw_tab_page_get_child(add));
	GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	GtkTextIter start;
	GtkTextIter end;
	char *content = NULL;
	gtk_text_buffer_get_start_iter(text_buffer, &start);
	gtk_text_buffer_get_end_iter(text_buffer, &end);
	content = gtk_text_buffer_get_text(text_buffer, &start, &end, false);
	const char* date = g_date_time_format_iso8601(g_date_time_new_now_local());
	json_add(content, date);
	// g_file_set_contents(filename, content, -1, NULL);
	adw_tab_view_close_page(self->page, add);
	gtk_button_set_icon_name(self->add_page, "list-add-symbolic");
	list_add(GTK_WIDGET(self->list), content, date);
	// g_free(content); 
	// TODO: 这里可以做一个优化，不要每次离开就立刻将text清理掉，改成程序关闭时。
}
