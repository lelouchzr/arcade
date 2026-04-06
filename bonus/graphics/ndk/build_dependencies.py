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


def build(prefix, cache):
    prefix.mkdir(parents=True, exist_ok=True)
    cache.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='arcade-ndk-') as temp:
        workspace = Path(temp)
        trees = {}
        for name, url, digest in ARCHIVES:
            archive = cache / (name + '.tar.bz2')
            if not archive.exists():
                urllib.request.urlretrieve('https://downloads.sourceforge.net/project/ndk-xx/' + url,
                                           archive)
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
                'application::~application() { suspend(); }\n\n'
                'void application::suspend()\n{\n'
                '\tif (!screen_) return;\n'
                '\tSCREEN *previous = ::set_term(screen_);\n\t::endwin();\n'
                '\tif (previous != screen_) ::set_term(previous);\n'
                '\t::delscreen(screen_);\n\tscreen_ = 0;\n}')
        flags = '-O2 -fPIC -fno-gnu-unique -std=gnu++98 -include cstdlib -include cstring'
        # Upstream Makefiles include headers from the adjacent ncursesxx source tree.
        include = f'-I{nc / "include"} -I{ndk / "include"}'
        env = dict(os.environ, CPLUS_INCLUDE_PATH=str(nc / 'include'))
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
