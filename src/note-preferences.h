#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define NOTE_TYPE_PREFERENCES (note_preferences_get_type())

G_DECLARE_FINAL_TYPE (NotePreferences, note_preferences, NOTE, PREFERENCES, AdwPreferencesWindow)

G_END_DECLS
