#!/usr/bin/env python3
"""Package the seven games and three standard backends without third-party runtimes."""
import argparse
import gzip
import hashlib
import json
from pathlib import Path
import platform
import re
import shutil
import subprocess
import tarfile
import tempfile


GAMES = ('snake', 'minesweeper', 'nibbler', 'pacman', 'qix', 'centipede', 'solarfox')
BACKENDS = ('ncurses', 'sdl2', 'sfml')
LAUNCHER = '''#!/bin/sh
set -eu
if [ "$#" -gt 1 ]; then
    echo "Usage: ./play [ncurses|sdl2|sfml]" >&2
    exit 84
fi
backend=${1:-ncurses}
case "$backend" in
    ncurses|sdl2|sfml) ;;
    *) echo "Unknown backend: $backend (choose ncurses, sdl2 or sfml)" >&2; exit 84 ;;
esac
package_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$package_dir"
exec ./arcade "./lib/arcade_${backend}.so"
'''


def command(*args):
    return subprocess.check_output(args, text=True).strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('--version', required=True)
    parser.add_argument('--output', type=Path, default=Path('dist'))
    args = parser.parse_args()
    if not re.fullmatch(r'v[0-9]+\.[0-9]+\.[0-9]+', args.version):
        parser.error('version must use vMAJOR.MINOR.PATCH')
    root = Path(__file__).resolve().parents[1]
    build = args.build.resolve()
    cache = (build / 'CMakeCache.txt').read_text()
    for setting in ('CMAKE_BUILD_TYPE:STRING=Release', 'ARCADE_BONUS:BOOL=ON',
                    'ARCADE_SANITIZERS:BOOL=OFF', 'ARCADE_EXTRA_GRAPHICS:BOOL=OFF'):
        if setting not in cache.splitlines():
            raise RuntimeError(f'Release build requires {setting}')
    system = {'Darwin': 'macos', 'Linux': 'linux'}.get(platform.system())
    architecture = {'aarch64': 'arm64', 'arm64': 'arm64', 'x86_64': 'x86_64'}.get(platform.machine())
    if not system or not architecture:
        raise RuntimeError('Only Linux/macOS arm64/x86_64 packages are supported')
    name = f'arcade-{args.version}-{system}-{architecture}'
    commit = command('git', '-C', str(root), 'rev-parse', 'HEAD')
    epoch = int(command('git', '-C', str(root), 'show', '-s', '--format=%ct', 'HEAD'))
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    archive_path = output / f'{name}.tar.gz'
    if archive_path.exists():
        raise RuntimeError(f'Refusing to replace existing archive: {archive_path}')
    with tempfile.TemporaryDirectory(prefix='arcade-package-') as temporary:
        package = Path(temporary) / name
        (package / 'lib').mkdir(parents=True)
        shutil.copy2(build / 'bin/arcade', package / 'arcade')
        for plugin in (*GAMES, *BACKENDS):
            shutil.copy2(build / 'lib' / f'arcade_{plugin}.so', package / 'lib')
        for notice in ('LICENSE', 'THIRD_PARTY_NOTICES.md'):
            shutil.copy2(root / notice, package / notice)
        (package / 'play').write_text(LAUNCHER)
        (package / 'play').chmod(0o755)
        (package / 'README.txt').write_text(f'''Arcade {args.version} — {system} {architecture}

Extract the entire directory, then run:
    ./play ncurses
    ./play sdl2
    ./play sfml

The launcher selects the package directory even when called from elsewhere.
Keep lib/ next to arcade. All seven games and their bonus modes are included.
The terminal must be at least 90 columns by 30 rows.

Runtime dependencies (not bundled):
- Linux: built on Ubuntu 22.04 with GCC 12; glibc >= 2.35 and a GCC 12-compatible
  libstdc++ are required. Install libncurses6. For graphical modes also install
  libsdl2-2.0-0 libsfml-graphics2.5 libsfml-window2.5 libsfml-system2.5 on Ubuntu 22.04.
  Other distributions need the matching library SONAMEs recorded in manifest.json.
- macOS: built for macOS 15+ Apple Silicon. ncurses uses the system runtime;
  graphical modes require Homebrew SDL2 and SFML (brew install sdl2 sfml).
  Keep the matching major runtime versions shown in manifest.json.

Menu: Up/Down selects a game, Left/Right changes backend, Enter plays.
F2/Tab backend; F3 game; F4 restart; F5 menu; F6 pause; D/F7 bonus; Esc quit.
Scores are created in data/. ARCADE_SCORE_FILE can select another writable path.

The ten additional graphics libraries and legacy adapters are available from source:
https://github.com/lelouchzr/arcade
License: MIT for original remake code; see THIRD_PARTY_NOTICES.md.
''')
        dependencies = {}
        for plugin in BACKENDS:
            binary = package / 'lib' / f'arcade_{plugin}.so'
            dependencies[plugin] = command(*(['otool', '-L'] if system == 'macos' else ['ldd']),
                                           str(binary)).splitlines()[1:]
        manifest = {
            'version': args.version, 'commit': commit, 'system': system,
            'architecture': architecture, 'build_os': platform.platform(),
            'dirty_source': bool(command('git', '-C', str(root), 'status', '--porcelain')),
            'games': list(GAMES), 'backends': list(BACKENDS), 'runtime_dependencies': dependencies,
            'files': {str(p.relative_to(package)): hashlib.sha256(p.read_bytes()).hexdigest()
                      for p in sorted(package.rglob('*')) if p.is_file()},
        }
        (package / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')

        def normalize(info):
            info.uid = info.gid = 0
            info.uname = info.gname = ''
            info.mtime = epoch
            info.pax_headers = {}
            return info

        temporary_archive = output / (archive_path.name + '.tmp')
        try:
            with temporary_archive.open('wb') as raw:
                with gzip.GzipFile(filename='', mode='wb', fileobj=raw, mtime=epoch) as compressed:
                    with tarfile.open(fileobj=compressed, mode='w') as archive:
                        for path in (package, *sorted(package.rglob('*'))):
                            archive.add(path, arcname=str(path.relative_to(package.parent)),
                                        recursive=False, filter=normalize)
            temporary_archive.replace(archive_path)
        finally:
            temporary_archive.unlink(missing_ok=True)
    digest = hashlib.sha256(archive_path.read_bytes()).hexdigest()
    archive_path.with_name(archive_path.name + '.sha256').write_text(f'{digest}  {archive_path.name}\n')
    print(archive_path)


if __name__ == '__main__':
    main()
