# mini-shell

A simple C++ shell for learing.

- One command execution
- Input/output redirect
- A pipeline working with two commands

## Build

```sh
$ g++ -std=c++17 main.cpp
```

## Usage

```sh
$ ./a.out
@ ls
a.out  main.cpp
@ cat main.cpp | wc -l > out.txt
@ cat out.txt
130
@ quit
```
