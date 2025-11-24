# UDU
UDU is an extremely fast and cross-platform tool for summarizing file and directory sizes by recursively scanning directories using a parallel traversal engine implemented with [OpenMP](https://www.openmp.org/) which makes it significantly faster than traditional tools on multi-core systems.

![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/gnualmalki/udu/ci.yml?label=Linux) | ![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/gnualmalki/udu/ci.yml?label=MacOS) | ![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/gnualmalki/udu/ci.yml?label=Windows)

See [Benchmarks](./BENCHMARKS.md).

## Build Instructions
Building UDU requires a modern C compiler such as [GCC](https://gcc.gnu.org/) 9.5 or [Clang](https://clang.llvm.org/) 14.0.0 (or later), [MSVC](https://visualstudio.microsoft.com/) 17.2 or later, [CMake](https://cmake.org/) 3.15 and [OpenMP](https://www.openmp.org/) 3.0 or later (optional: parallel processing).

*On Windows, older MSVC versions do not support the OpenMP features we need. The build will work but `udu` will run single-threaded. Use MSVC 17.2 or newer, or alternatively use GCC or Clang through [MSYS2](https://www.msys2.org/) or [Cygwin](https://www.cygwin.com/), which provide modern toolchains with complete OpenMP 3.0 support.*

#### UNIX
Clone the repository and build using these commands:
```
git clone --depth=1 https://github.com/gnualmalki/udu.git
cd udu
cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_OPENMP=ON -DENABLE_LTO=ON
cmake --build build
```

#### Windows (MSYS2)
After installing MSYS2, open the MSYS2 terminal and install the required packages using `pacman -S gcc cmake`, then follow the UNIX instructions above.

#### Windows (MSVC)
using the developer command prompt:
```
git clone --depth=1 https://github.com/gnualmalki/udu.git
cd udu
cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_OPENMP=ON -DENABLE_LTO=ON
cmake --build build --config Release
```

## Usage
```
Usage: udu [option(s)...] [path(s)...]
 extremely fast disk usage analyzer with parallel traversal engine.

 OPTIONS:
  -a, --apparent-size    show file sizes instead of disk usage
                          (apparent = bytes reported by filesystem,
                           disk usage = actual space allocated)
  -h, --help             display this help and exit
  -q, --quiet            display output at program exit (default)
  -v, --verbose          display each processed file
      --version          display version info and exit
  -X, --exclude=PATTERN  skip files or directories that match glob pattern
                          *        any characters
                          ?        single character
                          [abc]    any char in set
                          Examples: '*.log', 'temp?', '[0-9]*'

 EXAMPLE:
   udu ~/ -avX epstein-files

Report bugs to <https://github.com/gnualmalki/udu/issues>
```

## License
THIS PROGRAM IS FREE SOFTWARE: YOU CAN REDISTRIBUTE IT AND OR MODIFY IT UNDER THE TERMS OF THE GNU GENERAL PUBLIC LICENSE AS PUBLISHED BY THE FREE SOFTWARE FOUNDATION, EITHER VERSION 3 OF THE LICENSE, OR ANY LATER VERSION.

THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
