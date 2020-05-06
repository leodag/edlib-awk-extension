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

To install the library so it can easily be loaded by gawk, run `sudo make install` in the build directory. This way, no additional varialbes need to be set when invoking awk.

```shell
$ awk -iedlib 'BEGIN { edlib(result, "ATCG", "ATCCG"); print(result["cigar"]) }'
```

Alternatively to installation, change AWKPATH to include the directory containing edlib.awk, and AWKLIBPATH to include the directory containing edlib.so:

```shell
$ AWKPATH=$(pwd) AWKLIBPATH=$(pwd)/build awk -iedlib 'BEGIN { edlib(result, "ATCG", "ATCCG"); print(result["cigar"]) }'
```

An example script (./awk4) is included which can be used to run `awk` with the parameters set as needed.

## Usage

After installation, the extension should be loaded through either through the `-iedlib` command line argument or `@include "edlib"` at the start of the script:

```shell
$ awk -iedlib 'BEGIN { edlib(result, "ATCG", "ATCCG"); print(result["cigar"]) }' # prints '3=1D1='
```

```shell
$ awk '@include "edlib"; BEGIN { edlib(result2, "ATCG", "ATCCG"); print(result2["cigar"]) }'
```

The `edlib` function accepts up to 6 arguments: destination array, sequence one, sequence two, alignment method, cigar output mode and maximum distance. The alignment method defaults to "NW", the cigar output mode defaults to "EXTENDED" and the maximum distance defaults to -1 (unlimited). Here are some examples:

```awk
@include "edlib"
BEGIN {
    edlib(result, "ATCGG", "ATCCGAAAAAA", "HW", "STANDARD", 5)
    print(result["cigar"]) # prints '5M'
}
```

```awk
@include "edlib"
BEGIN {
    edlib(result, "ATCGG", "GGGGATCCGAAAAAA", "HW")
    print(result["cigar"]) # prints '3=1X1='
}
```

The following fields are available in the result array:

```awk
result["cigar"] = 3=1X1=
result["edit_distance"] = 1
result["start_locations"]["0"] = 4
result["end_locations"]["0"] = 8
result["alphabet_length"] = 4
```

The function `edlib_get_nice_alignment` is available to obtain the alignment in a nice format:

```awk
@include "edlib"
BEGIN {
    query = "ATCGG"
    target = "GGGGATCCGAAAAAA"
    edlib(result, query, target, "NW")
    edlib_get_nice_alignment(nice_align, result, query, target)
    print(nice_align["target_aligned"])  # prints 'GGGGATCCGAAAAAA'
    print(nice_align["matched_aligned"]) # prints '----|||.|------'
    print(nice_align["query_aligned"])   # prints '----ATCGG------'
}
```

For convenience another function `edlib_print_nice_alignment` is available to print the alignment:

```awk
@include "edlib"
BEGIN {
    query = "ATCGG"
    target = "GGGGATCCGAAAAAA"
    edlib(result, query, target, "NW")
    edlib_print_nice_alignment(nice_align, result, query, target)
    # prints 'T:  GGGGATCCGAAAAAA (0 - 14)
    #             ----|||.|------
    #         Q:  ----ATCGG------ (0 - 4)'
}
```
