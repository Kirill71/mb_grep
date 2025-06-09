# Grep like tool

A multithreaded file search tool implemented in C++. It searches for a given query string or regular expression across a directory tree, optionally filtering by file extension and case sensitivity.

## Features

*  Multithreaded directory traversal and file searching
*  Supports both substring and regex-based matching
*  Optional case-insensitive search
*  Filters files by extension
*  Ignores binary files automatically
*  Warns if regex-looking pattern is used without `--regex`

## Usage

```bash
  ./mb_grep <query> <directory> [--regex] [--ignore-case] [--ext=.txt]
```

### Options

| Flag            | Description                             |
| --------------- | --------------------------------------- |
| `--regex`       | Treat the query as a regular expression |
| `--ignore-case` | Perform a case-insensitive search       |
| `--ext=.ext`    | Only search files with this extension   |

### Example

```bash
  ./mb_grep error /var/log --ignore-case --regex --ext=.log
```

This searches for the regex `error` in all `.log` files under `/var/log`, ignoring case.

## Build
#### Linux/MacOS  
First of all set the execution rights to the `scripts` folder, execute the command below in project root folder.
```bash
  chmod -R +x scripts/
```
Run the provided build script:
```bash
  make build
```
It will build `Release` and `Debug` builds.

#### Windows
If you use Windows you will have to build the project using `CMake` tool.  
Alternatively, you can use MS Visual Studio as an IDE it already supports `CMake` based projects.

## Run
Go to the build folder and execute something like that:
```bash
  ./mb_grep error /var/log --ignore-case --regex --ext=.log
```
Note: May require superuser rights to visit some directories.

## Clean
Removes all generated binaries or temporary files.

#### Linux/MacOS
Make sure that you've already executed this command:
```bash
  chmod -R +x scripts/
```
then do:

```bash
  make clean
```
#### Windows
If you use Windows you will have to clean the project via `CMake` tool
 or try MS Visual Studio out, it supports `CMake` based projects nowadays.

## Pattern Debug Tip

If your query is a regex-like pattern (e.g. `n+im`, `.*`, `^pattern`) but `--regex` is not set, results may be unexpected.Make sure to use `--regex` to treat it as a regular expression.  
The application will print the warning like that:  
`Warning: The pattern "n+im" looks like a regular expression, but --regex flag was not set.` and stops execution.

## Tested Compilers
This project has been tested with the following compilers:

| Compiler    | Version               | Platform     |
|-------------|-----------------------| ------------ |
| GCC         | 13.3.0                | Ubuntu 24.04 |
| Apple Clang | 16.0.0                | macOS 14     |
| MSVC        | 19.41.34120 (VS 2022) | Windows 11   |
