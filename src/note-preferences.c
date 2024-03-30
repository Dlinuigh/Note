#include "config.h"

#include "note-preferences.h"

struct _NotePreferences
{
	AdwApplicationWindow parent_instance;

	/* Template widgets */
	AdwPreferencesPage *apperence;
	AdwPreferencesPage *editor;
	AdwPreferencesPage *sync;
	AdwPreferencesPage *save;

	GtkSpinButton *title_num;
	GtkSpinButton *subtitle_num;
	GtkScale *subtitle_line;
	GtkSwitch *custom_font;
	GtkFontDialogButton *custom_font_border;
	GtkFontDialogButton *custom_font_title;
	GtkFontDialogButton *custom_font_subtitle;
	GtkSpinButton *height;
	GtkSpinButton *width;
	AdwExpanderRow *editor_custom_font;
	GtkFontDialogButton *editor_font;
	AdwEntryRow *save_path;
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
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, height);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, width);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, editor_custom_font);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, editor_font);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, save_path);
}

static void
note_preferences_init(NotePreferences *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
	GSettings *settings = g_settings_new("org.lion_claw.note");
	//TODO - no need to use get default, this will default be used.
	GVariant *height = g_settings_get_default_value(settings, "height");
	GVariant *width = g_settings_get_default_value(settings, "width");
	gtk_spin_button_set_value(self->height, (double)g_variant_get_uint32(height));
	gtk_spin_button_set_value(self->width, (double)g_variant_get_uint32(width));
}
