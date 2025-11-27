# UDU

UDU is an extremely fast POSIX-command-line-program for summarizing file and directory sizes by recursively scanning directories using a parallel traversal engine implemented with [OpenMP](https://www.openmp.org/) making it significantly faster than traditional tools on multi-core systems.

See [Benchmarks](./BENCHMARKS.md).

## Installation

### Pre-built Binaries

You can download pre-built binaries from the [GitHub Releases](https://github.com/gnualmalki/udu/releases) page.

### Build from Source

Building UDU requires a modern C compiler such as [GCC](https://gcc.gnu.org/) 9.5 or [Clang](https://clang.llvm.org/) 14.0.0 (or later), [GNU Make](https://www.gnu.org/software/make/), and [OpenMP](https://www.openmp.org/) 3.0 or later (optional: for parallel processing).

Clone the repository and build using these commands:

```bash
git clone --depth=1 https://github.com/gnualmalki/udu.git
cd udu
make -B
```

Install with:
```bash
make install # may require sudo
```

## Usage

```bash
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

Report bugs to <https://github.com/makestatic/udu/issues>
```

## License

<sub>THIS PROGRAM IS FREE SOFTWARE: YOU CAN REDISTRIBUTE IT AND/OR MODIFY IT UNDER THE TERMS OF THE GNU GENERAL PUBLIC LICENSE AS PUBLISHED BY THE FREE SOFTWARE FOUNDATION, EITHER VERSION 3 OF THE LICENSE, OR ANY LATER VERSION.</sub>

<sub>THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. SEE THE GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.</sub>
