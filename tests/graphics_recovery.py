#!/usr/bin/env python3
"""Check failed display switches, including real ncurses -> Xlib without DISPLAY."""
import errno
import fcntl
import os
from pathlib import Path
import pty
import select
import shutil
import signal
import struct
import subprocess
import sys
import tempfile
import termios
import time


def environment(work):
    env = dict(os.environ, ARCADE_TEST_SCRIPT=str(work / 'script'),
               ARCADE_TEST_LOG=str(work / 'log'), ARCADE_SCORE_FILE=str(work / 'scores.tsv'))
    for name in ('ARCADE_TEST_STEP', 'ARCADE_TEST_CAPTURE', 'ARCADE_LEGACY_GAME',
                 'ARCADE_LEGACY_GRAPHICS', 'DISPLAY'):
        env.pop(name, None)
    env['ARCADE_TEST_FAIL_OPEN'] = 'script_b'
    return env


def scripted(build, work):
    for suffix in ('a', 'b', 'c'):
        shutil.copy2(build / 'tests' / f'test_display_{suffix}.so', work / 'lib')
    actions = ['ENTER', 'SPACE', 'F6', 'WAIT', 'F2', 'WAIT', 'F2', 'WAIT',
               'F2', 'WAIT', 'F5', 'LEFT', 'WAIT', 'RIGHT', 'WAIT', 'ESC']
    (work / 'script').write_text('\n'.join(actions) + '\n')
    result = subprocess.run([build / 'bin' / 'arcade', work / 'lib' / 'test_display_a.so'],
                            cwd=work, env=environment(work), capture_output=True,
                            text=True, timeout=15)
    assert result.returncode == 0, (result.returncode, result.stderr)
    log = (work / 'log').read_text()
    assert log.count('OPEN script_b\n') == 1, log
    assert 'DISPLAY UNAVAILABLE: script_b - test display unavailable' in log, log
    assert '[--] OFFSCREEN PREVIEW' in log, log
    assert log.count('OPEN script_a\n') >= 3 and log.count('OPEN script_c\n') >= 2, log
    paused = log.split('INPUT F6\n', 1)[1].split('INPUT F5\n', 1)[0]
    sessions = [line for line in paused.splitlines() if line.startswith('SESSION ')]
    assert len(sessions) >= 6 and len(set(sessions)) == 1, paused
    assert paused.count('PAUSED') >= 6, paused

    # An explicitly requested backend that cannot open must still fail at startup.
    failure = subprocess.run([build / 'bin' / 'arcade', work / 'lib' / 'test_display_b.so'],
                             cwd=work, env=environment(work), capture_output=True,
                             text=True, timeout=5)
    assert failure.returncode == 84 and 'test display unavailable' in failure.stderr, failure
    print('Failed display skipped in both directions; score/generation/pause retained; startup 84 OK')


def native(build, work):
    if not all((build / 'lib' / f'arcade_{name}.so').exists() for name in ('ncurses', 'xlib')):
        print('Native ncurses/Xlib regression skipped: optional modules not built')
        return
    for path in (work / 'lib').glob('test_display_*.so'):
        path.unlink()
    for name in ('ncurses', 'xlib'):
        shutil.copy2(build / 'lib' / f'arcade_{name}.so', work / 'lib')
    failure = subprocess.run([build / 'bin' / 'arcade', './lib/arcade_xlib.so'],
                             cwd=work, env=environment(work), capture_output=True,
                             text=True, timeout=5)
    assert failure.returncode == 84 and 'Xlib: cannot open display' in failure.stderr, failure
    child, master = pty.fork()
    if child == 0:
        os.chdir(work)
        env = environment(work)
        env['TERM'] = 'xterm-256color'
        os.execve(build / 'bin' / 'arcade',
                  [str(build / 'bin' / 'arcade'), './lib/arcade_ncurses.so'], env)
    fcntl.ioctl(master, termios.TIOCSWINSZ, struct.pack('HHHH', 40, 160, 0, 0))
    output = bytearray()
    reaped = False

    def expect(text):
        deadline = time.monotonic() + 8
        while time.monotonic() < deadline:
            if text in output:
                return
            if select.select([master], [], [], .05)[0]:
                try:
                    data = os.read(master, 65536)
                except OSError as error:
                    if error.errno != errno.EIO:
                        raise
                    data = b''
                if not data:
                    break
                output.extend(data)
        raise AssertionError(f'Missing native output {text!r}: {output[-3000:]!r}')

    try:
        expect(b'GAME LIBRARY')
        os.write(master, b'\x1bOC')  # xterm application-mode Right, as enabled by ncurses.
        expect(b'DISPLAY UNAVAILABLE: xlib')
        expect(b'Xlib: cannot open display')
        output.clear()
        os.write(master, b'\r')
        expect(b'SCORE')
        expect(b'DISPLAY UNAVAILABLE: xlib')
        os.write(master, b'\t')  # The failed backend must now be skipped.
        time.sleep(.1)
        os.write(master, b'\x1b')
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            # Keep draining the PTY: a full output buffer can block ncurses before it polls Esc.
            if select.select([master], [], [], .05)[0]:
                try:
                    output.extend(os.read(master, 65536))
                except OSError as error:
                    if error.errno != errno.EIO:
                        raise
            done, status = os.waitpid(child, os.WNOHANG)
            if done:
                reaped = True
                assert os.WIFEXITED(status) and os.WEXITSTATUS(status) == 0, (status, output[-10000:])
                break
            time.sleep(.05)
        else:
            raise AssertionError('Native ncurses did not exit normally after failed Xlib switch')
        print('Real ncurses: no DISPLAY, Xlib refusal recovered, game playable, Tab and Escape OK')
    finally:
        if not reaped:
            os.kill(child, signal.SIGKILL)
            os.waitpid(child, 0)
        os.close(master)


def main():
    build = Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix='arcade-graphics-recovery-') as directory:
        work = Path(directory)
        (work / 'lib').mkdir()
        shutil.copy2(build / 'lib' / 'arcade_minesweeper.so', work / 'lib')
        scripted(build, work)
        native(build, work)


if __name__ == '__main__':
    main()
