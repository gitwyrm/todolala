# Todolala

A simple command-line todo app written in C using the Zig build system.

It stores todos using the markdown task syntax `- [x] my task` in a todo.md file in the current directory. Anything that isn't a task is ignored, so you can add other markdown text in the file.

# Building

```bash
zig build
```

This cross compiles for Windows, macOS and Linux. The compiled executables are in the folder zig-out.

# Usage

By default the executable is called todo so that you have to type less, but you can of course rename it.

```bash
todo
```

Without arguments, it prints out help information:

```
Usage:
  todo "<task>"       - Add a new task to todo.md.
  todo list           - List all unfinished tasks.
  todo check <index>  - Mark the <index>th unfinished task as finished.
  todo remove <index> - Remove the <index>th unfinished task.
  todo clean          - Remove all finished tasks.
```