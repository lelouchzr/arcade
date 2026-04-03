#!/usr/bin/env python3
"""Exercise the real launcher with deterministic display plugins and no GUI."""
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root, build = (Path(argument).resolve() for argument in sys.argv[1:])
with tempfile.TemporaryDirectory(prefix='arcade-integration-') as directory:
    work = Path(directory)
    (work / 'lib').mkdir()
    for name in ('arcade_snake.so', 'arcade_minesweeper.so', 'arcade_nibbler.so'):
        source = build / 'lib' / name
        if source.exists():
            shutil.copy2(source, work / 'lib' / name)
    for suffix in ('a', 'b'):
        shutil.copy2(build / 'tests' / f'test_display_{suffix}.so', work / 'lib')
    (work / 'lib' / 'foreign.so').write_text('not an executable shared library')
    script = ['BACKSPACE'] * 6 + ['TEXT A', 'TEXT D', 'TEXT A', 'ENTER', 'SPACE']
    script += ['WAIT'] * 3 + ['F6', 'WAIT', 'F6', 'F2', 'F7', 'SPACE', 'F4', 'SPACE']
    script += ['F5', 'ENTER', 'SPACE'] + ['F3'] * (2 if (build / 'lib/arcade_nibbler.so').exists() else 1) + ['WAIT', 'F4', 'F5', 'ESC']
    (work / 'script').write_text('\n'.join(script) + '\n')
    env = dict(os.environ, ARCADE_TEST_SCRIPT=str(work / 'script'),
               ARCADE_TEST_LOG=str(work / 'log'), ARCADE_SCORE_FILE=str(work / 'scores.tsv'))
    env.pop('ARCADE_TEST_STEP', None)
    result = subprocess.run([build / 'bin' / 'arcade', work / 'lib' / 'test_display_a.so'],
                            cwd=work, env=env, capture_output=True, text=True, timeout=15)
    assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
    log = (work / 'log').read_text()
    for expected in ('OPEN script_a', 'OPEN script_b', 'CLOSE script_a', 'PAUSED', 'ADA_',
                     '01 / MINESWEEPER', '01 / SNAKE', '02 / SESSION'):
        assert expected in log, f'Missing integration evidence: {expected}\n{log}'
    scores = (work / 'scores.tsv').read_text()
    assert '"minesweeper" "ADA"' in scores, scores
    assert len(scores.splitlines()) >= 3, scores
    for arguments in ([], ['one', 'two'], ['missing.so'], ['lib/arcade_snake.so'], ['script']):
        failure = subprocess.run([build / 'bin' / 'arcade', *arguments], cwd=work, env=env,
                                 capture_output=True, text=True, timeout=5)
        assert failure.returncode == 84, (arguments, failure.returncode)
        assert failure.stderr, arguments
    help_result = subprocess.run([build / 'bin' / 'arcade', '--help'], cwd=work, capture_output=True, timeout=5)
    assert help_result.returncode == 0
    print('Real-core integration: names, scores, game/display switching, pause, restart, menu, errors OK')

with tempfile.TemporaryDirectory(prefix='arcade-seven-games-') as directory:
    work = Path(directory)
    (work / 'lib').mkdir()
    names = []
    for game, title in (('centipede', 'CENTIPEDE'), ('minesweeper', 'MINESWEEPER'),
                        ('nibbler', 'NIBBLER / 32'), ('pacman', 'PACMAN'),
                        ('qix', 'QIX'), ('snake', 'SNAKE'), ('solarfox', 'SOLAR FOX')):
        source = build / 'lib' / f'arcade_{game}.so'
        if source.exists():
            shutil.copy2(source, work / 'lib' / source.name)
            names.append(title)
    for suffix in ('a', 'b'):
        shutil.copy2(build / 'tests' / f'test_display_{suffix}.so', work / 'lib')
    actions = ['ENTER']
    for _ in names:
        actions += ['WAIT', 'F2', 'F4', 'F3']
    actions += ['F5'] + ['DOWN'] * (len(names) - 1) + ['WAIT', 'ESC']
    (work / 'script').write_text('\n'.join(actions) + '\n')
    env = dict(os.environ, ARCADE_TEST_SCRIPT=str(work / 'script'),
               ARCADE_TEST_LOG=str(work / 'log'), ARCADE_SCORE_FILE=str(work / 'scores.tsv'))
    env.pop('ARCADE_TEST_STEP', None)
    result = subprocess.run([build / 'bin' / 'arcade', work / 'lib' / 'test_display_a.so'],
                            cwd=work, env=env, capture_output=True, text=True, timeout=15)
    assert result.returncode == 0, result.stderr
    log = (work / 'log').read_text()
    for title in names:
        assert f'01 / {title}' in log, f'Game not exercised by real core: {title}'
    assert f'> {names[-1]}' in log, 'Menu did not scroll to its last game'
    print(f'Real-core integration: all {len(names)} games, hot display switches, restarts and scrolling OK')
