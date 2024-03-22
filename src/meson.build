blueprints = custom_target('blueprints',
  input: files(
    'gtk/help-overlay.blp',
    'note-preferences.blp',
    'note-window.blp',
  ),
  output: '.',
  command: [find_program('blueprint-compiler'), 'batch-compile', '@OUTPUT@', '@CURRENT_SOURCE_DIR@', '@INPUT@'],
)

note_sources = [
  'main.c',
  'note-application.c',
  'note-window.c',
  'note-preferences.c',
]

note_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
  dependency('json-glib-1.0'),
]

note_sources += gnome.compile_resources('note-resources',
  'note.gresource.xml',
  c_name: 'note',
  dependencies: blueprints,
)

executable('note', note_sources,
  dependencies: note_deps,
       install: true,
)

