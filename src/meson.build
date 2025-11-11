petrock_sources = [
  'main.c',
  'petrock-application.c',
  'petrock-window.c',
]

petrock_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
]

petrock_sources += gnome.compile_resources('petrock-resources',
  'petrock.gresource.xml',
  c_name: 'petrock'
)

executable('petrock', petrock_sources,
  dependencies: petrock_deps,
       install: true,
)
