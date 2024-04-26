/* note-application.c
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

#include "config.h" // IWYU pragma: export

#include "note-application.h"
#include "note-window.h"
#include "note-preferences.h"

struct _NoteApplication
{
	AdwApplication parent_instance;
};

G_DEFINE_TYPE(NoteApplication, note_application, ADW_TYPE_APPLICATION)

NoteApplication *
note_application_new(const char *application_id,
					 GApplicationFlags flags)
{
	g_return_val_if_fail(application_id != NULL, NULL);

	return g_object_new(NOTE_TYPE_APPLICATION,
						"application-id", application_id,
						"flags", flags,
						NULL);
}

static void
note_application_activate(GApplication *app)
{
	GtkWindow *window;

	g_assert(NOTE_IS_APPLICATION(app));

	window = gtk_application_get_active_window(GTK_APPLICATION(app));

	if (window == NULL)
		window = g_object_new(NOTE_TYPE_WINDOW,
							  "application", app,
							  NULL);

	gtk_window_present(window);
}

static void
note_application_class_init(NoteApplicationClass *klass)
{
	GApplicationClass *app_class = G_APPLICATION_CLASS(klass);

	app_class->activate = note_application_activate;
}

static void
note_application_about_action(GSimpleAction *action,
							  GVariant *parameter,
							  gpointer user_data)
{
	static const char *developers[] = {"Unknown", NULL};
	NoteApplication *self = user_data;
	GtkWindow *window = NULL;

	g_assert(NOTE_IS_APPLICATION(self));

	window = gtk_application_get_active_window(GTK_APPLICATION(self));

	adw_show_about_window(window,
						  "application-name", "note",
						  "application-icon", "org.lion_claw.note",
						  "developer-name", "Unknown",
						  "version", "0.1.0",
						  "developers", developers,
						  "copyright", "© 2024 Unknown",
						  NULL);
}

static void
note_application_quit_action(GSimpleAction *action,
							 GVariant *parameter,
							 gpointer user_data)
{
	NoteApplication *self = user_data;

	g_assert(NOTE_IS_APPLICATION(self));

	g_application_quit(G_APPLICATION(self));
}

static void
note_preferences_action(GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data)
{
	NoteApplication *self = user_data;
	NotePreferences *window;
	g_assert(NOTE_IS_APPLICATION(self));
	window = g_object_new(NOTE_TYPE_PREFERENCES, NULL);
	gtk_window_present(GTK_WINDOW(window));
}

// int selected_page=0;

// static void
// note_add_action(GSimpleAction *action,
// 				GVariant *parameter,
// 				gpointer user_data)
// {
// 	NoteApplication *self = user_data;
// 	NoteWindow *window;
// 	g_assert(NOTE_IS_APPLICATION(self));
// 	window = NOTE_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(self)));
// 	if(selected_page==0){
// 		note_add_init(window);
// 		selected_page=1;
// 	}else{
// 		note_add_close(window);
// 		selected_page=0;
// 	}
	
// 	/* gtk_widget_set_visible (GTK_WIDGET (window->note_list), false); */
// }

static const GActionEntry app_actions[] = {
	{"quit", note_application_quit_action},
	{"about", note_application_about_action},
	{"preferences", note_preferences_action},
	// {"add", note_add_action},
	// {"edit", note_edit_action},
};

static void
note_application_init(NoteApplication *self)
{
	g_action_map_add_action_entries(G_ACTION_MAP(self),
									app_actions,
									G_N_ELEMENTS(app_actions),
									self);
	gtk_application_set_accels_for_action(GTK_APPLICATION(self),
										  "app.quit",
										  (const char *[]){"<primary>q", NULL});
										  
}
