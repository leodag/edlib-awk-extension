# edlib-awk-extension

A GNU awk extension to use the edlib library.

edlib's source and header are bundled since old versions of GCC do not compile static libraries with -fPIC; which makes using its libedlib.a in a shared library impossible without modifying its CMakeLists or bundling it.

## Building

This project uses CMake to build itself. Run the following commands at the project's root to build:

1. `mkdir build && cd build`
2. `cmake -D CMAKE_BUILD_TYPE=Release ..`
3. `make`

This will create a `edlib.so` in the `build` directory.

## Installation

To install the library so it can easily be loaded by gawk, run `sudo make install` in the build directory.

## Usage

After installation, the extension should be loaded through either through the `-ledlib` command line argument or `@load "edlib"` at the start of the script:

```shell
awk -ledlib 'BEGIN { edlib(result, "ATCG", "ATCCG"); print(result["cigar"]) }' # prints '3=1D1='
```

```shell
awk '@load "edlib"; BEGIN { edlib(result2, "ATCG", "ATCCG"); print(result2["cigar"]) }'
```

Alternatively, if the extension was not installed, it can be loaded from the current directory by specifying the path as ./edlib, or by changing the AWKLIBPATH variable:

```shell
awk -l./edlib 'BEGIN { edlib(result, "ATCG", "ATCCG"); print(result["cigar"]) }'
```

```shell
awk '@load "./edlib"; BEGIN { edlib(result, "ATCG", "ATCCG"); print(result["cigar"]) }'
```

```shell
AWKLIBPATH=$(pwd) awk -ledlib 'BEGIN { edlib(result, "ATCG", "ATCCG"); print(result["cigar"]) }'
```

The function accepts up to 6 arguments: destination array, sequence one, sequence two, alignment method, cigar output mode and maximum distance. The alignment method defaults to "NW", the cigar output mode defaults to "EXTENDED" and the maximum distance defaults to -1 (unlimited). Here are some examples:

```shell
awk -ledlib 'BEGIN { edlib(result, "ATCGG", "ATCCGAAAAAA", "HW", "STANDARD", 5); print(result["cigar"]) }' # prints '5M'
```

```shell
awk -ledlib 'BEGIN { edlib(result, "ATCGG", "GGGGATCCGAAAAAA", "HW"); print(result["cigar"]) }' # prints '3=1X1='
```

The following fields are available in the result array:

```awk
result["cigar"] = 3=1X1=
result["edit_distance"] = 1
result["start_locations"]["0"] = 4
result["end_locations"]["0"] = 8
result["alphabet_length"] = 4
```
