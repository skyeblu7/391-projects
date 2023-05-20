/* types.h - Defines to use the familiar explicitly-sized types in this
 * OS (uint32_t, int8_t, etc.).  This is necessary because we don't want
 * to include <stdint.h> when building this OS
 * vim:ts=4 noexpandtab
 */

#ifndef _TYPES_H
#define _TYPES_H

#define NULL 0
#define NAME_LEN 122
#define CHAR_LIM 122

#ifndef ASM

/* Types defined here just like in <stdint.h> */
typedef int int32_t;
typedef unsigned int uint32_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef unsigned char uint8_t;

/* directory entry (dentry) struct */
typedef struct dentry{
    uint8_t* fname; //32B filename
    uint8_t ftype; //4B file type 
    uint8_t inode; //4B inode index
}dentry_t;

/* boot block struct */
typedef struct bootBlock{
    void* addr; //address of boot block in memory
    int32_t dir_count; //number of dentries in the system
    int32_t inode_count; //number of inodes
    int32_t data_count; //number of data blocks
}bootBlock_t;

/* fops jumptable struct */
typedef struct functionOps{
    int32_t (*read)(int32_t, uint8_t*, int32_t);
    int32_t (*write)(int32_t, uint8_t*, int32_t);
    int32_t (*open)(int32_t);
    int32_t (*close)(int32_t);
}fops_t;


/* fd tracker struct
 * For the rtc, this will be a dentry and db_num and byte_num
 * are ignored. For files they are used to keep track of which
 * data block in the inode we're on, and which byte of that data
 * block we're on. For directories, the db_num value will keep track
 * of which dentry we're on while traversing the '.' directory and 
 * the byte_num field, which byte of that dentry's 4B name
 * we are currently reading.
 */
typedef struct filedescriptor{
    int32_t db_num;
    int32_t byte_num;
    int32_t inode;
    fops_t* fops;
    int32_t flags;
}fd_t;

/* pcb struct */
typedef struct pcb{
    uint8_t active;
    uint8_t curr_pid;
    int8_t parent_pid;
    fd_t fda[8];
    uint8_t arg[NAME_LEN];
    uint8_t cmd[NAME_LEN];
    uint32_t esp;
    uint32_t ebp;
    uint32_t parent_esp;
    uint32_t parent_ebp;
}pcb_t; 

/* terminal struct */
typedef struct terminal{
    int screen_x;
    int screen_y;
    char typing_buf[CHAR_LIM];
    int typing_buf_idx;
    int active;
    void* vidmem;
    int top_pid;
    int enter_pressed;
}term_t;

#endif /* ASM */

#endif /* _TYPES_H */
