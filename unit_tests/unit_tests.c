#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "../chess.h"

/// testing framework ///

#define TESTFAILED(X) testFailed(__FILE__, __LINE__, __func__, X)
void testFailed(const char* test_file, const int line_number, const char* test_name, const char* message)
{
  fprintf(stderr, "TEST FAILED: %s, file %s, line %i - %s\n", test_name, test_file, line_number, message);
};

/// testing framework ///


