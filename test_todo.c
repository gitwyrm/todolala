#include "munit.h"
#include "todo.h"

// Mock data for testing
static char *mock_lines[] = {
    "- [ ] Task 1\n",
    "- [x] Task 2\n",
    "- [ ] Task 3\n",
    NULL
};

// Test get_unfinished_tasks
static MunitResult test_get_unfinished_tasks(const MunitParameter params[], void *data) {
    todo_lines = mock_lines;

    int count = 0;
    int *unfinished = get_unfinished_tasks(&count);

    munit_assert_int(count, ==, 2);  // Expect 2 unfinished tasks
    munit_assert_not_null(unfinished);
    munit_assert_int(unfinished[0], ==, 0);  // First unfinished task index
    munit_assert_int(unfinished[1], ==, 2);  // Second unfinished task index

    free(unfinished);
    return MUNIT_OK;
}

// Test add_todo
static MunitResult test_add_todo(const MunitParameter params[], void *data) {
    todos_filename = "test_todo.md";
    remove(todos_filename);

    add_todo("New task");

    FILE *file = fopen(todos_filename, "r");
    munit_assert_not_null(file);

    char buffer[256];
    fgets(buffer, sizeof(buffer), file);
    fclose(file);

    munit_assert_string_equal(buffer, "- [ ] New task\n");
    remove(todos_filename);
    return MUNIT_OK;
}

static MunitTest tests[] = {
    { "/get_unfinished_tasks", test_get_unfinished_tasks, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/add_todo", test_add_todo, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
    "/todo-tests",
    tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char* argv[]) {
    return munit_suite_main(&suite, NULL, argc, argv);
}