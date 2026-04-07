#!/usr/bin/env python3
"""Exercise actual display plugins under Xvfb and a real pseudoterminal."""
import errno
import fcntl
import os
from pathlib import Path
import pty
import re
import select
import signal
import struct
import subprocess
import sys
import termios
import time

binary, libraries = sys.argv[1:3]
libraries = Path(libraries)
names = [name for name in ('aalib', 'ndk++', 'ncurses', 'libcaca', 'sdl2', 'sfml',
                           'allegro5', 'gtk+', 'qt5', 'xlib', 'irrlicht', 'opengl', 'vulkan')
         if (libraries / ('arcade_' + name + '.so')).exists()
         and (len(sys.argv) == 3 or name in sys.argv[3:])]
terminal = [name for name in names if name in ('ncurses', 'ndk++', 'aalib', 'libcaca')]
env = dict(os.environ, TERM='xterm', CACA_DRIVER='slang', ARCADE_AA_DRIVER='slang',
           AAOPTS='')
if names:
    pid, master = pty.fork()
    if pid == 0:
        env['ARCADE_TEST_TERMINAL_KEYS'] = '1'
        os.execve(binary, [binary, str(libraries), *names], env)
    fcntl.ioctl(master, termios.TIOCSWINSZ, struct.pack('HHHH', 60, 180, 0, 0))
    deadline = time.monotonic() + 90
    output = bytearray()
    pending = bytearray()
    status = None
    done = 0
    try:
        while time.monotonic() < deadline:
            if select.select([master], [], [], .1)[0]:
                try:
                    data = os.read(master, 65536)
                except OSError as error:
                    if error.errno != errno.EIO:
                        raise
                    break
                if not data:
                    break
                output.extend(data)
                pending.extend(data)
                while b'TERMINAL_READY' in pending:
                    _, _, suffix = pending.partition(b'TERMINAL_READY')
                    pending = bytearray(suffix)
                    os.write(master, b'd')
            done, status = os.waitpid(pid, os.WNOHANG)
            if done:
                break
        else:
            raise RuntimeError('Terminal display test timed out')
        while not done:
            done, status = os.waitpid(pid, os.WNOHANG)
            if time.monotonic() >= deadline:
                raise RuntimeError('Native display cleanup timed out')
            if not done:
                time.sleep(.01)
        # Drain bytes emitted just before the child exited.
        while select.select([master], [], [], 0)[0]:
            try:
                data = os.read(master, 65536)
            except OSError as error:
                if error.errno != errno.EIO:
                    raise
                break
            if not data:
                break
            output.extend(data)
        Path('native-terminal-output.bin').write_bytes(output)
        if not os.WIFEXITED(status) or os.WEXITSTATUS(status):
            print(output[-5000:].decode(errors='replace'))
            raise RuntimeError(f'Terminal graphics failed, status {status}')
        if b'real game/display frames; hot replacement and unloading passed' not in output:
            raise RuntimeError('Native display test did not reach its completion marker')
        for title in (b'SNAKE', b'MINESWEEPER', b'NIBBLER', b'PACMAN', b'QIX', b'CENTIPEDE', b'SOLAR FOX'):
            if terminal and title not in output:
                raise RuntimeError(f'Terminal output lost game title: {title!r}')
        pattern = b'(' + b'|'.join(re.escape(name.encode()) for name in names)
        pattern += rb'): presentation, events and seven games passed; round [12]'
        for match in re.finditer(pattern, output):
            print(match.group().decode())
        print(f'{len(names) * 28} real game/display frames across two mixed GUI/terminal cycles')
        print(f'{len(terminal)} terminal displays: rendering, keyboard, two lifecycle rounds passed')
    finally:
        if not done:
            try:
                os.kill(pid, signal.SIGKILL)
                os.waitpid(pid, 0)
            except ProcessLookupError:
                pass
        os.close(master)
print(f'{len(names)} actual display plugins verified')
