#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Constants
#define MAX_LINE_LENGTH 256

// Global variables

/**
 * Global pointer to the filename in use (defaults to "todo.md").
 */
extern const char *todos_filename;

/**
 * Global pointer holding all lines from the TODO file.
 */
extern char **todo_lines;

// Utility functions

void print_usage(const char *prog_name);
char *skip_leading_whitespace(char *str);

// File operations

char **get_all_lines(void);
void save_todos(void);

// Task operations

int *get_unfinished_tasks(int *count_out);
int *get_finished_tasks(int *count_out);
void delete_line(int line_index);
void remove_finished_tasks(void);
void remove_task(int index);
void check_todo(int index);
void list_todos(void);
void add_todo(const char *task);

// Helper functions

int call_fn_with_indexes(int argc, char *argv[], int index, void (*fn)(int));
int compare_int_desc(const void *a, const void *b);
