#include "config.h"

#include "note-preferences.h"

struct _NotePreferences
{
	AdwApplicationWindow parent_instance;

	/* Template widgets */
	GSettings* settings;
	AdwPreferencesPage *apperence;
	AdwPreferencesPage *editor;
	AdwPreferencesPage *sync;
	AdwPreferencesPage *save;
	GtkAdjustment *title_num;
	GtkAdjustment *subtitle_num;
	GtkAdjustment *subtitle_line;
	GtkSwitch *custom_font;
	GtkFontDialogButton *custom_font_border;
	GtkFontDialogButton *custom_font_title;
	GtkFontDialogButton *custom_font_subtitle;
	GtkSwitch* custom_window_size;
	GtkAdjustment *height;
	GtkAdjustment *width;
	GtkSwitch *custom_editor_font;
	GtkFontDialogButton *custom_font_editor;
	AdwEntryRow *save_path;
	GtkAdjustment *lock_time;
};

G_DEFINE_FINAL_TYPE(NotePreferences, note_preferences, ADW_TYPE_PREFERENCES_WINDOW)

static void
note_preferences_class_init(NotePreferencesClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class, "/org/lion_claw/note/note-preferences.ui");
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, apperence);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, editor);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, sync);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, save);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, title_num);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, subtitle_num);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, subtitle_line);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, custom_font);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, custom_font_border);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, custom_font_title);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, custom_font_subtitle);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, custom_window_size);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, height);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, width);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, custom_editor_font);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, custom_font_editor);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, save_path);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, lock_time);
}

static void note_preferences_close(NotePreferences *self){
	g_settings_set_uint(self->settings, "height", (uint)gtk_adjustment_get_value(self->height));
	g_settings_set_uint(self->settings, "width", (uint)gtk_adjustment_get_value(self->width));
	g_settings_set_boolean(self->settings, "custom-window-size", gtk_switch_get_active(self->custom_window_size));
	g_settings_set_uint(self->settings, "title-num", (uint)gtk_adjustment_get_value(self->title_num));
	g_settings_set_uint(self->settings, "subtitle-num", (uint)gtk_adjustment_get_value(self->subtitle_num));
	g_settings_set_uint(self->settings, "subtitle-line", (uint)gtk_adjustment_get_value(self->subtitle_line));
	g_settings_set_boolean(self->settings, "custom-font", gtk_switch_get_active(self->custom_font));
	g_settings_set_boolean(self->settings, "custom-editor-font", gtk_switch_get_active(self->custom_editor_font));
	g_settings_set_string(self->settings, "custom-font-border", pango_font_description_to_string(gtk_font_dialog_button_get_font_desc(self->custom_font_border)));
	g_settings_set_string(self->settings, "custom-font-title", pango_font_description_to_string(gtk_font_dialog_button_get_font_desc(self->custom_font_title)));
	g_settings_set_string(self->settings, "custom-font-subtitle", pango_font_description_to_string(gtk_font_dialog_button_get_font_desc(self->custom_font_subtitle)));
	g_settings_set_string(self->settings, "custom-font-editor", pango_font_description_to_string(gtk_font_dialog_button_get_font_desc(self->custom_font_editor)));
	g_settings_set_int(self->settings, "lock-time", (int)gtk_adjustment_get_value(self->lock_time));
	//TODO - fontdiagbutton 因为只能显示字体和大小，所以即便fontdiag能够设置weight和language之类的信息也不会到fontdiagbutton,只能说适合高级设置。
	gtk_window_close(GTK_WINDOW(self));
}

static void
note_preferences_init(NotePreferences *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
	self->settings = g_settings_new("org.lion_claw.note");
	gtk_adjustment_set_value(self->height, g_settings_get_uint(self->settings, "height"));
	gtk_adjustment_set_value(self->width, g_settings_get_uint(self->settings, "width"));
	gtk_adjustment_set_value(self->title_num, g_settings_get_uint(self->settings, "title-num"));
	gtk_adjustment_set_value(self->subtitle_num, g_settings_get_uint(self->settings, "subtitle-num"));
	gtk_adjustment_set_value(self->subtitle_line, g_settings_get_uint(self->settings, "subtitle-line"));
	gtk_switch_set_active(self->custom_window_size, g_settings_get_boolean(self->settings, "custom-window-size"));
	gtk_switch_set_active(self->custom_font, g_settings_get_boolean(self->settings, "custom-font"));
	gtk_switch_set_active(self->custom_editor_font, g_settings_get_boolean(self->settings, "custom-editor-font"));
	gtk_font_dialog_button_set_font_desc(self->custom_font_border, pango_font_description_from_string(g_settings_get_string(self->settings, "custom-font-border")));
	gtk_font_dialog_button_set_font_desc(self->custom_font_title, pango_font_description_from_string(g_settings_get_string(self->settings, "custom-font-title")));
	gtk_font_dialog_button_set_font_desc(self->custom_font_subtitle, pango_font_description_from_string(g_settings_get_string(self->settings, "custom-font-subtitle")));
	gtk_font_dialog_button_set_font_desc(self->custom_font_editor, pango_font_description_from_string(g_settings_get_string(self->settings, "custom-font-editor")));
	gtk_font_dialog_button_set_dialog(self->custom_font_border, gtk_font_dialog_new());
	gtk_font_dialog_button_set_dialog(self->custom_font_title, gtk_font_dialog_new());
	gtk_font_dialog_button_set_dialog(self->custom_font_subtitle, gtk_font_dialog_new());
	gtk_font_dialog_button_set_dialog(self->custom_font_editor, gtk_font_dialog_new());
	gtk_adjustment_set_value(self->lock_time, g_settings_get_int(self->settings, "lock-time"));
	g_signal_connect(self, "close-request", G_CALLBACK(note_preferences_close), NULL);
}