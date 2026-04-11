# Binary releases

[Download the latest release](https://github.com/lelouchzr/arcade/releases/latest).

| Archive | Target |
|---|---|
| `arcade-v3.0.0-linux-x86_64.tar.gz` | Linux x86-64, Ubuntu 22.04 build, GCC 12 runtime, glibc ≥2.35 |
| `arcade-v3.0.0-macos-arm64.tar.gz` | macOS 15+, Apple Silicon |

Each archive contains the executable, all seven game plugins and bonuses, the three standard backends, a launcher, license/notices, and a SHA256 manifest. Graphics runtimes are installed separately as described in the included README. The ten additional backends remain available in source builds.

```sh
tar -xzf arcade-v3.0.0-linux-x86_64.tar.gz
cd arcade-v3.0.0-linux-x86_64
./play ncurses
# With SDL2/SFML runtimes installed:
./play sdl2
./play sfml
```

Use `sha256sum -c ARCHIVE.tar.gz.sha256` on Linux or `shasum -a 256 -c ARCHIVE.tar.gz.sha256` on macOS. Keep `lib/` next to the executable. The launcher sets the working directory automatically.

## Maintainer workflow

Run **Release binaries** in GitHub Actions with a semantic version. It builds and tests both platforms, checks extracted archives, then uploads artifacts. Publish the verified archives and checksums in a GitHub release pointing to that exact source commit.

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DARCADE_EXTRA_GRAPHICS=OFF
cmake --build build-release --parallel 4
ctest --test-dir build-release --output-on-failure
python3 tools/package_release.py build-release --version v3.0.0
python3 tests/release_package.py dist/*.tar.gz --display-smoke build-release/tests/display_smoke
```

Raw build, test, and profiling logs are excluded from Git. Workflow logs are available in [GitHub Actions](https://github.com/lelouchzr/arcade/actions).
