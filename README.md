# c't Puzzle from Issue 19/2023

There were two steps:
First, a puzzle consisting of nine unique pieces had to be solved, forming a 3x3 picture.
Every puzzle piece has two characters associated with every outside edge (so four times two characters per piece), written in hexadecimal.
After solving the puzzle, it can be rotated according to an arrow on one of the pieces.
The arrow must point upwards.
For the second step, starting left top and going right, top to bottom, we can concatinate the characters that are now horizontal, for a total of 18 characters.
The message is encoded using a Caesar-chiffre.
A quick enumeration of all 26 solutions quickly yields the correct answer.

<img src="/solution.png?raw=true" width="350" alt="The solved puzzle">

## Complexity
The puzzle has `9! = 362880` possibilities of placing the tiles, and each can be rotated to one of four positions (`4^9 = 262144`), again divided by four because of rotational symmetry, giving a total of 23781703680 possibilities.

## License
The tool is governed by the GNU GPL v2.0 license. See [`LICENSE`](LICENSE) for more information.

## Requirements
 - [CMake](https://cmake.org/)
 - C++17, meaning a recent MSVC/GCC/Clang that supports at least C++17
 - Qt 6
 
## How to build

On Linux or MacOS, if CMake, Qt 6 and a compiler are readily available:
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

On Windows, use the CMake GUI to create project files for Visual Studio, open the project and build in Release mode for optimal performance.
You might need to adapt the "Additional Qt search path" in `PROJECT_CMAKE_SEARCH_PATH` to point to your installation.
