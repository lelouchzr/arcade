#!/usr/bin/env python3
"""Drive bonus controls through the real core and the shared plugin ABI."""
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


def scenario(build, game, commands):
    with tempfile.TemporaryDirectory(prefix="arcade-bonus-integration-") as directory:
        work = Path(directory)
        (work / "lib").mkdir()
        shutil.copy2(build / "lib" / f"arcade_{game}.so", work / "lib")
        display = build / "tests" / "test_display_a.so"
        (work / "script").write_text("\n".join(["ENTER", *commands, "ESC"]) + "\n")
        env = dict(os.environ, ARCADE_TEST_SCRIPT=str(work / "script"),
                   ARCADE_TEST_LOG=str(work / "log"), ARCADE_SCORE_FILE=str(work / "scores.tsv"))
        env.pop("ARCADE_TEST_STEP", None)
        env.pop("ARCADE_LEGACY_GAME", None)
        env.pop("ARCADE_LEGACY_GRAPHICS", None)
        result = subprocess.run([str(build / "bin" / "arcade"), str(display)],
                                cwd=work, env=env, capture_output=True, text=True, timeout=15)
        assert result.returncode == 0, (game, result.returncode, result.stderr)
        return (work / "log").read_text()


def main():
    build = Path(sys.argv[1]).resolve()
    pacman = scenario(build, "pacman", ["TEXT d", "WAIT", "TEXT z", "WAIT",
                                        "TEXT v", "WAIT", "TEXT v", "WAIT"])
    assert "STATE CAMERA 1" in pacman and "STATE CAMERA 0" in pacman, pacman
    assert "SPRITES 1" in pacman, "Z did not launch a visible jump through the real core"
    mines = scenario(build, "minesweeper", ["TEXT ?", "TEXT ?", "WAIT", "TEXT f",
                                           "TEXT d", "TEXT d", "TEXT d", "RIGHT", "DOWN",
                                           "RIGHT", "ENTER", "WAIT"])
    assert "QUESTIONS 1" in mines, "question-mark binding was dropped by the core"
    assert "WIDTH 10 HEIGHT 10" in mines, "custom difficulty editor was not accessible"
    centipede = scenario(build, "centipede", ["TEXT d", "WAIT", "TEXT i", "TEXT l",
                                             "TEXT h", "WAIT"])
    assert "SECOND 1" in centipede, "bonus second pilot did not reach the display"
    assert "IJKL H P2" in centipede, "second pilot controls are not visible"
    print("Real-core bonus controls: jump, camera, question marks, custom board, second pilot passed")


if __name__ == "__main__":
    main()
