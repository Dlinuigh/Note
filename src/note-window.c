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

static void
note_window_init(NoteWindow *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
	// self->header_bar = ADW_HEADER_BAR(adw_header_bar_new());
	// self->page=ADW_TAB_VIEW(adw_tab_view_new());
	GtkWidget *list_box = gtk_list_box_new();
	Note *note = g_object_new(NOTE_TYPE_NOTE, NULL);
	AdwTabPage *list = adw_tab_view_append(self->page, GTK_WIDGET(list_box));
	filename = g_strconcat(g_get_home_dir(), "/Documents/file.txt", NULL);
	JsonParser* parser = json_parser_new();
	// strcat(filename, "/Documents/file.txt"); // 代码会导致不能自动释放资源。
	GFile *file = g_file_new_for_path(filename);
	if (g_file_query_exists(file, NULL) != true)
		// if(g_file_open_readwrite(file, NULL, NULL)==G_IO_ERROR_NOT_FOUND)
		// {
		g_file_create(file, G_FILE_CREATE_PRIVATE, NULL, NULL);
	// g_file_set_contents(filename, "", -1, NULL);
	// }
	g_object_unref(file);
	// GFileIOStream *fd;
	// if((fd=g_file_open_readwrite(filename, NULL, NULL)==G_IO_ERROR_NOT_FOUND)){
	// 	g_file_create(file, G_FILE_CREATE_PRIVATE, NULL, NULL);
	// 	g_file_new_for_path()
	// }
	// g_free(filename);
	gtk_label_set_label(note->date, "2024/3/20");
	// note->title = ADW_ACTION_ROW(adw_action_row_new());
	gtk_list_box_append(GTK_LIST_BOX(list_box), GTK_WIDGET(note));
	adw_tab_view_set_selected_page(self->page, list);
	// gtk_widget_show(GTK_WIDGET(self->page));
}

void note_add_init(NoteWindow *self)
{
	/* gtk_widget_set_visible (GTK_WIDGET (self->header_bar), (gboolean) FALSE); */
	/* free (self->note_list); */
	// GtkTextView * text = gtk_text_view_new_with_buffer(gtk_text_buffer_new(gtk_text_tag_table_new()))
	GtkTextView *text;
	GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
	gchar *content;
	g_file_get_contents(filename, &content, NULL, NULL);
	gtk_text_buffer_set_text(buffer, content, -1);
	g_free(content);
	text = GTK_TEXT_VIEW(gtk_text_view_new_with_buffer(buffer));

	AdwTabPage *add = adw_tab_view_append(self->page, GTK_WIDGET(text));
	// AdwTabPage *add = adw_tab_view_append(self->page, GTK_WIDGET(gtk_text_view_new()));
	adw_tab_view_set_selected_page(self->page, add);
	gtk_button_set_icon_name(self->add_page, "go-previous-symbolic");
	// GtkListBox *list_box = GTK_LIST_BOX(gtk_stack_page_get_child(self->note_list));
	// Note *note = g_object_new(NOTE_TYPE_NOTE, NULL);
	// // g_date_time_new_now_local()
	// gtk_list_box_append(list_box, GTK_WIDGET(note));
	// printf("I am here\n");
}

void note_add_close(NoteWindow *self)
{
	AdwTabPage *add = adw_tab_view_get_selected_page(self->page);
	GtkTextView *text = GTK_TEXT_VIEW(adw_tab_page_get_child(add));
	GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	GtkTextIter start;
	GtkTextIter end;
	char *content = NULL;
	gtk_text_buffer_get_start_iter(text_buffer, &start);
	gtk_text_buffer_get_end_iter(text_buffer, &end);
	content = gtk_text_buffer_get_text(text_buffer, &start, &end, false);
	g_file_set_contents(filename, content, -1, NULL);
	adw_tab_view_close_page(self->page, add);
	gtk_button_set_icon_name(self->add_page, "list-add-symbolic");
	g_free(content);
}
