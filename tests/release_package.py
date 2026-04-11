#!/usr/bin/env python3
"""Verify an extracted release and play its real ncurses games from another directory."""
import argparse
import errno
import fcntl
import hashlib
import json
import os
from pathlib import Path
import platform
import pty
import select
import shutil
import signal
import struct
import subprocess
import tarfile
import tempfile
import termios
import time


def terminal(package, work, game_index, title):
    child, master = pty.fork()
    if child == 0:
        os.chdir(work)
        env = dict(os.environ, TERM='xterm-256color', ARCADE_SCORE_FILE=str(work / 'scores.tsv'))
        for variable in ('DISPLAY', 'ARCADE_LEGACY_GAME', 'ARCADE_LEGACY_GRAPHICS'):
            env.pop(variable, None)
        os.execve(package / 'play', [str(package / 'play'), 'ncurses'], env)
    fcntl.ioctl(master, termios.TIOCSWINSZ, struct.pack('HHHH', 40, 160, 0, 0))
    output = bytearray()
    reaped = False

    def read(timeout=.05):
        if select.select([master], [], [], timeout)[0]:
            try:
                output.extend(os.read(master, 65536))
            except OSError as error:
                if error.errno != errno.EIO:
                    raise

    def expect(value):
        values = value if isinstance(value, tuple) else (value,)
        deadline = time.monotonic() + 8
        while time.monotonic() < deadline:
            read()
            if all(item in output for item in values):
                output.clear()
                return
        raise AssertionError(f'Packaged terminal missing {value!r}: {output[:1500]!r} ... {output[-500:]!r}')

    try:
        expect(b'GAME LIBRARY')
        # Start each game on a fresh terminal, avoiding assumptions about ncurses delta output.
        os.write(master, b'\x1bOB' * game_index + b'\r')
        expect((title, b'ARCADE / 400'))
        os.write(master, b'\x1b')
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            read()
            done, status = os.waitpid(child, os.WNOHANG)
            if done:
                reaped = True
                assert os.WIFEXITED(status) and os.WEXITSTATUS(status) == 0, (status, output[-2000:])
                break
        else:
            raise AssertionError('Packaged terminal did not exit normally')
    finally:
        if not reaped:
            os.kill(child, signal.SIGKILL)
            os.waitpid(child, 0)
        os.close(master)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    parser.add_argument('--display-smoke', type=Path, required=True)
    args = parser.parse_args()
    archive = args.archive.resolve()
    expected = archive.with_name(archive.name + '.sha256').read_text().split()[0]
    assert hashlib.sha256(archive.read_bytes()).hexdigest() == expected
    with tempfile.TemporaryDirectory(prefix='arcade release with spaces ') as temporary:
        work = Path(temporary).resolve()
        with tarfile.open(archive) as source:
            for member in source.getmembers():
                assert (work / member.name).resolve().is_relative_to(work)
                assert member.isfile() or member.isdir()
            options = {'filter': 'data'} if hasattr(tarfile, 'data_filter') else {}
            source.extractall(work, **options)
        package = next(p for p in work.iterdir() if p.is_dir())
        manifest = json.loads((package / 'manifest.json').read_text())
        assert len(manifest['games']) == 7 and len(manifest['backends']) == 3
        for name, digest in manifest['files'].items():
            assert hashlib.sha256((package / name).read_bytes()).hexdigest() == digest, name
        help_result = subprocess.run([package / 'arcade', '--help'], cwd=work,
                                     capture_output=True, text=True, timeout=5)
        assert help_result.returncode == 0 and 'USAGE' in help_result.stdout, help_result
        invalid = subprocess.run([package / 'play', 'missing'], cwd=work,
                                 capture_output=True, text=True, timeout=5)
        assert invalid.returncode == 84 and 'Unknown backend' in invalid.stderr, invalid
        for index, title in enumerate((b'CENTIPEDE', b'MINESWEEPER', b'NIBBLER', b'PACMAN',
                                       b'QIX', b'SNAKE', b'SOLAR FOX')):
            terminal(package, work, index, title)
        smoke = args.display_smoke.resolve()
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_RENDER_DRIVER='software')
        subprocess.run([smoke, package / 'lib/arcade_sdl2.so'], env=env, check=True, timeout=15)
        if platform.system() == 'Linux':
            xvfb = shutil.which('xvfb-run')
            assert xvfb, 'xvfb-run is required to validate packaged SFML'
            subprocess.run([xvfb, '-a', smoke, package / 'lib/arcade_sfml.so'], check=True, timeout=20)
        print('Release verified: hashes, CLI errors, relocated launcher, all seven ncurses games, SDL2'
              + (', SFML' if platform.system() == 'Linux' else ''))


if __name__ == '__main__':
    main()
