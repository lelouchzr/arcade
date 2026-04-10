#!/usr/bin/env python3
"""Run real NDK++ reopen/unload cycles in a PTY, optionally enforcing Valgrind results."""
import argparse
import errno
import fcntl
import os
from pathlib import Path
import pty
import select
import signal
import struct
import termios
import time
import xml.etree.ElementTree as ET

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('binary')
parser.add_argument('plugin')
parser.add_argument('--valgrind')
parser.add_argument('--log', type=Path, default=Path('ndk-valgrind'))
parser.add_argument('--cycles', type=int, default=32)
args = parser.parse_args()
command = [args.binary, args.plugin, str(args.cycles)]
xml_path = Path(str(args.log) + '.xml').resolve()
text_path = Path(str(args.log) + '.log').resolve()
if args.valgrind:
    command = [args.valgrind, '--tool=memcheck', '--leak-check=full', '--default-suppressions=no',
               '--show-leak-kinds=all', '--errors-for-leak-kinds=all',
               '--error-exitcode=99', '--keep-debuginfo=yes', '--num-callers=24',
               '--xml=yes', '--xml-file=' + str(xml_path),
               '--log-file=' + str(text_path), *command]
child, master = pty.fork()
if child == 0:
    os.execvpe(command[0], command, dict(os.environ, TERM='xterm', AAOPTS=''))
fcntl.ioctl(master, termios.TIOCSWINSZ, struct.pack('HHHH', 40, 120, 0, 0))
output = bytearray()
reaped = False
status = None
deadline = time.monotonic() + 90
try:
    while time.monotonic() < deadline:
        if select.select([master], [], [], .05)[0]:
            try:
                data = os.read(master, 65536)
            except OSError as error:
                if error.errno != errno.EIO:
                    raise
                data = b''
            if data:
                output.extend(data)
            elif reaped:
                break
        if not reaped:
            done, status = os.waitpid(child, os.WNOHANG)
            reaped = bool(done)
    else:
        raise RuntimeError('NDK++ lifecycle timeout')
    if not reaped or not os.WIFEXITED(status) or os.WEXITSTATUS(status):
        raise RuntimeError(f'NDK++ lifecycle failed: {status}; {output[-1000:]!r}')
    if b'NDK_LIFECYCLE_OK:' not in output:
        raise RuntimeError('NDK++ lifecycle completion marker missing')
    print(output[output.rfind(b'NDK_LIFECYCLE_OK:'):].decode().strip())
    if args.valgrind:
        document = ET.parse(xml_path).getroot()
        errors = document.findall('error')
        finished = document.findall("status[state='FINISHED']")
        suppressed = sum(int(item.findtext('count', '0')) for item in document.findall('suppcounts/pair'))
        if errors or not finished or suppressed:
            details = [item.findtext('kind') for item in errors]
            raise RuntimeError(f'Valgrind errors or incomplete run: {details}')
        print('Valgrind: 0 errors, 0 lost blocks, 0 reachable allocations; no suppressions')
finally:
    if not reaped:
        os.kill(child, signal.SIGKILL)
        os.waitpid(child, 0)
    os.close(master)
