# newls

![Language](https://img.shields.io/badge/language-C-blue.svg)
![License](https://img.shields.io/badge/license-GPLv3-green.svg)

A POSIX-compliant implementation of the Unix `ls` utility written in C in my free time.

## Flags
- **Long listing format (`-l`)**: Displays file permissions, hard link count, owner, group, byte size, last modification timestamp, and filename.

- **Hidden files (`-a`)**: Includes entries starting with `.` (including `.` and `..`).

- **Reverse sorting (`-r`)**: Reverses the order of the sort output.

- **Size sorting (`-S`)**: Sorts directory entries by file size.


## Building

Compile the project from source using `gcc` or `clang`:

```bash
gcc -Wall -Wextra -O2 -o newls main.c

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

Inspect a specific directory:

```bash
./newls -l /etc

```

## Licence

This project is licensed under the **GNU General Public License v3.0** — see the [LICENCE](https://www.google.com/search?q=LICENSE) file for details.
