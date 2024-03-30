#include "config.h"

#include "note-preferences.h"

struct _NotePreferences
{
	AdwApplicationWindow  parent_instance;

	/* Template widgets */
    AdwPreferencesPage      *apperence;
	AdwPreferencesPage		*editor;
	AdwPreferencesPage		*sync;
	AdwPreferencesPage		*save;
};

G_DEFINE_FINAL_TYPE (NotePreferences, note_preferences, ADW_TYPE_PREFERENCES_WINDOW)

static void
note_preferences_class_init (NotePreferencesClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lion_claw/note/note-preferences.ui");
    gtk_widget_class_bind_template_child(widget_class, NotePreferences, apperence);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, editor);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, sync);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, save);
}

static void
note_preferences_init (NotePreferences *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));
	/* GSettings *settings = g_settings_new("org.lion_claw.note"); */
	/* printf("%d,%d\n", g_settings_get_int(settings, "width"), g_settings_get_int(settings, "height")); */
}
