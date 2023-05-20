#ifndef TESTS_H
#define TESTS_H

#include "types.h"

// test launcher
void launch_tests();

/* Division Error Exception Test */
void div_zero();
int access_video_mem();
void access_not_in_range();
void access_no_table_entry ();
int access_kernel_mem();
int page_structure();
void test_getArgs();




#endif /* TESTS_H */
