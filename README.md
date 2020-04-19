# edlib-awk-extension

A GNU awk extension to use the edlib library.

## Building

This project uses CMake to build itself. Run the following commands at the project's root to build:

1. `git submodule init && git submodule update` - this will download `edlib`'s repository into the vendor/edlib directory
2. `mkdir build && cd build`
3. `cmake -D CMAKE_BUILD_TYPE=Release ..`
4. `make`

This will create a `edlib.so` in the `build` directory.

## Installation

To install the library so it can easily be loaded by gawk, run `sudo make install` in the build directory.

## Usage

After installation, the extension should be loaded through either through the `-ledlib` command line argument or `@load "edlib"` at the start of the script:

```shell
awk -ledlib 'BEGIN { print(edlib("ATCG", "ATCCG")) }' # prints '3M1D1M'
```

```shell
awk '@load "edlib"; BEGIN { print(edlib("ATCG", "ATCCG")) }'
```

Alternatively, if the extension was not installed, it can be loaded from the current directory by specifying the path as ./edlib, or by changing the AWKLIBPATH variable:

```shell
awk -l./edlib 'BEGIN { print(edlib("ATCG", "ATCCG")) }'
```

```shell
awk '@load "./edlib"; BEGIN { print(edlib("ATCG", "ATCCG")) }'
```

```shell
AWKLIBPATH=$(pwd) awk -ledlib 'BEGIN { print(edlib("ATCG", "ATCCG")) }'
```

The function accepts up to 4 arguments: sequence one, sequence two, alignment method and maximum distance. The alignment method defaults to "NW", and the maximum distance defaults to -1 (unlimited). Here are some examples:

```shell
awk -ledlib 'BEGIN { print(edlib("ATCG", "ATCCGAAAAAA", "HW", 5)) }' # prints '3M1I'
```

```shell
awk -ledlib 'BEGIN { print(edlib("ATCG", "GGGGATCCGAAAAAA", "HW")) }' # prints '3M1I'
```
