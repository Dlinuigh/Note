#include "config.h"

#include "note-preferences.h"

struct _NotePreferences
{
	AdwApplicationWindow  parent_instance;

	/* Template widgets */
	/* AdwHeaderBar        *header_bar; */
	/* GtkListBox          *note_list; */
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
	/* gtk_widget_class_bind_template_child (widget_class, Notepreferences, header_bar); */
	/* gtk_widget_class_bind_template_child (widget_class, Notepreferences, note_list); */
    gtk_widget_class_bind_template_child(widget_class, NotePreferences, apperence);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, editor);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, sync);
	gtk_widget_class_bind_template_child(widget_class, NotePreferences, save);
	/* gtk_widget_class_bind_template_child(widget_class, NotePreferences, apperence_list); */
}

static void
note_preferences_init (NotePreferences *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));
	self->apperence = ADW_PREFERENCES_PAGE (adw_preferences_page_new ());
	self->editor = ADW_PREFERENCES_PAGE (adw_preferences_page_new ());
	self->sync = ADW_PREFERENCES_PAGE (adw_preferences_page_new ());
	self->save = ADW_PREFERENCES_PAGE (adw_preferences_page_new ());
	/* gtk_box_append (GTK_BOX (self->apperence), GTK_WIDGET(self->apperence_list)); */
	/* adw_preferences_page_add (self->apperence, ) */
}
