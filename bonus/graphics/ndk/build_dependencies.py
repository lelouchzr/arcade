#!/usr/bin/env python3
"""Build pinned upstream NDK++ sources with modern compiler/lifecycle compatibility fixes."""
import argparse
import hashlib
import os
import re
from pathlib import Path
import shutil
import subprocess
import tarfile
import tempfile
import urllib.request

ARCHIVES = (
    ('ncurses', 'https://invisible-mirror.net/archives/ncurses/ncurses-6.4.tar.gz',
     '6931283d9ac87c5073f30b6290c4c75f21632bb4fc3603ac8100812bed248159'),
    ('ncursesxx', 'ncursesxx/0.0.1beta4/ncursesxx-0.0.1beta4.tar.bz2',
     '62b689dabea004a1f98ad65a65602dc0782e5e24d0d87fef512b5d922274bcb0'),
    ('ndk', 'alt-ndk%2B%2B/0.0.1alpha4/ndk%2B%2B-0.0.1alpha4.tar.bz2',
     'f33c56abe654a2035790d177caacea60eb9dbab30903576ab627079e34d7cb9a'),
)


def replace(path, old, new):
    text = path.read_text()
    if old not in text:
        raise RuntimeError(f'Upstream source changed: {path}')
    path.write_text(text.replace(old, new, 1))


def build_terminal(tree, prefix):
    # Release roots with their SCREEN, then terminfo caches once at module teardown.
    # Keep this bridge inside the matched private library; the application uses no private layout.
    replace(tree / 'ncurses/base/lib_set_term.c', '\tWINDOWLIST *wl;',
            '#if NO_LEAKS\n'
            '        if (sp->_panelHook.destroy && sp->_panelHook.stdscr_pseudo_panel) {\n'
            '            sp->_panelHook.destroy(sp->_panelHook.stdscr_pseudo_panel);\n'
            '            sp->_panelHook.stdscr_pseudo_panel = 0;\n'
            '            sp->_panelHook.top_panel = 0;\n'
            '            sp->_panelHook.bottom_panel = 0;\n'
            '        }\n'
            '#endif\n\tWINDOWLIST *wl;')
    implementation = tree / 'ncurses/base/lib_freeall.c'
    with implementation.open('a') as output:
        output.write('\n#if !NO_LEAKS || !NCURSES_SP_FUNCS\n'
                     '#error "Arcade terminal requires leak cleanup and screen functions"\n'
                     '#endif\n'
                     'NCURSES_EXPORT(void) arcade_ndk_cleanup(void);\n'
                     'NCURSES_EXPORT(void) arcade_ndk_cleanup(void)\n'
                     '{ _nc_freeall_sp(0); }\n')
    env = dict(os.environ, CFLAGS='-O2 -fPIC')
    subprocess.run(['sh', str(tree / 'configure'), '--prefix=' + str(prefix),
                    '--without-shared', '--without-debug', '--without-ada', '--without-cxx',
                    '--without-cxx-binding', '--without-progs', '--without-tests',
                    '--without-manpages', '--without-gpm', '--disable-db-install',
                    '--disable-leaks', '--enable-sp-funcs', '--disable-widec',
                    '--enable-overwrite', '--disable-ext-colors', '--with-xterm-kbs=DEL',
                    '--with-default-terminfo-dir=/usr/share/terminfo',
                    '--with-terminfo-dirs=/etc/terminfo:/lib/terminfo:/usr/share/terminfo'],
                   cwd=tree, env=env, check=True)
    for folder in ('include', 'ncurses', 'panel'):
        subprocess.run(['make', '-C', str(tree / folder), '-j',
                        str(min(os.cpu_count() or 2, 8))], check=True)
    for folder, target in (('include', 'install'), ('ncurses', 'install.libs'),
                            ('panel', 'install.libs'), ('panel', 'install.includes')):
        subprocess.run(['make', '-C', str(tree / folder), target], check=True)
    license_dir = prefix / 'share/licenses/ncurses'
    license_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(tree / 'COPYING', license_dir)


def build(prefix, cache):
    prefix.mkdir(parents=True, exist_ok=True)
    cache.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='arcade-ndk-') as temp:
        workspace = Path(temp)
        trees = {}
        for name, url, digest in ARCHIVES:
            archive = cache / ('ncurses-6.4.tar.gz' if name == 'ncurses' else name + '.tar.bz2')
            if not archive.exists():
                address = url if url.startswith('https://') else (
                    'https://downloads.sourceforge.net/project/ndk-xx/' + url)
                urllib.request.urlretrieve(address, archive)
            if hashlib.sha256(archive.read_bytes()).hexdigest() != digest:
                raise RuntimeError(f'SHA256 mismatch: {archive}; remove it and retry')
            destination = workspace / name
            destination.mkdir()
            with tarfile.open(archive) as source:
                # Reject links and paths escaping the fresh extraction directory.
                for member in source.getmembers():
                    target = (destination / member.name).resolve()
                    if not target.is_relative_to(destination) or not (member.isfile() or member.isdir()):
                        raise RuntimeError(f'Unsafe archive member: {member.name}')
                options = {'filter': 'data'} if hasattr(tarfile, 'data_filter') else {}
                source.extractall(destination, **options)
            trees[name] = next(destination.iterdir())
        build_terminal(trees['ncurses'], prefix)
        nc, ndk = trees['ncursesxx'], trees['ndk']
        replace(nc / 'include/ncurses/basic_pen.hh', 'typedef acs<charT>\tacs;',
                'typedef ncurses::acs<charT>\tacs;')
        header_ndk = ndk / 'include/ndk++/transport.hh'
        text, count = re.subn(r'namespace ndk\s*\{',
                              'namespace ndk {\nclass engine;\nbool higher(engine *, engine *);',
                              header_ndk.read_text(), count=1)
        if count != 1:
            raise RuntimeError('NDK++ namespace missing')
        header_ndk.write_text(text)
        replace(ndk / 'src/text.cc', 'std::max(max_len,', 'std::max<unsigned int>(max_len,')
        header = nc / 'include/ncurses/application.hh'
        replace(header, '#include <curses.h>',
                '#include <curses.h>\nextern \"C\" void arcade_ndk_cleanup();')
        replace(header, 'int width() const;',
                'SCREEN *screen() const { return screen_; }\n\t\tvoid resume();\n\t\tvoid suspend();\n\t\tint width() const;')
        replace(ndk / 'src/application.cc', 'palette::obj_type &p = palette::instance();',
                'ncurses::color initialize_pool(COLOR_BLACK, COLOR_WHITE);\n\tpalette::obj_type &p = palette::instance();')
        implementation = nc / 'src/application.cc'
        replace(implementation, 'application::application()\n{',
                'application::application() : screen_(0) { resume(); }\n\n'
                'void application::resume()\n{\n'
                '\tif (screen_) { ::set_term(screen_); ::reset_prog_mode(); return; }')
        replace(implementation, 'application::~application()\n{\n\t::endwin();\n\t::delscreen(screen_);\n}',
                'application::~application() { suspend(); ::arcade_ndk_cleanup(); }\n\n'
                'void application::suspend()\n{\n'
                '\tif (!screen_) return;\n'
                '\tSCREEN *previous = ::set_term(screen_);\n\t::endwin();\n'
                '\tif (previous != screen_) ::set_term(previous);\n'
                '\t::delscreen(screen_);\n\tscreen_ = 0;\n}')
        flags = '-O2 -fPIC -fno-gnu-unique -std=gnu++98 -include cstdlib -include cstring'
        # Upstream Makefiles include headers from the adjacent ncursesxx source tree.
        include = f'-I{nc / "include"} -I{ndk / "include"} -I{prefix / "include"}'
        env = dict(os.environ)
        for name, tree in (('ncursesxx', nc), ('ndk++', ndk)):
            subprocess.run(['make', '-C', str(tree / 'src'), 'lib' + name + '.a', '-j', str(min(os.cpu_count() or 2, 8)),
                            'FLAGS=' + flags + ' ' + include], check=True, env=env)
            libraries = list(tree.rglob('lib' + name + '.a'))
            if len(libraries) != 1:
                raise RuntimeError(f'Expected one lib{name}.a, found {libraries}')
            (prefix / 'lib').mkdir(exist_ok=True)
            shutil.copy2(libraries[0], prefix / 'lib')
            shutil.copytree(tree / 'include', prefix / 'include', dirs_exist_ok=True)
            license_dir = prefix / 'share/licenses' / name
            license_dir.mkdir(parents=True, exist_ok=True)
            for license_file in tree.glob('COPYING*'):
                shutil.copy2(license_file, license_dir)
    print(f'NDK++ installed in {prefix}; pass -DARCADE_NDK_ROOT={prefix} to CMake')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--prefix', type=Path, required=True)
    parser.add_argument('--cache', type=Path, default=Path.home() / '.cache/arcade-ndk')
    args = parser.parse_args()
    build(args.prefix.resolve(), args.cache.resolve())
