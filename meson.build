project(
  'gparted',
  'cpp',
  version : '0.31.0',
  meson_version: '>= 0.46.0',
)

cpp = meson.get_compiler('cpp')

add_project_arguments('-DHAVE_CONFIG_H', language: 'cpp')
add_project_arguments('-DGNOME_ICONDIR="' + join_paths(get_option('datadir'), 'pixmaps') + '"', language: 'cpp')
add_project_arguments('-DGNOMELOCALEDIR="' + join_paths(get_option('datadir'), 'locale') + '"', language: 'cpp')

glibmm_dep = dependency('glibmm-2.4', version: '>=2.14.0')
gtkmm_dep = dependency('gtkmm-2.4', version: '>2.8')
gthread_dep = dependency('gthread-2.0')
libdl_dep = cpp.find_library('dl')
libparted_dep = dependency('libparted', version: '>=1.7.1')
uuid_dep = dependency('uuid')

conf = configuration_data()

if get_option('man')
  subdir('doc')
endif

if get_option('libparted_dmraid')
  conf.set('USE_LIBPARTED_DMRAID', 1)
endif

if get_option('online-resize')
  conf.set('ENABLE_ONLINE_RESIZE', 1)
endif

gparted_version = meson.project_version()
conf.set_quoted('VERSION', gparted_version)
conf.set_quoted('PACKAGE_VERSION', gparted_version)
conf.set_quoted('PACKAGE_STRING', 'gparted' + gparted_version)

# Define variables dependening on what versions of the libs are installed

# Check for libparted 2.0 to 3.0 inclusive for a bug where loop table
# creation doesn't delete old partitions.
if dependency('libparted', version: ['>=2', '<=3'], required: false).found()
  conf.set('ENABLE_LOOP_DELETE_OLD_PTNS_WORKAROUND', true)
else
  conf.set('ENABLE_LOOP_DELETE_OLD_PTNS_WORKAROUND', false)
endif

if dependency('libparted', version: '>=2.2', required: false).found()
  conf.set('USE_LIBPARTED_LARGE_SECTOR_SUPPORT', 1)
else
  conf.set('ENABLE_PT_REREAD_WORKAROUND', 1)
endif

parted_fs_resize_dep = [ ]
if cpp.has_function('ped_file_system_resize', dependencies: libparted_dep)
  conf.set('HAVE_LIBPARTED_FS_RESIZE', 1)
else
  parted_fs_resize_dep = cpp.find_library('parted-fs-resize')
  conf.set('HAVE_LIBPARTED_FS_RESIZE', 1)
endif

if dependency('libparted', version: '>=3.2', required: false).found()
  conf.set('HAVE_LIBPARTED_FS_RESIZE', 1)
  parted_fs_resize_dep = cpp.find_library('parted-fs-resize')
endif

if dependency('gtkmm-2.4', version: '>=2.22.0', required: false).found()
  conf.set('HAVE_GET_MESSAGE_AREA', 1)
endif

cxx_std = 'c++98'

if dependency('gtkmm-2.4', version: '>=2.45.40', required: false).found()
  cxx_std = 'c++11'
endif

if dependency('sigc++-2.0', version: '>-2.5.1', required: false).found()
  cxx_std = 'c++11'
endif

conf.set('PACKAGE', 'gparted')
conf.set('PACKAGE_NAME', 'gparted')

#TODO: Optionalize the below if necessary
conf.set('ENABLE_NLS', true)
conf.set('HAVE_BIND_TEXTDOMAIN_CODESET', 1)
conf.set('HAVE_DCGETTEXT', 1)
conf.set('HAVE_DLFCN_H', 1)
conf.set('HAVE_GETTEXT', 1)
conf.set('HAVE_GET_MESSAGE_AREA', 1)
conf.set('HAVE_GLIB_REGEX', 1)
conf.set('HAVE_GTK_SHOW_URI', 1)
conf.set('HAVE_INTTYPES_H', 1)
conf.set('HAVE_LV_MESSAGES', 1)
conf.set('HAVE_LIBPARTED', 1)
conf.set('HAVE_LIBUUID', 1)
conf.set('HAVE_LOCAL_H', 1)
conf.set('HAVE_MEMORY_H', 1)
conf.set('HAVE_SET_DEFAULT_ICON_NAME', 1)
conf.set('HAVE_STDINT_H', 1)
conf.set('HAVE_STRINGS_H', 1)
conf.set('HAVE_STRING_H', 1)
conf.set('HAVE_SYS_STAT_H', 1)
conf.set('HAVE_SYS_TYPES_H', 1)
conf.set('HAVE_UNISTD_H', 1)
conf.set('STDC_HEADERS', 1)
conf.set('ENABLE_HELP_DOC', false)

configure_file(
  output: 'config.h',
  configuration: conf
)

include = [
  include_directories('include'),
  include_directories('src'),
  include_directories('.'),
]

# po stuff
i18n = import('i18n')

intltool = find_program('intltool-merge')

gettext_package = 'gparted'
add_project_arguments('-DGETTEXT_PACKAGE="' + gettext_package + '"', language: 'cpp')
podir= 'po'
subdir(podir)

po_conf = configuration_data()

po_conf.set('bindir', join_paths(get_option('prefix'), get_option('bindir')))

desktop_in = configure_file(
  input: 'gparted.desktop.in.in',
  output: 'gparted.desktop.in',
  configuration: po_conf
)

i18n.merge_file(
  input: desktop_in,
  output: 'gparted.desktop',
  type: 'desktop',
  po_dir: 'po',
  install: true,
  install_dir: join_paths(get_option('datadir'), 'applications')
)

custom_target(
  'gparted.appdata.xml',
  input: 'gparted.appdata.xml.in',
  output: 'gparted.appdata.xml',
  command: [ intltool, '--xml-style', podir, '@INPUT@', '@OUTPUT@'],
  install: true,
  install_dir: join_paths(get_option('datadir'), 'appdata')
)

if get_option('polkit')
  polkit_dep = dependency('polkit-agent-1', version: '>=0.102')
  polkitpolicydir = join_paths(get_option('datadir'), 'polkit-1/actions')

  polkit_policy_in = configure_file(
    input: 'org.gnome.gparted.policy.in.in',
    output: 'org.gnome.gparted.policy.in',
    configuration: po_conf
  )

  i18n.merge_file(
    input: polkit_policy_in,
    output: 'org.gnome.gparted.policy',
    po_dir: 'po',
    install: true,
    install_dir: polkitpolicydir
  )
endif

subdir('data')
subdir('src')
