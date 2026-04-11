#!/usr/bin/env python3
"""Capture the real core's draw commands with the offscreen test renderer."""
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from PIL import Image

root = Path(__file__).resolve().parents[1]
build = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else root / 'build'
output = root / 'screenshots'
output.mkdir(exist_ok=True)
games = [name for name in ('centipede', 'minesweeper', 'nibbler', 'pacman', 'qix', 'snake', 'solarfox')
         if (build / 'lib' / f'arcade_{name}.so').exists()]
scenes = {'menu': ['WAIT']}
for index, game in enumerate(games):
    scenes[game] = ['DOWN'] * index + ['ENTER'] + (['SPACE'] if game == 'minesweeper' else ['WAIT'])
    if game != 'minesweeper':
        actions = ['DOWN'] * index + ['ENTER', 'F7']
        if game == 'pacman':
            actions += ['SPACE'] + ['WAIT'] * 10
        elif game == 'qix':
            actions += ['RIGHT'] * 3
        elif game == 'solarfox':
            actions += ['WAIT'] * 20
        scenes[f'{game}_bonus'] = actions
        if game == 'nibbler':
            scenes['nibbler_original'] = ['DOWN'] * index + ['ENTER', 'F7', 'F7']
    else:
        scenes['minesweeper_custom'] = ['DOWN'] * index + ['ENTER'] + ['F7'] * 3 + [
            'TEXT 4', 'TEXT 0', 'DOWN', 'TEXT 2', 'TEXT 4', 'DOWN', 'TEXT 9', 'TEXT 9',
            'ENTER', 'SPACE']
with tempfile.TemporaryDirectory(prefix='arcade-capture-') as directory:
    work = Path(directory)
    (work / 'lib').mkdir()
    for source in (build / 'lib').glob('arcade_*.so'):
        shutil.copy2(source, work / 'lib' / source.name)
    for name, actions in scenes.items():
        (work / 'script').write_text('\n'.join([*actions, 'ESC']) + '\n')
        env = dict(os.environ, ARCADE_TEST_SCRIPT=str(work / 'script'),
                   ARCADE_TEST_CAPTURE=str(work / 'capture.ppm'),
                   ARCADE_SCORE_FILE=str(work / 'scores.tsv'))
        env.pop('ARCADE_TEST_STEP', None)
        env.pop('ARCADE_LEGACY_GAME', None)
        env.pop('ARCADE_LEGACY_GRAPHICS', None)
        subprocess.run([build / 'bin' / 'arcade', build / 'tests' / 'test_display_a.so'],
                       cwd=work, env=env, check=True, timeout=15)
        with Image.open(work / 'capture.ppm') as image:
            image.save(output / f'{name}.png')
        print(output / f'{name}.png')
