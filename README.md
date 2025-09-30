# rename

Rename multiple files at once by replacing a certain PATTERN with a REPLACEMENT.

Written in C++23.

Supports regular expression patterns and group substitutions.

### Examples

```
rename foo bar  .
rename foo-(\d+).txt bar-$1.txt
```

### Build

###

```
mkdir build
cd build
cmake ..
make
```

### Install

```
sudo make install
```

The `CMAKE_INSTALL_BINDIR` option can be used to install in a non-default path.

```
cmake .. -DCMAKE_INSTALL_BINDIR=/usr/local/bin
```
