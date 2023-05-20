
#ifndef PAGING_H
#define PAGING_H
#include "x86_desc.h"
#include "lib.h"
#define VIDEO       0xB8000
#define ACACHE      0xB9000
#define BCACHE      0xBA000
#define CCACHE      0xBB000
#define NUM_TABLE_ENTRIES  1024
#define NUM_DIR_ENTRIES    1024

void paging_setup();
/* accessing functions from page_enabling.S */
extern void loadPageDirectory();
extern void enablePaging();
extern void enableMixing();
#endif
