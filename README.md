# Todolala

A simple command-line todo app written in C using the Zig build system.

It stores todos using the markdown task syntax `- [x] my task` in a todo.md file in the current directory. Anything that isn't a task is ignored, so you can add other markdown to the file and Todolala will not touch it. It is actually used to manage the todo section at the bottom of this readme.

# Building

```bash
zig build
```

This cross compiles for Windows, macOS and Linux. The compiled executables are in the folder zig-out.

# Running tests

You can run the test suite using zig:

```bash
zig run test_todo.c todo.c munit.c -DTESTING
```

# Usage

By default the executable is called todo so that you have to type less, but you can of course rename it.

```bash
todo
```

Without arguments, it prints out help information:

```
Usage:
  todo [<file.md>] "<task>"           - Add a new task (default file: todo.md).
  todo [<file.md>] l(ist)             - List all unfinished tasks.
  todo [<file.md>] c(heck) <index>    - Mark the <index>th unfinished task as finished.
  todo [<file.md>] r(emove) <index>   - Remove the <index>th unfinished task.
  todo [<file.md>] clean              - Remove all finished tasks.

You can also use multiple <index>es for check and remove commands, i.e. todo check 1 2 3.
```

So to see all of the todos in this readme, you would do:

```bash
todo README.md list

# or using the shortform for list, which is just l

todo README.md l
```

If you don't specify a filename, todo.md is used, so you can also just use:

```bash
todo list
todo check 2 6
```

# ToDo

- [ ] command to sort todos with finished tasks at bottom
- [ ] command to list finished todos
