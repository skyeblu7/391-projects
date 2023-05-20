#ifndef _SYSTEMCALL_H_
#define _SYSTEMCALL_H_
#include "x86_desc.h"
#include "lib.h"
#include "types.h"
#include "paging.h"
#include "scheduler.h"

#define PHYS8MB 0x800000
#define P0KESP 0x7FFFFC
#define OFFSET8KB 0x10000
#define PHYS4MB 0x400000
#define USER_VIRTUAL 0x08000000 //128MB
#define IMG_OFFSET 0x48000
#define USER_SP_VIRTUAL 0x83FFFFC //132MB-1B
#define ELF0 0x7f
#define ELF1 0x45
#define ELF2 0x4c
#define ELF3 0x46
#define SIZEOF_FDA 8
#define NUM_PROC 6
#define ADDR_OFFSET 22
#define TABLE_ADDR_OFFSET 12
#define PHYS_BASE_ADDR 2
#define EXCEPTION_HALT_STATUS 99
#define EXCEPTION_HALT_RETURN 256
#define USER_VIDMAP_ADDR 34
#define USER_VIDMAP 0x08800000 // 136MB
#define A 0
#define B 1
#define C 2

extern uint8_t procA;
extern uint8_t procB;
extern uint8_t procC;

/* holder array for pcbs and curr_pcb pointer */
pcb_t* pcbarray[6];
pcb_t* curr_pcb;

// system calls
int sysHalt(uint8_t status);
int sysExecute(uint8_t* command);
int sysRead(int32_t fd, uint8_t* buf, int32_t nbytes);
int sysWrite(int32_t fd, uint8_t* buf, int32_t nbytes);
int sysOpen(uint8_t* filename);
int sysClose(int32_t fd);
int32_t vidmap (uint8_t** screen_start);
int sysGetArgs(uint8_t* buf, int32_t nbytes);


extern void flushingTLB();
void parseCmd (uint8_t* input, uint8_t* cmd, uint8_t* arg);

#endif
