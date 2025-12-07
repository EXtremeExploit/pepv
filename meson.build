project(
    'pepv',
    'c',
    'cpp',
    default_options: ['c_std=c18', 'cpp_std=c++20'],
    meson_version: '>=1.1',
)

# TODO: Add versioning

if get_option('buildtype') == 'debug'
    add_project_arguments('-DDEBUG', language: 'cpp')
endif

threads = dependency('threads')
gtk = dependency('gtk+-3.0')
libalpm = dependency('libalpm')
tracy = dependency('tracy')

message('prefix: ' + get_option('prefix'))
message('datadir: ' + get_option('datadir'))
message('buildtype: ' + get_option('buildtype'))

if get_option('tracy_enable') and get_option('buildtype') != 'debugoptimized'
    warning('Profiling builds should set -- buildtype = debugoptimized')
endif

executable(
    'pepv',
    ['src/events.cpp', 'src/main.cpp', 'src/pkgs.cpp', 'src/utils.cpp'],
    dependencies: [threads, gtk, libalpm, tracy],
    export_dynamic: true, # GTK needs this
    cpp_args: [
        '-DPROJECTNAME="' + meson.project_name() + '"',
        '-DPREFIX="' + get_option('prefix') + '"',
        '-DDATADIR="' + get_option('datadir') + '"',
        '-Werror',
    ],
    install: true,
)

install_data('data/pepv.ui')

install_data(
    'data/pepv.desktop',
    install_dir: get_option('prefix') / get_option('datadir') / 'applications',
)

install_data(
    'README.md',
    install_dir: get_option('prefix') / get_option('datadir') / 'doc' / meson.project_name(),
)

install_data(
    'LICENSE',
    rename: 'COPYING',
    install_dir: get_option('prefix') / get_option('datadir') / 'licenses' / meson.project_name(),
)

# TODO: better install dir for this to make desktop and gtk icons work on debug builds/uninstalled tests
foreach size : [16, 22, 24, 32, 36, 48, 64, 72, 96, 128, 256, 512, 1024]
    install_data(
        'data/icons/sizes/pepv-@0@.png'.format(size),
        install_dir: get_option('prefix') / get_option('datadir') / 'icons/hicolor/@0@x@0@/apps/'.format(size),
        rename: meson.project_name() + '.png',
        install_mode: 'rw-r--r--',
    )
endforeach
