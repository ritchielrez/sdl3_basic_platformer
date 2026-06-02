# Knightfall

A basic 2D platformer built with SDL3 and C++20.

## Building

### Prerequisites

- CMake >= 3.10
- A C++20 compiler (GCC, Clang, or MSVC)
- Linux: ALSA development headers (`libasound2-dev` on Debian/Ubuntu)

All other dependencies are vendored in `vendor/` and built as static libraries.

### Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Debug Build

The debug build includes an ImGui overlay (toggle with F1):

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Run

```sh
./build/main
```

Run from the project root directory so assets are found at `./assets/`.

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).

Assets are CC0. See [assets/LICENSE & CREDITS.txt](assets/LICENSE%20%26%20CREDITS.txt).
