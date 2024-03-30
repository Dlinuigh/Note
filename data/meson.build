desktop_file = i18n.merge_file(
        input: 'org.lion_claw.note.desktop.in',
       output: 'org.lion_claw.note.desktop',
         type: 'desktop',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'applications'
)

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
  test('Validate desktop file', desktop_utils, args: [desktop_file])
endif

appstream_file = i18n.merge_file(
        input: 'org.lion_claw.note.metainfo.xml.in',
       output: 'org.lion_claw.note.metainfo.xml',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'metainfo'
)

appstreamcli = find_program('appstreamcli', required: false, disabler: true)
test('Validate appstream file', appstreamcli,
     args: ['validate', '--no-net', '--explain', appstream_file])

schemas_dir = join_paths(get_option('datadir') , 'glib-2.0' , 'schemas')
settings_schemas = 'org.lion_claw.note.gschema.xml'

gnome.compile_schemas(build_by_default:true)
install_data(settings_schemas ,
  install_dir: schemas_dir
)

compile_schemas = find_program('glib-compile-schemas', required: false, disabler: true)
test('Validate schema file',
     compile_schemas,
     args: ['--strict', '--dry-run', meson.current_source_dir()])

subdir('icons')
