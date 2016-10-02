#ifndef TLDR_TEST_FOO_H_
#define TLDR_TEST_FOO_H_

#include <tldr/export.h>

#ifdef __cplusplus
extern "C" {
#endif

TLDR_EXPORT extern const int foo_test_data;
TLDR_EXPORT extern int foo_test_proc(void);
TLDR_EXPORT extern void foo_test_imports(void);

#ifdef __cplusplus
}
#endif

#endif
