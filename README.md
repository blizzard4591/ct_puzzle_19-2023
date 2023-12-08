# c't Puzzle from Issue 19/2023 and 28/2023
See [here](https://www.heise.de/select/ct/2023/28/2329707360694897070) and [here](https://github.com/607011/chilly) for more information on the game.

This tool can find the shortest path to the exit (classic version) or collect all presents and find the shortest path (christmas version).
Keep in mind that solving the christmas version takes about 42 Gigabytes of RAM.
You have been warned.

## License
The tool is governed by the GNU GPL v2.0 license. See [`LICENSE`](LICENSE) for more information.

## Requirements
 - [CMake](https://cmake.org/)
 - C++17, meaning a recent MSVC/GCC/Clang that supports at least C++17
 - LZ4
 
## How to build

On Linux or MacOS, if CMake, `liblz4-dev` and a compiler are readily available:
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

On Windows, use the CMake GUI to create project files for Visual Studio, open the project and build in Release mode for optimal performance.
You might need to to compile LZ4 from `https://github.com/lz4/lz4` yourself and supply updated/matching variants of the files in `thirdParty/lz4/x64_Debug` and `thirdParty/lz4/x64_Release`.
