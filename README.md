# newls

![Language](https://img.shields.io/badge/language-C-blue.svg)
![License](https://img.shields.io/badge/license-GPLv3-green.svg)

A POSIX-compliant implementation of the Unix `ls` utility written in C in my free time.

## Flags
- **Long listing format (`-l`)**: Displays the `total` 1K block header, aligned columns for file permissions, hard link count, owner, group, byte size, last modification timestamp, filename, and literal symlink targets (`name -> target`).
- **Hidden files (`-a`)**: Includes all entries starting with `.` (including `.` and `..`).
- **Almost all (`-A`)**: Includes entries starting with `.`, excluding `.` and `..`.
- **Reverse sorting (`-r`)**: Reverses the order of the active sort mode.
- **Size sorting (`-S`)**: Sorts directory entries by file size (largest first by default, breaking ties lexicographically).
- **Time sorting (`-t`)**: Sorts directory entries by last modification time (newest first by default, using nanosecond precision and breaking ties lexicographically).

By default (without `-S` or `-t`), entries are sorted lexicographically.

## Building

Compile the project from source using `gcc` or `clang`:

```bash
gcc -std=c99 -O2 -Wall -Wextra -D_DEFAULT_SOURCE main.c -o newls

```

## Usage

```bash
./newls [OPTIONS] [FILE/DIRECTORY...]

```

### Examples

List files in the current directory:

```bash
./newls

```

List all files (including hidden ones) in long format, reversed:

```bash
./newls -lar

```

List entries almost all in long format sorted by modification time:

```bash
./newls -lAt
```

Inspect a specific directory:

```bash
./newls -l /etc

```

## Licence

This project is licensed under the **GNU General Public License v3.0** — see the [LICENCE](https://www.google.com/search?q=LICENSE) file for details.
